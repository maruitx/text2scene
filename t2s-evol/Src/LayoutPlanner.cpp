#include "LayoutPlanner.h"
#include "SceneSemGraph.h"
#include "TSScene.h"
#include "Model.h"
#include "Utility.h"


LayoutPlanner::LayoutPlanner()
{
	m_closeSampleTh = 0.03;
	m_sceneMetric = params::inst()->globalSceneUnitScale;
}


LayoutPlanner::~LayoutPlanner()
{
}

void LayoutPlanner::initPlaceByAlignRelation()
{
	// geometry align of the matched nodes
	for (int mi = 0; mi < m_matchedSg->m_nodeNum; mi++)
	{
		SemNode& m_matchedSgNode = m_matchedSg->m_nodes[mi];
		if (!m_matchedSgNode.isAligned && m_matchedSgNode.nodeType == "relation")
		{
			int mRefNodeId, mActiveNodeId;

			if (!m_matchedSg->findRefNodeForRelationNode(m_matchedSgNode, mRefNodeId, mActiveNodeId))
			{
				break;
			}

			int currRefNodeId = m_matchedSg->m_toNewSgNodeIdMap[mRefNodeId];
			int currActiveNodeId = m_matchedSg->m_toNewSgNodeIdMap[mActiveNodeId];

			// find ref models
			MetaModel &mRefModel = m_matchedSg->getModelWithNodeId(mRefNodeId);
			MetaModel &tarRefModel = m_currSg->getModelWithNodeId(currRefNodeId);

			MetaModel &currActiveModel = m_currSg->getModelWithNodeId(currActiveNodeId);
			MetaModel &mActiveModel = m_matchedSg->getModelWithNodeId(mActiveNodeId);

			// compute transformation matrix based on the ref models
			// initial alignment; align the rotation etc.	
			mat4 alignTransMat = computeTransMat(mRefModel, tarRefModel);

			// find the target position on new ref obj using the U, V w.r.t the original parent
			vec3 mUVH = mActiveModel.parentPlaneUVH;

			vec3 initPositionInScene = currActiveModel.position; // get the pos of model in current scene

			// find the position after initial alignment
			vec3 alignedPosition = TransformPoint(alignTransMat, initPositionInScene); // position after initial alignment

			SuppPlane& tarRefSuppPlane = tarRefModel.suppPlane;
			vec3 targetPosition = tarRefSuppPlane.getPointByUV(mUVH.x, mUVH.y); // position in the current scene, support plane is already transformed
			//for (int ci = 0; ci < 4; ci++)
			//{
			//	qDebug() << QString("corner%1 %2 %3 %4").arg(ci).arg(tarRefSuppPlane.m_corners[ci].x).arg(tarRefSuppPlane.m_corners[ci].y).arg(tarRefSuppPlane.m_corners[ci].z) << "\n";
			//}

			vec3 translationVec = targetPosition - alignedPosition;

			mat4 adjustTransMat;
			adjustTransMat = adjustTransMat.translate(translationVec);

			mat4 finalTransMat = adjustTransMat*alignTransMat;

			// transform active model by initial alignment
			// initial alignment will make the relative orientation between the new active model and the target active model right
			currActiveModel.updateWithTransform(finalTransMat);

			//qDebug() << QString("Preview:%1 Query anchor:%2").arg(m_matchedSg->m_matchListId).arg(toQString(mRefModel.catName));
			//mRefModel.transformation.print();
			//qDebug() << QString("Preview:%1 Current anchor:%2").arg(m_matchedSg->m_matchListId).arg(toQString(tarRefModel.catName));
			//tarRefModel.transformation.print();

			//qDebug() << QString("Preview:%1 Current active:%2").arg(m_matchedSg->m_matchListId).arg(toQString(newActiveModel.catName));
			//qDebug() << QString("alignedPos");
			//alignedPosition.print();
			//qDebug() << QString("targetPos");
			//targetPosition.print();
			//qDebug() << QString("Parent UV:%1 %2").arg(mUVH.x).arg(mUVH.y);
			//
			//qDebug() << QString("TSG-Anchor:%1 USG-Anchor:%2 Current:%3").arg(toQString(mRefModel.catName)).arg(toQString(tarRefModel.catName)).arg(toQString(newActiveModel.catName));
		}
	}
}

mat4 LayoutPlanner::computeTransMat(const MetaModel &fromModel, const MetaModel &toModel)
{
	mat4 rotMat = GetRotationMatrix(fromModel.frontDir, toModel.frontDir);
	mat4 transMat = GetTransformationMat(rotMat, fromModel.position, toModel.position);

	return transMat;
}

void LayoutPlanner::adjustPlacement(int metaModelID)
{
	MetaModel &currMd = m_currScene->getMetaModel(metaModelID);
	mat4 transMat;
	vec3 translateVec;

	int parentNodeId = m_currScene->m_ssg->findParentNodeIdForModel(metaModelID);
	int parentMetaModelId = m_currScene->m_ssg->m_objectGraphNodeToModelListIdMap[parentNodeId];

	QString sampleType;

	if (parentNodeId != -1)
	{
		MetaModel &parentMd = m_currScene->getMetaModel(parentMetaModelId);

		SuppPlane &parentSuppPlane = parentMd.suppPlane;
		if (parentSuppPlane.m_isInited)
		{
			sampleType = " on parent-" + m_currScene->m_ssg->m_nodes[parentNodeId].nodeName;

			vec3 currUVH = currMd.parentPlaneUVH; // UV, and H w.r.t to parent support plane
			std::vector<double> stdDevs(2, 0.1);

			bool candiFound = false;
			while (!candiFound)
			{
				vec3 newPos = parentSuppPlane.randomGaussSamplePtByUV(currUVH, stdDevs);

				adjustPlacementForSpecialModel(currMd, newPos);

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

	currMd.updateWithTransform(transMat);

	// update meta model in SSG
	m_currScene->m_ssg->m_metaScene.m_metaModellList[metaModelID] = currMd;

	qDebug() << QString("  Preview:%2 Resolve trial:%1 Type:%3 Vec:(%4,%5,%6) Name:%7").arg(currMd.trialNum).arg(m_currScene->m_previewId).arg(sampleType)
		.arg(translateVec.x*m_sceneMetric).arg(translateVec.y*m_sceneMetric).arg(translateVec.z*m_sceneMetric)
		.arg(toQString(m_currScene->m_ssg->m_metaScene.m_metaModellList[metaModelID].catName));
}

bool LayoutPlanner::isPosCloseToInvalidPos(const vec3 &pos, int metaModelId)
{
	for (int i = 0; i < m_collisionPositions[metaModelId].size(); i++)
	{
		double d = (pos - m_collisionPositions[metaModelId][i]).length();

		if (d < m_closeSampleTh / m_sceneMetric)
		{
			return true;
		}
	}

	// test for over-hang positions

	return false;
}

void LayoutPlanner::updateCollisionPostions(const std::vector<std::vector<vec3>> &collisionPositions)
{
	m_collisionPositions.clear();
	m_collisionPositions = collisionPositions;
}

void LayoutPlanner::adjustPlacementForSpecialModel(const MetaModel &currMd, vec3 &newPos)
{
	if (currMd.catName == "headphones")
	{
		Model *m = m_currScene->getModel(currMd.name);
		vec3 bbRange = m->getBBRange(currMd.transformation);

		// headphone is not aligned consistently
		double zOffset = 0.5*min(bbRange.x, bbRange.y);

		newPos.z += zOffset;
	}
}
