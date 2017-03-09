#include "CollisionManager.h"
#include "MetaData.h"
#include "TSScene.h"
#include "Model.h"
#include "SceneSemGraph.h"

CollisionManager::CollisionManager(TSScene *s)
	:m_scene(s)
{
	// build BVH
	m_boxBVHs.resize(m_scene->modelNum());
	m_invalidPositions.resize(m_scene->modelNum());

	m_trialNumLimit = 20;
	m_closeSampleTh = 0.02;

	m_sceneMetric = params::inst()->globalSceneUnitScale;
}


CollisionManager::~CollisionManager()
{
	for (int i = 0; i < m_scene->modelNum(); i++)
	{
		if (m_boxBVHs[i])
		{
			delete m_boxBVHs[i];
		}

		m_invalidPositions[i].clear();
	}

	m_boxBVHs.clear();
	m_invalidPositions.clear();
}

bool CollisionManager::checkCollisionBVH(Model *testModel, int testMetaModelIdx)
{
	bool isCollide = false;
	MetaModel& testMetaModel = m_scene->getMetaModel(testMetaModelIdx);

	if (!testMetaModel.isBvhReady)
	{
		rebuildBVH(testModel, testMetaModelIdx);
	}

	// do not check for model that is already placed in the scene
	if (testMetaModel.isAlreadyPlaced)
	{
		return false;
	}

	// trans mat that brings model at pos in current scene
	mat4 testModelTransMat = testMetaModel.transformation;

	for (int i = 0; i < m_scene->modelNum(); i++)
	{
		MetaModel& refMetaModel = m_scene->getMetaModel(i);
		bool isModelAlreadyPlaced = refMetaModel.isAlreadyPlaced;

		// only check collision with model that is already placed in the scene
		if (i != testMetaModelIdx && isModelAlreadyPlaced)
		{
			Model* refModel = m_scene->getModel(refMetaModel.name);

			if (refModel != NULL)
			{
				mat4 refModelTransMat = refMetaModel.transformation;

				if (!refMetaModel.isBvhReady)
				{
					rebuildBVH(refModel, i);
				}

				TrianglePredicate predicate(testModel, testModelTransMat, refModel, refModelTransMat);
				isCollide = m_boxBVHs[testMetaModelIdx]->hit(*m_boxBVHs[i], predicate);

				if (isCollide)
				{
					qDebug() << QString(" Preview:%1 Collide Type: %2 Model:%3 DBSSG:%4").
						arg(m_scene->m_previewId).arg("BVH").arg(refMetaModel.catName.c_str()).arg(m_scene->m_ssg->m_metaScene.m_sceneFileName);

					testMetaModel.isBvhReady = false;

					// store invalid positions to speed up
					m_invalidPositions[testMetaModelIdx].push_back(testMetaModel.position);
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
					qDebug() << QString(" Preview:%1 Collide Type: %2 DBSSG:%3").arg(m_scene->m_previewId).arg(collisionType).arg(m_scene->m_ssg->m_metaScene.m_sceneFileName);
					return isCollide;
				}
			}
		}
	}

	return false;
}

bool CollisionManager::resolveCollision(int metaModelID)
{
	MetaModel &currMd = m_scene->getMetaModel(metaModelID);
	mat4 transMat;
	vec3 translateVec;

	int parentNodeId = m_scene->m_ssg->findParentNodeId(metaModelID);
	int parentMetaModelId = m_scene->m_ssg->m_objectGraphNodeIdToModelSceneIdMap[parentNodeId];

	QString sampleType;
	
	if (parentNodeId != -1)
	{
		MetaModel &parentMd = m_scene->getMetaModel(parentMetaModelId);

		SuppPlane &parentSuppPlane = parentMd.suppPlane;
		if (parentSuppPlane.m_isInited)
		{
			sampleType = " on parent-" + m_scene->m_ssg->m_nodes[parentNodeId].nodeName;

			vec3 currUVH = currMd.parentPlaneUVH; // UV, and H w.r.t to parent support plane
			std::vector<double> stdDevs(2, 0.1);

			bool candiFound = false;
			while (!candiFound)
			{
				vec3 newPos = parentSuppPlane.randomGaussSamplePtByUV(currUVH, stdDevs);

				if (!isPosCloseToInvalidPos(newPos, metaModelID))
				{
					candiFound = true;
					translateVec = newPos - currMd.position;
				}
			}
		}
	}
	else
	{
		sampleType = "on floor";

		std::vector<double> shiftVals;
		std::vector<double> dMeans(2, 0); // set mean to be (0,0)
		std::vector<double> stdDevs(2, 0.2);

		bool candiFound = false;
		while (!candiFound)
		{
			GenNNormalDistribution(dMeans, stdDevs, shiftVals);
			translateVec = vec3(shiftVals[0] / m_sceneMetric, shiftVals[1] / m_sceneMetric, 0);

			vec3 newPos = currMd.position + translateVec;
			if (!isPosCloseToInvalidPos(newPos, metaModelID))
			{
				candiFound = true;
			}
		}
	}

	transMat = transMat.translate(translateVec);

	currMd.position = transMat*currMd.position;
	currMd.transformation = transMat*currMd.transformation;
	currMd.frontDir = TransformVector(currMd.transformation, currMd.frontDir);
	currMd.upDir = TransformVector(currMd.transformation, currMd.upDir);
	currMd.suppPlane.tranfrom(transMat);

	// update meta model in SSG
	m_scene->m_ssg->m_metaScene.m_metaModellList[metaModelID] = currMd;

	qDebug() << QString("  Preview:%2 Resolve trial:%1 Type:%3 Vec:(%4,%5,%6) Name:%7").arg(currMd.trialNum).arg(m_scene->m_previewId).arg(sampleType)
		.arg(translateVec.x*m_sceneMetric).arg(translateVec.y*m_sceneMetric).arg(translateVec.z*m_sceneMetric)
		.arg(QString(m_scene->m_ssg->m_metaScene.m_metaModellList[metaModelID].catName.c_str()));

	return true;
}

bool CollisionManager::isPosCloseToInvalidPos(const vec3 &pos, int metaModelId)
{
	for (int i = 0; i < m_invalidPositions[metaModelId].size(); i++)
	{
		double d = (pos - m_invalidPositions[metaModelId][i]).length();

		if (d < m_closeSampleTh/m_sceneMetric)
		{
			return true;
		}
	}

	return false;
}