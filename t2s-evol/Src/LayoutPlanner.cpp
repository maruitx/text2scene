#include "LayoutPlanner.h"
#include "RelationModelManager.h"
#include "SceneSemGraph.h"
#include "TSScene.h"
#include "Model.h"
#include "CollisionManager.h"
#include "Utility.h"


LayoutPlanner::LayoutPlanner(RelationModelManager *relManager)
	:m_relModelManager(relManager)
{
	m_trialNumLimit = 1000;
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
		if (!m_matchedSgNode.isAligned && m_matchedSgNode.nodeType == "pair_relation")
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
			currActiveModel.theta = GetRotAngleR(tarRefModel.frontDir , currActiveModel.frontDir, vec3(0,0,1));			
		}
	}
}

void LayoutPlanner::computeLayout(TSScene *currScene)
{
	if (currScene->m_toPlaceModelIds.empty())
	{
		currScene->m_toPlaceModelIds = makeToPlaceModelIds(currScene);		
	}

	if (currScene->m_toPlaceModelIds.size() == 1)
	{
		computeSingleObjLayout(currScene, currScene->m_toPlaceModelIds[0]);
	}
	else
	{
		computeGroupObjLayout(currScene, currScene->m_toPlaceModelIds);
	}
}

void LayoutPlanner::computeSingleObjLayout(TSScene *currScene, int metaModelId)
{
	MetaModel &md = currScene->getMetaModel(metaModelId);
	Model *currModel = currScene->getModel(md.name);

	if (currModel == NULL || !currModel->m_loadingDone) return;

	if (!md.isConstranitsExtracted)
	{
		m_relModelManager->collectConstraintsForModel(currScene, metaModelId);
		md.isConstranitsExtracted = true;

		md.layoutPassScore = m_relModelManager->computeLayoutPassScore(currScene, metaModelId);		
	}

	CollisionManager *currCM = currScene->m_collisionManager;

	if (md.trialNum > m_trialNumLimit)
	{
		qDebug() << QString("   Preview %1 Reach test trial limit; Place model anyway; Collision may exist").arg(currScene->m_previewId);
		md.isAlreadyPlaced = true; // reach trial limit, although collision happens still set it to be placed
		return;
	}

	bool isModelCollideWithScene = currCM->checkCollisionBVH(currModel, metaModelId);
	if (isModelCollideWithScene)
	{
		int anchorModelId;
		Eigen::VectorXd newPlacement = computeNewPlacement(currScene, metaModelId, currCM->m_collisionPositions, anchorModelId);
		updateWithNewPlacement(currScene, anchorModelId, metaModelId, newPlacement);
		md.trialNum++;
	}
	else
	{
		//bool isRelationVoilated = m_relModelManager->isRelationsViolated(currScene, metaModelId);

		//if (isRelationVoilated && md.trialNum < m_trialNumLimit)
		//{
		//	adjustPlacement(currScene, metaModelId, currCM->m_collisionPositions);
		//	md.trialNum++;
		//}
		//else
		//{
		//	md.isAlreadyPlaced = true;
		//}


		if (md.layoutScore == 0 && !md.isAlreadyPlaced)
		{
			md.layoutScore = m_relModelManager->computeRelationScore(currScene, metaModelId, makePlacementVec(md.position, md.theta));
		}

		if (md.layoutScore < md.layoutPassScore && md.trialNum < m_trialNumLimit)
		{
			int anchorModelId;
			Eigen::VectorXd newPlacement = computeNewPlacement(currScene, metaModelId, currCM->m_collisionPositions, anchorModelId);			
			double newPlacementScore = m_relModelManager->computeRelationScore(currScene, metaModelId, newPlacement);			

			if (newPlacementScore > md.layoutScore)
			{				
				updateWithNewPlacement(currScene, anchorModelId, metaModelId, newPlacement);
				md.layoutScore = newPlacementScore;
			}

			md.trialNum++;
		}
		else
		{
			md.isAlreadyPlaced = true;
		}
	}
}

void LayoutPlanner::computeGroupObjLayout(TSScene *currScene, const std::vector<int> &modelIds)
{
	for (int i = 0; i < modelIds.size(); i++)
	{
		int metaModelId = modelIds[i];
		MetaModel &md = currScene->getMetaModel(metaModelId);
		Model *currModel = currScene->getModel(md.name);

		if (md.isAlreadyPlaced) continue;	

		if (currModel == NULL || !currModel->m_loadingDone) return;

		if (!md.isConstranitsExtracted)
		{
			m_relModelManager->collectConstraintsForModel(currScene, metaModelId);
			md.isConstranitsExtracted = true;

			md.layoutPassScore = m_relModelManager->computeLayoutPassScore(currScene, metaModelId);
		}

		CollisionManager *currCM = currScene->m_collisionManager;

		if (md.trialNum > m_trialNumLimit)
		{
			qDebug() << QString("   Preview %1 Reach test trial limit; Place model anyway; Collision may exist").arg(currScene->m_previewId);
			md.isAlreadyPlaced = true; // reach trial limit, although collision happens still set it to be placed
			continue;
		}

		//bool isModelCollideWithScene = currCM->checkCollisionBVH(currModel, metaModelId);
		//if (isModelCollideWithScene)
		//{
		//	int anchorModelId;
		//	Eigen::VectorXd newPlacement = computeNewPlacement(currScene, metaModelId, currCM->m_collisionPositions, anchorModelId);
		//	updateWithNewPlacement(currScene, anchorModelId, metaModelId, newPlacement);
		//	md.trialNum++;
		//}
		//else
		//{
		//	bool isRelationVoilated = m_relModelManager->isRelationsViolated(currScene, metaModelId);

		//	if (isRelationVoilated && md.trialNum < m_trialNumLimit)
		//	{
		//		int anchorModelId;
		//		Eigen::VectorXd newPlacement = computeNewPlacement(currScene, metaModelId, currCM->m_collisionPositions, anchorModelId);
		//		updateWithNewPlacement(currScene, anchorModelId, metaModelId, newPlacement);
		//		md.trialNum++;
		//	}
		//	else
		//	{
		//		md.isAlreadyPlaced = true;
		//	}
		//}

		bool isModelCollideWithScene = currCM->checkCollisionBVH(currModel, metaModelId);
		if (isModelCollideWithScene)
		{
			int anchorModelId;
			Eigen::VectorXd newPlacement = computeNewPlacement(currScene, metaModelId, currCM->m_collisionPositions, anchorModelId);
			updateWithNewPlacement(currScene, anchorModelId, metaModelId, newPlacement);
			md.trialNum++;
		}
		else
		{
			//bool isRelationVoilated = m_relModelManager->isRelationsViolated(currScene, metaModelId);

			//if (isRelationVoilated && md.trialNum < m_trialNumLimit)
			//{
			//	adjustPlacement(currScene, metaModelId, currCM->m_collisionPositions);
			//	md.trialNum++;
			//}
			//else
			//{
			//	md.isAlreadyPlaced = true;
			//}


			if (md.layoutScore == 0 && !md.isAlreadyPlaced)
			{
				md.layoutScore = m_relModelManager->computeRelationScore(currScene, metaModelId, makePlacementVec(md.position, md.theta));
			}

			if (md.layoutScore < md.layoutPassScore && md.trialNum < m_trialNumLimit)
			{
				int anchorModelId;
				Eigen::VectorXd newPlacement = computeNewPlacement(currScene, metaModelId, currCM->m_collisionPositions, anchorModelId);
				double newPlacementScore = m_relModelManager->computeRelationScore(currScene, metaModelId, newPlacement);

				if (newPlacementScore > md.layoutScore)
				{
					updateWithNewPlacement(currScene, anchorModelId, metaModelId, newPlacement);
					md.layoutScore = newPlacementScore;
				}

				md.trialNum++;
			}
			else
			{
				md.isAlreadyPlaced = true;
			}
		}
	}
}

std::vector<int> LayoutPlanner::makeToPlaceModelIds(TSScene *currScene)
{
	std::vector<int> orderedIds;
	SceneSemGraph *currSSG = currScene->m_ssg;

	if (currSSG == NULL)
	{
		return orderedIds;
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

	orderedIds.insert(orderedIds.end(), explictModelIds.begin(), explictModelIds.end());

	// collect and order implicit models
	std::vector<int> implicitModelIds;
	for (int i = 0; i < currSSG->m_levelOfObjs.size(); i++)
	{
		for (int j = 0; j < currSSG->m_levelOfObjs[i].size(); j++)
		{
			int modelId = currSSG->m_levelOfObjs[i][j];
			MetaModel &md = currScene->getMetaModel(modelId);

			if (!md.isAlreadyPlaced && std::find(orderedIds.begin(), orderedIds.end(), modelId) == orderedIds.end())
			{
				implicitModelIds.push_back(modelId);
			}
		}
	}

	orderedIds.insert(orderedIds.end(), implicitModelIds.begin(), implicitModelIds.end());
	return orderedIds;
}

Eigen::VectorXd LayoutPlanner::computeNewPlacement(TSScene *currScene, int metaModelID, const std::vector<std::vector<vec3>> &collisonPositions, int &anchorModelId)
{
	m_relModelManager->updateCollisionPostions(collisonPositions);

	// propose position from the explicit constraint
	// candidate position is in world frame
	return m_relModelManager->sampleNewPosFromConstraints(currScene, metaModelID, anchorModelId);
}

void LayoutPlanner::updateWithNewPlacement(TSScene *currScene, int anchorModelId, int currModelID, const Eigen::VectorXd &newPlacement)
{
	MetaModel &currMd = currScene->getMetaModel(currModelID);
	//mat4 transMat = currMd.transformation;

	vec3 newPos(newPlacement[0], newPlacement[1], newPlacement[2]);

	mat4 transMat = computeTransMatFromPos(currScene, anchorModelId, currModelID, newPos, newPlacement[3]);
	currMd.updateWithTransform(transMat);
	currMd.theta = newPlacement[3];
	currMd.isBvhReady = false;

	// update meta model in SSG
	currScene->m_ssg->m_metaScene.m_metaModellList[currModelID] = currMd;
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

mat4 LayoutPlanner::computeTransMatFromPos(TSScene *currScene, int anchorModelId, int currModelID, vec3 newPos, double newTheta)
{
	mat4 transMat;

	MetaModel &currMd = currScene->getMetaModel(currModelID);

	mat4 rotMat;
	if (anchorModelId != -1)
	{
		MetaModel &anchorMd = currScene->getMetaModel(anchorModelId);
		vec3 anchorFront = anchorMd.frontDir;

		//mat4 invMat = currMd.transformation.inverse();
		// vec3 initModelDir = TransformVector(invMat, currMd.frontDir);
		//double initTheta = GetRotAngleR(anchorFront, initModelDir, vec3(0, 0, 1));
		// double rotTheta = newTheta - initTheta;

		double rotTheta = newTheta - GetRotAngleR(anchorFront, currMd.frontDir, vec3(0, 0, 1));;

		rotMat = GetRotationMatrix(vec3(0, 0, 1), rotTheta);
	}
	else
	{
		rotMat = mat4::identitiy();
	}

	// test whether candidate pos is valid for all implicit constraint

	adjustPlacementForSpecificModel(currScene, currMd, newPos);

	vec3 translateVec = newPos - TransformPoint(rotMat, currMd.position);
	transMat = rotMat;
	transMat.a14 = translateVec.x;
	transMat.a24 = translateVec.y;
	transMat.a34 = translateVec.z;

	return transMat;
}

mat4 LayoutPlanner::computeModelAlignTransMat(const MetaModel &fromModel, const MetaModel &toModel)
{
	mat4 rotMat = GetRotationMatrix(fromModel.frontDir, toModel.frontDir);
	mat4 transMat = GetTransformationMat(rotMat, fromModel.position, toModel.position);

	return transMat;
}

Eigen::VectorXd LayoutPlanner::makePlacementVec(vec3 pos, double theta)
{
	Eigen::VectorXd newPlacement(4);

	newPlacement[0] = pos.x;
	newPlacement[1] = pos.y;
	newPlacement[2] = pos.z;
	newPlacement[3] = theta;

	return newPlacement;
}

