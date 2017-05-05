#include "CollisionManager.h"
#include "MetaData.h"
#include "TSScene.h"
#include "Model.h"
#include "SceneSemGraph.h"

#include "triangle_triangle_intersection.h"

TrianglePredicate::TrianglePredicate(Model *first, mat4 &firstTransform, Model *second, mat4 &secondTransform)
{
	model[0] = first, model[1] = second;
	transform[0] = firstTransform, transform[1] = secondTransform;
}

bool TrianglePredicate::operator()(const SimpleBoxNodeData &a, const SimpleBoxNodeData &b) const
{
	// retrieve triangle coordinates, transformed, and perform triangle test 
	for (int i = 0; i < a.triangles.size(); ++i) {
		vec3 p1, q1, r1;
		model[0]->getTriangle(a.triangles[i], p1, q1, r1);
		p1 = TransformPoint(transform[0], p1);
		q1 = TransformPoint(transform[0], q1);
		r1 = TransformPoint(transform[0], r1);
		for (int j = 0; j < b.triangles.size(); ++j) {
			vec3 p2, q2, r2;
			model[1]->getTriangle(b.triangles[j], p2, q2, r2);
			p2 = TransformPoint(transform[1], p2);
			q2 = TransformPoint(transform[1], q2);
			r2 = TransformPoint(transform[1], r2);

			if (Guigue::tri_tri_overlap_test_3d(&p1.x, &q1.x, &r1.x,
				&p2.x, &q2.x, &r2.x)) return true;
		}
	}
	return false;
}

CollisionManager::CollisionManager(TSScene *s)
	:m_scene(s)
{
	// build BVH
	m_boxBVHs.resize(m_scene->modelNum());
	m_collisionPositions.resize(m_scene->modelNum());
}

CollisionManager::~CollisionManager()
{
	if (!m_boxBVHs.empty())
	{
		for (int i = 0; i < m_scene->modelNum(); i++)
		{
			if (m_boxBVHs[i])
			{
				delete m_boxBVHs[i];
			}

			m_collisionPositions[i].clear();
		}

		m_boxBVHs.clear();
	}

	m_collisionPositions.clear();
}

bool CollisionManager::checkCollisionBVH(Model *testModel, int testMetaModelIdx)
{
	bool isCollide = false;
	MetaModel& testMd = m_scene->getMetaModel(testMetaModelIdx);

	if (!testMd.isBvhReady)
	{
		rebuildBVH(testModel, testMetaModelIdx);
	}

	// do not check for model that is already placed in the scene
	if (testMd.isAlreadyPlaced)
	{
		return false;
	}

	// trans mat that brings model at pos in current scene
	mat4 testModelTransMat = testMd.transformation;

	for (int i = 0; i < m_scene->modelNum(); i++)
	{
		MetaModel& refMd = m_scene->getMetaModel(i);

		// only check collision with model that is already placed in the scene
		if (i != testMetaModelIdx && refMd.isAlreadyPlaced)
		//if (i != testMetaModelIdx)
		{
			Model* refModel = m_scene->getModel(refMd.name);

			if (refModel != NULL)
			{
				if (!refMd.isBvhReady)
				{
					rebuildBVH(refModel, i);
				}
				mat4 refModelTransMat = refMd.transformation;
				TrianglePredicate predicate(testModel, testModelTransMat, refModel, refModelTransMat);
				isCollide = m_boxBVHs[testMetaModelIdx]->hit(*m_boxBVHs[i], predicate);

				if (isCollide)
				{
					qDebug() << QString(" Preview:%1 %6 Type: %2 Model:%3 DBSSG:%4 TrialNum:%5").
						arg(m_scene->m_previewId).arg("BVH").arg(toQString(refMd.catName)).arg(m_scene->m_ssg->m_metaScene.m_sceneFileName).arg(testMd.trialNum).arg(toQString(testMd.catName));

					testMd.isBvhReady = false;

					// store invalid positions to speed up
					m_collisionPositions[testMetaModelIdx].push_back(testMd.position);
					return true;
				}
			}
		}
	}

	return false;
}

void CollisionManager::rebuildBVH(Model *testModel, int idx)
{
	if (m_boxBVHs[idx])
	{
		delete m_boxBVHs[idx];
	}

	// get the boxes of the leaf nodes of the model BVH 
	std::vector<BvhLeafNode> nodes;
	testModel->getBvhLeafNodes(nodes);

	MetaModel& testMetaModel = m_scene->getMetaModel(idx);

	// apply transformation
	for (int i = 0; i < nodes.size(); ++i) {
		nodes[i].box = nodes[i].box.transformBoudingBox(testMetaModel.transformation);
	}

	std::vector<SimpleBoxNodeData> boxes(nodes.size());
	for (int i = 0; i < nodes.size(); ++i) {
		boxes[i].box = nodes[i].box;
		boxes[i].triangles = nodes[i].triangles;
	}
	m_boxBVHs[idx] = new BoxBvh<SimpleBoxNodeData>(boxes);
	testMetaModel.isBvhReady = true;
}

bool CollisionManager::isRayIntersect(const Ray &ray, int parentModelId, double &z)
{
	MetaModel &parentMd = m_scene->getMetaModel(parentModelId);
	Model *parentModel = m_scene->getModel(parentMd.name);

	bool isIntersect;

	if (parentModel!=NULL)
	{
		if (!parentMd.isBvhReady)
		{
			rebuildBVH(parentModel, parentModelId);
		}

		BoxHitRecord hitRecord;
		isIntersect = m_boxBVHs[parentModelId]->hit(ray, hitRecord);
		if (isIntersect)
		{
			float3 hitPos = ray.point(hitRecord.t);
			z = hitPos.z;
			return true;
		}
	}

	return false;
}

bool CollisionManager::intersectAABB(const vec3 &miA, const vec3 &maA, const vec3 &miB, const vec3 &maB, double delta /*= 0*/)
{
	return (miA.x + delta <= maB.x && maA.x - delta >= miB.x) &&
		(miA.y + delta <= maB.y && maA.y - delta >= miB.y) &&
		(miA.z + delta <= maB.z && maA.z - delta >= miB.z);
}

bool CollisionManager::checkCollision(Model *testModel, int testModelIdx)
{
	bool isCollide = false;

	// do not check for model that is already placed in the scene
	MetaModel& testMetaModel = m_scene->getMetaModel(testModelIdx);
	if (testMetaModel.isAlreadyPlaced)
	{
		return false;
	}

	// trans mat that brings model at pos in current scene
	mat4 testModelTransMat = testMetaModel.transformation;

	// compute the new transformed AABB
	BoundingBox transTestBB = testModel->m_bb.transformBoudingBox(testModelTransMat);
	vec3 testModelMi = transTestBB.mi();
	vec3 testModelMa = transTestBB.ma();

	double delta = 0.01 / params::inst()->globalSceneUnitScale;

	QString collisionType;

	for (int i = 0; i < m_scene->modelNum(); i++)
	{
		MetaModel& refMetaModel = m_scene->getMetaModel(i);
		bool isModelAlreadyPlaced = refMetaModel.isAlreadyPlaced;

		// only check collision with model that is already placed in the scene
		if (i != testModelIdx && isModelAlreadyPlaced)
		{
			Model* refModel = m_scene->getModel(refMetaModel.name);

			if (refModel != NULL)
			{
				// compute the new transformed BB
				BoundingBox transRefBB = refModel->m_bb.transformBoudingBox(refMetaModel.transformation);
				vec3 refModelMi = transRefBB.mi();
				vec3 refModelMa = transRefBB.ma();

				bool isAABBCollide = intersectAABB(testModelMi, testModelMa, refModelMi, refModelMa, delta);
				bool isOBBMeshCollide = false;
				bool isMeshMeshCollide = false;

				if (isAABBCollide)
				{

					collisionType = "AABB(true)";
					// test transformed BB to model with current scene transformation
					isOBBMeshCollide = refModel->checkCollisionBBTriangles(testModel->m_bb, testModelTransMat, refMetaModel.transformation);

					if (isOBBMeshCollide)
					{
						collisionType += "_OBBMesh(true)";
						isMeshMeshCollide = refModel->checkCollisionTrianglesTriangles(testModel, testModelTransMat, refMetaModel.transformation);

						if (isMeshMeshCollide)
						{
							collisionType += "_MeshMesh(true)";
						}
					}
				}

				//isCollide = isAABBCollide && isOBBMeshCollide;
				isCollide = isAABBCollide && isOBBMeshCollide && isMeshMeshCollide;

				if (isCollide)
				{
					//qDebug() << QString(" Preview:%1 Collide Type: %2 DBSSG:%3").arg(m_scene->m_previewId).arg(collisionType).arg(m_scene->m_ssg->m_metaScene.m_sceneFileName);
					return isCollide;
				}
			}
		}
	}

	return false;
}
