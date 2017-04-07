#include "LayoutPlanner.h"
#include "RelationModelManager.h"
#include "SceneSemGraph.h"
#include "TSScene.h"
#include "Model.h"
#include "Utility.h"


LayoutPlanner::LayoutPlanner(RelationModelManager *relManager)
	:m_relModelManager(relManager)
{
	m_closeSampleTh = 0.03;
	m_sceneMetric = params::inst()->globalSceneUnitScale;
}

LayoutPlanner::~LayoutPlanner()
{
}

void LayoutPlanner::initPlaceByAlignRelation(SceneSemGraph *matchedSg, SceneSemGraph *currSg)
{
	// geometry align of the matched nodes
	for (int mi = 0; mi < matchedSg->m_nodeNum; mi++)
	{
		SemNode& m_matchedSgNode = matchedSg->m_nodes[mi];
		if (!m_matchedSgNode.isAligned && m_matchedSgNode.nodeType == "relation")
		{
			int mRefNodeId, mActiveNodeId;

			if (!matchedSg->findRefNodeForRelationNode(m_matchedSgNode, mRefNodeId, mActiveNodeId))
			{
				break;
			}

			int currRefNodeId = matchedSg->m_toNewSgNodeIdMap[mRefNodeId];
			int currActiveNodeId = matchedSg->m_toNewSgNodeIdMap[mActiveNodeId];

			// find ref models
			MetaModel &mRefModel = matchedSg->getModelWithNodeId(mRefNodeId);
			MetaModel &tarRefModel = currSg->getModelWithNodeId(currRefNodeId);

			MetaModel &currActiveModel = currSg->getModelWithNodeId(currActiveNodeId);
			MetaModel &mActiveModel = matchedSg->getModelWithNodeId(mActiveNodeId);

			// compute dir alignment matrix based on the ref models
			mat4 dirRotMat = GetRotationMatrix(mRefModel.frontDir, tarRefModel.frontDir);

			// find the target position on new ref obj using the U, V w.r.t the original parent
			vec3 mUVH = mActiveModel.parentPlaneUVH;
			SuppPlane& tarRefSuppPlane = tarRefModel.suppPlane;
			vec3 targetPosition = tarRefSuppPlane.getPointByUV(mUVH.x, mUVH.y); // position in the current scene, support plane is already transformed
			//for (int ci = 0; ci < 4; ci++)
			//{
			//	qDebug() << QString("corner%1 %2 %3 %4").arg(ci).arg(tarRefSuppPlane.m_corners[ci].x).arg(tarRefSuppPlane.m_corners[ci].y).arg(tarRefSuppPlane.m_corners[ci].z) << "\n";
			//}

			vec3 initPositionInScene = currActiveModel.position; // get the pos of model in current scene
			vec3 translationVec = targetPosition - dirRotMat*initPositionInScene;

			mat4 translateMat;
			translateMat = translateMat.translate(translationVec);

			//mat4 finalTransMat = adjustTransMat*alignTransMat;
			mat4 finalTransMat = translateMat*dirRotMat;

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

std::vector<int> LayoutPlanner::makePlacementOrder(TSScene *currScene)
{
	std::vector<int> orderedIds;
	SceneSemGraph *currSSG = currScene->m_ssg;

	if (currSSG == NULL)
	{
		return orderedIds;
	}

	// first collect already placed models
	for (int i=0; i < currSSG->m_levelOfObjs.size(); i++)
	{
		for (int j=0;  j< currSSG->m_levelOfObjs[i].size(); j++)
		{
			int modelId = currSSG->m_levelOfObjs[i][j];
			MetaModel& md = currScene->getMetaModel(modelId);

			if (md.isAlreadyPlaced)
			{
				orderedIds.push_back(modelId);
			}			
		}
	}

	// collect explicit models
	std::vector<int> explictModelIds;
	for (int i = 0; i < currSSG->m_levelOfObjs.size(); i++)
	{
		for (int j = 0; j < currSSG->m_levelOfObjs[i].size(); j++)
		{
			int modelId = currSSG->m_levelOfObjs[i][j];
			MetaModel& md = currScene->getMetaModel(modelId);

			int nodeId = currSSG->getNodeIdWithModelId(modelId);
			SemNode &sgNode = currSSG->m_nodes[nodeId];

			if (!md.isAlreadyPlaced && sgNode.matchingStatus == SemNode::ExplicitNode)
			{
				explictModelIds.push_back(currSSG->m_levelOfObjs[i][j]);
			}
		}
	}

	////  
	//for (int i=0; i < explictModelIds.size(); i++)
	//{
	//	int modelId = explictModelIds[i];
	//	int nodeId = currSSG->getNodeIdWithModelId(modelId);		
	//}

	orderedIds.insert(orderedIds.end(), explictModelIds.begin(), explictModelIds.end());

	// collect and order implicit models
	std::vector<int> implicitModelIds;
	for (int i = 0; i < currSSG->m_levelOfObjs.size(); i++)
	{
		for (int j = 0; j < currSSG->m_levelOfObjs[i].size(); j++)
		{
			int modelId = currSSG->m_levelOfObjs[i][j];		

			if (std::find(orderedIds.begin(), orderedIds.end(), modelId) == orderedIds.end())
			{
				implicitModelIds.push_back(modelId);
			}
		}
	}

	orderedIds.insert(orderedIds.end(), implicitModelIds.begin(), implicitModelIds.end());
	return orderedIds;
}

mat4 LayoutPlanner::computeTransMat(const MetaModel &fromModel, const MetaModel &toModel)
{
	mat4 rotMat = GetRotationMatrix(fromModel.frontDir, toModel.frontDir);
	mat4 transMat = GetTransformationMat(rotMat, fromModel.position, toModel.position);

	return transMat;
}

void LayoutPlanner::adjustPlacement(TSScene *currScene, int metaModelID, const std::vector<std::vector<vec3>> &collisonPositions)
{
	updateCollisionPostions(collisonPositions);

	MetaModel &currMd = currScene->getMetaModel(metaModelID);

	bool collisionFlag = true;
	bool implicitFlag = true;

	mat4 transMat;

	while (collisionFlag)
	{
		// propose position from the explicit constraint
		// candidate position in world frame
		int anchorModelId;
		Eigen::VectorXd candiPos = m_relModelManager->sampleFromExplicitRelation(currScene, metaModelID, anchorModelId);
		vec3 newPos(candiPos[0], candiPos[1], candiPos[2]);
		adjustPlacementForSpecificModel(currScene, currMd, newPos);

		if (!isPosCloseToInvalidPos(newPos, metaModelID))
		{
			collisionFlag = false;

			//
			MetaModel &anchorMd = currScene->getMetaModel(anchorModelId);
			vec3 anchorFront = anchorMd.frontDir;

			double initTheta = GetRotAngleR(anchorFront, currMd.frontDir, vec3(0,0,1));
			double currTheta = candiPos[3];
			double rotTheta = currTheta - initTheta;

			mat4 rotMat = GetRotationMatrix(vec3(0, 0, 1), rotTheta);

			// test whether candidate pos is valid for all implicit constraint
			vec3 translateVec = newPos - TransformPoint(rotMat, currMd.position);
			transMat = rotMat;
			transMat.a14 = translateVec.x;
			transMat.a24 = translateVec.y;
			transMat.a34 = translateVec.z;

			break;
		}
	}


	//vec3 translateVec;

	//int parentNodeId = currScene->m_ssg->findParentNodeIdForModel(metaModelID);
	//int parentMetaModelId = currScene->m_ssg->m_graphNodeToModelListIdMap[parentNodeId];

	//QString sampleType;

	//if (parentNodeId != -1)
	//{
	//	MetaModel &parentMd = currScene->getMetaModel(parentMetaModelId);

	//	SuppPlane &parentSuppPlane = parentMd.suppPlane;
	//	if (parentSuppPlane.m_isInited)
	//	{
	//		sampleType = " on parent-" + currScene->m_ssg->m_nodes[parentNodeId].nodeName;

	//		vec3 currUVH = currMd.parentPlaneUVH; // UV, and H w.r.t to parent support plane
	//		std::vector<double> stdDevs(2, 0.1);

	//		bool candiFound = false;
	//		while (!candiFound)
	//		{
	//			vec3 newPos = parentSuppPlane.randomGaussSamplePtByUV(currUVH, stdDevs);

	//			adjustPlacementForSpecificModel(currScene, currMd, newPos);

	//			if (!isPosCloseToInvalidPos(newPos, metaModelID))
	//			{
	//				candiFound = true;
	//				translateVec = newPos - currMd.position;
	//			}
	//		}
	//	}
	//}
	//else
	//{
	//	sampleType = "on floor";

	//	std::vector<double> shiftVals;
	//	std::vector<double> dMeans(2, 0); // set mean to be (0,0)
	//	std::vector<double> stdDevs(2, 0.2);

	//	bool candiFound = false;
	//	while (!candiFound)
	//	{
	//		GenNNormalDistribution(dMeans, stdDevs, shiftVals);
	//		translateVec = vec3(shiftVals[0] / m_sceneMetric, shiftVals[1] / m_sceneMetric, 0);

	//		vec3 newPos = currMd.position + translateVec;
	//		if (!isPosCloseToInvalidPos(newPos, metaModelID))
	//		{
	//			candiFound = true;
	//		}
	//	}
	//}

	//transMat = transMat.translate(translateVec);
	
	currMd.updateWithTransform(transMat);

	// update meta model in SSG
	currScene->m_ssg->m_metaScene.m_metaModellList[metaModelID] = currMd;

	//qDebug() << QString("  Preview:%2 Resolve trial:%1 Type:%3 Vec:(%4,%5,%6) Name:%7").arg(currMd.trialNum).arg(m_currScene->m_previewId).arg(sampleType)
	//	.arg(translateVec.x*m_sceneMetric).arg(translateVec.y*m_sceneMetric).arg(translateVec.z*m_sceneMetric)
	//	.arg(toQString(m_currScene->m_ssg->m_metaScene.m_metaModellList[metaModelID].catName));
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

void LayoutPlanner::adjustPlacementForSpecificModel(TSScene *currScene, const MetaModel &currMd, vec3 &newPos)
{
	if (currMd.catName == "headphones")
	{
		Model *m = currScene->getModel(currMd.name);
		vec3 bbRange = m->getBBRange(currMd.transformation);

		// headphone is not aligned consistently
		double zOffset = 0.5*min(bbRange.x, bbRange.y);

		newPos.z += zOffset;
	}
}
