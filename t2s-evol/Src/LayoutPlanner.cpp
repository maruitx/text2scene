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
	m_trialNumLimit = 200;
}

LayoutPlanner::~LayoutPlanner()
{
}

void LayoutPlanner::initPlaceByAlignRelation(SceneSemGraph *matchedSg, SceneSemGraph *currSg)
{
	currSg->buildSupportHierarchy();

	// geometry align of the matched nodes
	for (int mi = 0; mi < matchedSg->m_nodeNum; mi++)
	{
		SemNode& relNode = matchedSg->m_nodes[mi];
		if (!relNode.isAligned && relNode.nodeType.contains("relation") && !relNode.isSynthesized)
		{
			if (relNode.anchorNodeList.empty()) continue;
	
			int mRefNodeId = relNode.anchorNodeList[0];
			for (int ti=0; ti < relNode.activeNodeList.size(); ti++)
			{
				int mActiveNodeId = relNode.activeNodeList[ti];
				int currRefNodeId = matchedSg->m_nodeAlignMap[mRefNodeId];
				int currActiveNodeId = matchedSg->m_nodeAlignMap[mActiveNodeId];

				//if (!matchedSg->m_nodes[mRefNodeId].isAligned || !currSg->m_nodes[currRefNodeId].isAligned) continue;

				// find ref models
				MetaModel &mRefMd = matchedSg->getModelWithNodeId(mRefNodeId);
				MetaModel &currRefMd = currSg->getModelWithNodeId(currRefNodeId);

				MetaModel &currActiveMd = currSg->getModelWithNodeId(currActiveNodeId);
				MetaModel &mActiveMd = matchedSg->getModelWithNodeId(mActiveNodeId);

				// compute dir alignment matrix based on the ref models
				mat4 dirRotMat = GetRotationMatrix(mRefMd.frontDir, currRefMd.frontDir);

				// find the target position on new ref obj using the U, V w.r.t the original parent
				mat4 translateMat = mat4::identitiy();
				if (relNode.nodeName.contains("support"))
				{
					vec3 mUVH = mActiveMd.parentPlaneUVH;
					SuppPlane& tarRefSuppPlane = currRefMd.suppPlane;
					vec3 targetPosition = tarRefSuppPlane.getPointByUV(mUVH.x, mUVH.y); // position in the current scene, support plane is already transformed

					vec3 initPositionInScene = currActiveMd.position; // get the pos of model in current scene
					vec3 translationVec = targetPosition - dirRotMat*initPositionInScene;
					translateMat = translateMat.translate(translationVec);
				}
				else if (relNode.nodeType.contains("group"))
				{
					PairwiseRelationModel *pairModel = m_relModelManager->retrievePairModelFromGroup(toQString(currRefMd.catName), toQString(currActiveMd.catName), relNode.nodeName, "parentchild");
					if (pairModel!= NULL)
					{
						if (pairModel->m_conditionName.contains("parent"))
						{
							vec3 mUVH = mActiveMd.parentPlaneUVH;
							SuppPlane& tarRefSuppPlane = currRefMd.suppPlane;
							vec3 targetPosition = tarRefSuppPlane.getPointByUV(mUVH.x, mUVH.y); // position in the current scene, support plane is already transformed

							vec3 initPositionInScene = currActiveMd.position; // get the pos of model in current scene
							vec3 translationVec = targetPosition - dirRotMat*initPositionInScene;
							translateMat = translateMat.translate(translationVec);
						}
					}
				}
				else
				{
					vec3 targetPosition = currRefMd.position;
					vec3 translationVec = targetPosition - dirRotMat*mRefMd.position;
					translateMat = translateMat.translate(translationVec);
				}

				mat4 finalTransMat = translateMat*dirRotMat;

				currActiveMd.updateWithTransform(finalTransMat);
				currActiveMd.theta = GetRotAngleR(currRefMd.frontDir, currActiveMd.frontDir, vec3(0, 0, 1));

				int currActiveModelId = currSg->m_graphNodeToModelListIdMap[currActiveNodeId];
				initAlignmentOfChildren(currSg, currActiveModelId, finalTransMat);

				relNode.isAligned = true;
			}	
		}
	}
}

void LayoutPlanner::initPlaceUsingSynthesizedRelations(TSScene *currScene)
{
	SceneSemGraph *currSSG = currScene->m_ssg;

	// all unaligned node will be treated as synthesized
	for (int mi = 0; mi < currSSG->m_nodeNum; mi++)
	{
		SemNode& relNode = currSSG->m_nodes[mi];
		if (!relNode.isAligned && relNode.nodeType == "pair_relation" && !relNode.isSynthesized)
		{
			relNode.isAligned = true;
			relNode.isSynthesized = true;
		}
	}

	for (int mi = 0; mi <currSSG->m_nodeNum; mi++)
	{
		SemNode& relNode = currSSG->m_nodes[mi];
		if (!relNode.isAligned && relNode.nodeType == "pair_relation" && relNode.isSynthesized)
		{
			// edge dir: (active, relation), (relation, reference)

			if (!relNode.anchorNodeList.empty())
			{
				int refNodeId = relNode.anchorNodeList[0];
				int activeNodeId = relNode.activeNodeList[0];

				int refModelId = currSSG->m_graphNodeToModelListIdMap[refNodeId];
				int activeModelId = currSSG->m_graphNodeToModelListIdMap[activeNodeId];

				// compute transformation matrix based on the ref nodes
				MetaModel &refMd = currScene->getMetaModel(refModelId);
				MetaModel &actMd = currScene->getMetaModel(activeModelId);

				Model *refModel = currScene->getModel(refMd.name);
				Model *actModel = currScene->getModel(actMd.name);
				if (!refModel->m_loadingDone || !actModel->m_loadingDone) continue;

				// find relation model and sample from the model
				PairwiseRelationModel *pairModel = m_relModelManager->retrievePairwiseModel(currScene, refMd, actMd, relNode.nodeName);

				mat4 transMat;
				if (pairModel != NULL)
				{
					vec3 newPos;
					double newTheta;
					m_relModelManager->sampleFromRelationModel(currScene, pairModel, activeModelId, refModelId, newPos, newTheta);
					transMat = computeTransMatFromPos(currScene, refModelId, activeModelId, newPos, newTheta);
				}
				else
				{
					// Failure case: don't know the relationship

					SuppPlane &suppPlane = refMd.suppPlane;
					vec3 uvh = actMd.parentPlaneUVH;
					vec3 newPos = suppPlane.getPointByUV(uvh.x, uvh.y);					

					vec3 translateVec;
					translateVec = newPos - actMd.position;
					transMat = transMat.translate(translateVec);					
				}
				double previousActZ = actMd.position.z;
				actMd.updateWithTransform(transMat);
				// update meta model in SSG
				currScene->m_ssg->m_metaScene.m_metaModellList[activeModelId] = actMd;

				updatePlacementOfChildren(currScene, activeModelId, transMat);

				if (!relNode.nodeName.contains("support"))
				{
					// only transform parent in xy plane
					mat4 zMat = mat4::translate(vec3(0,0,previousActZ - actMd.position.z));
					updatePlacementOfParent(currScene, activeModelId, zMat*transMat);
				}
			}

			relNode.isAligned = true;
		}
	}

	int count = 0;
	for (int mi = 0; mi < currSSG->m_nodeNum; mi++)
	{
		SemNode& relNode = currSSG->m_nodes[mi];
		if (!relNode.isAligned && relNode.nodeType == "pair_relation" && relNode.isSynthesized)
		{
			count++;
		}
	}
	if (count==0)
	{
		currScene->m_ssg->m_allSynthNodesInited = true;
	}
}

void LayoutPlanner::computeLayout(TSScene *currScene)
{
	if (currScene->m_ssg!= NULL && !currScene->m_ssg->m_allSynthNodesInited)
	{
		initPlaceUsingSynthesizedRelations(currScene);
	}

	if (currScene->m_toPlaceModelIds.empty())
	{
		currScene->m_toPlaceModelIds = makeToPlaceModelIds(currScene);		
	}

	if (currScene->m_allConstraintsExtracted)
	{
		computeLayoutPassScoreForModels(currScene, currScene->m_toPlaceModelIds);
	}
	else
	{
		computeConstraintsForModels(currScene, currScene->m_toPlaceModelIds);
	}		

	if (currScene->m_toPlaceModelIds.size() == 1)
	{
		computeSingleObjLayout(currScene, currScene->m_toPlaceModelIds[0]);
	}
	else if (currScene->m_toPlaceModelIds.size() > 1)
	{
		computeGroupObjLayoutSeq(currScene, currScene->m_toPlaceModelIds);
	}
}

void LayoutPlanner::computeSingleObjLayout(TSScene *currScene, int metaModelId)
{
	MetaModel &md = currScene->getMetaModel(metaModelId);
	Model *currModel = currScene->getModel(md.name);

	if (currModel == NULL || !currModel->m_loadingDone) return;

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

// compute group layout simultaneously 
void LayoutPlanner::computeGroupObjLayout(TSScene *currScene, const std::vector<int> &toPlaceModelIds)
{
	// test collision	
	CollisionManager *currCM = currScene->m_collisionManager;
	bool isModelCollideWithScene = false;

	for (int i = 0; i < toPlaceModelIds.size(); i++)
	{
		int metaModelId = toPlaceModelIds[i];
		MetaModel &md = currScene->getMetaModel(metaModelId);
		Model *currModel = currScene->getModel(md.name);

		if (md.isAlreadyPlaced) continue;

		if (currModel == NULL || !currModel->m_loadingDone) return;

		if (md.trialNum > m_trialNumLimit)
		{
			qDebug() << QString("   Preview %1 Reach test trial limit; Place model anyway; Collision may exist").arg(currScene->m_previewId);
			md.isAlreadyPlaced = true; // reach trial limit, although collision happens still set it to be placed
			continue;
		}

		isModelCollideWithScene = currCM->checkCollisionBVH(currModel, metaModelId);
		if (isModelCollideWithScene)
		{
			break;
		}
	}


	if (isModelCollideWithScene)
	{
		// re-place all models if any model collides with current scene
		for (int i = 0; i < toPlaceModelIds.size(); i++)
		{
			int metaModelId = toPlaceModelIds[i];
			MetaModel &md = currScene->getMetaModel(metaModelId);
			int anchorModelId;
			Eigen::VectorXd newPlacement = computeNewPlacement(currScene, metaModelId, currCM->m_collisionPositions, anchorModelId);
			updateWithNewPlacement(currScene, anchorModelId, metaModelId, newPlacement);
			md.trialNum++;
		}
	}
	else
	{
		// initialize layout score
		MetaModel &firstMd = currScene->getMetaModel(toPlaceModelIds[0]);
		if (firstMd.layoutScore == 0 && !firstMd.isAlreadyPlaced)
		{
			std::vector<Eigen::VectorXd> currPlacements(toPlaceModelIds.size());
			for (int i = 0; i < toPlaceModelIds.size(); i++)
			{
				int metaModelId = toPlaceModelIds[i];
				MetaModel &md = currScene->getMetaModel(metaModelId);
				currPlacements[i] = makePlacementVec(md.position, md.theta);
				md.tempPlacement = currPlacements[i];
			}

			m_relModelManager->computeRelationScoreForGroup(currScene, toPlaceModelIds, currPlacements);
		}

		// compute current group score and layout pass score
		double currGroupLayoutScore = 0;
		double groupLayoutPassScore = 0;
		for (int i = 0; i < toPlaceModelIds.size(); i++)
		{
			int metaModelId = toPlaceModelIds[i];
			MetaModel &md = currScene->getMetaModel(metaModelId);
			currGroupLayoutScore += md.layoutScore;
			groupLayoutPassScore += md.layoutPassScore;
		}

		if (currGroupLayoutScore < groupLayoutPassScore && firstMd.trialNum < m_trialNumLimit)
		{
			std::vector<Eigen::VectorXd> newPlacements(toPlaceModelIds.size());
			std::vector<int> anchorIds(toPlaceModelIds.size());
			for (int i = 0; i < toPlaceModelIds.size(); i++)
			{
				int metaModelId = toPlaceModelIds[i];
				Eigen::VectorXd newPlacement = computeNewPlacement(currScene, metaModelId, currCM->m_collisionPositions, anchorIds[i]);
				newPlacements[i] = newPlacement;

				MetaModel &md = currScene->getMetaModel(metaModelId);
				md.tempPlacement = newPlacement;
				md.trialNum++;
			}

			double newGroupLayoutScore = m_relModelManager->computeRelationScoreForGroup(currScene, toPlaceModelIds, newPlacements);

			if (newGroupLayoutScore > currGroupLayoutScore)
			{
				for (int i = 0; i < toPlaceModelIds.size(); i++)
				{
					int metaModelId = toPlaceModelIds[i];
					updateWithNewPlacement(currScene, anchorIds[i], metaModelId, newPlacements[i]);
				}
			}
		}
		else
		{
			for (int i = 0; i < toPlaceModelIds.size(); i++)
			{
				int metaModelId = toPlaceModelIds[i];
				MetaModel &md = currScene->getMetaModel(metaModelId);
				md.isAlreadyPlaced = true;
			}
		}
	}
}

// compute group layout sequentially 
void LayoutPlanner::computeGroupObjLayoutSeq(TSScene *currScene, const std::vector<int> &toPlaceModelIds)
{
	// test collision	
	CollisionManager *currCM = currScene->m_collisionManager;
	bool isModelCollideWithScene = false;
	std::vector<int> tempPlacedModelIds;
	std::vector<int> restToPlaceModelIds;

	for (int i = 0; i < toPlaceModelIds.size(); i++)
	{
		int metaModelId = toPlaceModelIds[i];
		MetaModel &md = currScene->getMetaModel(metaModelId);
		Model *currModel = currScene->getModel(md.name);

		if (md.isAlreadyPlaced)
		{
			tempPlacedModelIds.push_back(metaModelId);
			continue;
		}

		if (currModel == NULL || !currModel->m_loadingDone) return;

		restToPlaceModelIds.push_back(metaModelId);
	}

	if (restToPlaceModelIds.empty()) return;

	int metaModelId = restToPlaceModelIds[0];
	MetaModel &md = currScene->getMetaModel(metaModelId);
	Model *currModel = currScene->getModel(md.name);

	isModelCollideWithScene = currCM->checkCollisionBVH(currModel, metaModelId);
	if (isModelCollideWithScene)
	{
		int anchorModelId;
		Eigen::VectorXd newPlacement = computeNewPlacement(currScene, metaModelId, currCM->m_collisionPositions, anchorModelId);
		updateWithNewPlacement(currScene, anchorModelId, metaModelId, newPlacement);
		md.trialNum++;

		if (md.trialNum == m_trialNumLimit)
		{
			if (md.isJustRollbacked || !doRollback(currScene, tempPlacedModelIds, metaModelId))
			{
				qDebug() << QString("   Preview %1 %2 reach test trial limit for collision; collision may exist").arg(currScene->m_previewId).arg(toQString(md.catName));
				md.isAlreadyPlaced = true; // reach trial limit, although collision happens still set it to be placed		
			}	
		}
	}
	else
	{
		// initialize layout score for all temporary placed objects from the group
		if (md.layoutScore == 0 && !md.isAlreadyPlaced)
		{
			tempPlacedModelIds.push_back(metaModelId);
			std::vector<Eigen::VectorXd> currPlacements(tempPlacedModelIds.size());
			for (int ti = 0; ti < tempPlacedModelIds.size(); ti++)
			{
				int tempModelId = tempPlacedModelIds[ti];
				MetaModel &tempMd = currScene->getMetaModel(tempModelId);
				currPlacements[ti] = makePlacementVec(tempMd.position, tempMd.theta);
				tempMd.tempPlacement = currPlacements[ti];
			}

			m_relModelManager->computeRelationScoreForGroup(currScene, tempPlacedModelIds, currPlacements);
		}

		// compute current group score and layout pass score
		double currGroupLayoutScore = 0;
		double groupLayoutPassScore = 0;
		for (int ti = 0; ti < tempPlacedModelIds.size(); ti++)
		{
			int tempModelId = tempPlacedModelIds[ti];
			MetaModel &tempMd = currScene->getMetaModel(tempModelId);
			currGroupLayoutScore += tempMd.layoutScore;
			groupLayoutPassScore += tempMd.layoutPassScore;
		}

		if (currGroupLayoutScore >= groupLayoutPassScore)
		{
			md.isAlreadyPlaced = true;
		}
		else
		{
			int anchorId;
			Eigen::VectorXd newPlacement = computeNewPlacement(currScene, metaModelId, currCM->m_collisionPositions, anchorId);
			md.tempPlacement = newPlacement;
			md.trialNum++;

			std::vector<Eigen::VectorXd> newPlacements(tempPlacedModelIds.size());
			newPlacements.back() = newPlacement;

			for (int ti = 0; ti < tempPlacedModelIds.size() - 1; ti++)
			{
				MetaModel &tempMd = currScene->getMetaModel(tempPlacedModelIds[ti]);
				newPlacements[ti] = makePlacementVec(tempMd.position, tempMd.theta);
			}

			double newGroupLayoutScore = m_relModelManager->computeRelationScoreForGroup(currScene, tempPlacedModelIds, newPlacements);
			if (newGroupLayoutScore > currGroupLayoutScore && md.trialNum < m_trialNumLimit)
			{
				updateWithNewPlacement(currScene, anchorId, metaModelId, newPlacements.back());
			}
			else if(md.trialNum == m_trialNumLimit)
			{
				if (md.isJustRollbacked || !doRollback(currScene, tempPlacedModelIds, metaModelId))
				{
					qDebug() << QString("   Preview %1 %2 reach test trial limit for relationship; relation may violate").arg(currScene->m_previewId).arg(toQString(md.catName));
					updateWithNewPlacement(currScene, anchorId, metaModelId, newPlacements.back());
					md.isAlreadyPlaced = true; // reach trial limit, although collision happens still set it to be placed		
				}
			}
		}
	}
}

bool LayoutPlanner::doRollback(TSScene *currScene, std::vector<int> &tempPlacedIds, int currModelId)
{
	if (tempPlacedIds.empty()) return false;

	int lastPlaceModelId = tempPlacedIds.back();
	MetaModel &lastMd = currScene->getMetaModel(lastPlaceModelId);

	if (lastMd.isJustRollbacked || lastMd.trialNum == m_trialNumLimit) return false;

	lastMd.isAlreadyPlaced = false;
	lastMd.trialNum = 0;
	lastMd.isJustRollbacked = true;
	tempPlacedIds.pop_back();

	// update last placement 
	CollisionManager *currCM = currScene->m_collisionManager;
	currCM->m_collisionPositions[lastPlaceModelId].push_back(lastMd.position);
	int anchorModelId;
	Eigen::VectorXd newPlacement = computeNewPlacement(currScene, lastPlaceModelId, currCM->m_collisionPositions, anchorModelId);
	updateWithNewPlacement(currScene, anchorModelId, lastPlaceModelId, newPlacement);

	MetaModel &md = currScene->getMetaModel(currModelId);
	md.trialNum = 0;

	return true;
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

void LayoutPlanner::computeConstraintsForModels(TSScene *currScene, const std::vector<int> &toPlaceModelIds)
{
	for (int i = 0; i < toPlaceModelIds.size(); i++)
	{
		int metaModelId = toPlaceModelIds[i];
		MetaModel &md = currScene->getMetaModel(metaModelId);

		if (!md.isConstraintExtracted)
		{
			m_relModelManager->collectConstraintsForModel(currScene, metaModelId);
		}		
	}

	int count = 0;
	for (int i=0; i < toPlaceModelIds.size(); i++)
	{
		int metaModelId = toPlaceModelIds[i];
		MetaModel &md = currScene->getMetaModel(metaModelId);
		if (md.isConstraintExtracted)
		{
			count++;
		}
	}

	if (count == toPlaceModelIds.size())
	{
		currScene->m_allConstraintsExtracted = true;
	}
}

void LayoutPlanner::computeLayoutPassScoreForModels(TSScene *currScene, const std::vector<int> &toPlaceModelIds)
{
	for (int i = 0; i < toPlaceModelIds.size(); i++)
	{
		int metaModelId = toPlaceModelIds[i];
		MetaModel &md = currScene->getMetaModel(metaModelId);
		md.layoutPassScore = m_relModelManager->computeLayoutPassScore(currScene, metaModelId);
	}
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

	// lift model in case the Z position is not accurate
	Model *currModel = currScene->getModel(currMd.name);
	if (currModel!= NULL)
	{
		vec3 bbMin = TransformPoint(currMd.transformation, currModel->m_bb.mi());
		double d = currMd.position.z - bbMin.z;
		if (d > 0)
		{
			mat4 liftMat = mat4::translate(0, 0, d);
			currMd.updateWithTransform(liftMat);
		}
	}

	// update meta model in SSG
	currScene->m_ssg->m_metaScene.m_metaModellList[currModelID] = currMd;

	// update all supported children
	updatePlacementOfChildren(currScene, currModelID, transMat);
}

void LayoutPlanner::updatePlacementOfChildren(TSScene *currScene, int currModelId, mat4 transMat)
{
	std::vector<int> childrenList = currScene->m_ssg->m_childListOfModel[currModelId];
	if (!childrenList.empty())
	{
		for (int i = 0; i < childrenList.size(); i++)
		{
			int childModelId = childrenList[i];
			MetaModel &childMd = currScene->getMetaModel(childModelId);
			childMd.updateWithTransform(transMat);

			// update MetaModel in SSG
			currScene->m_ssg->m_metaScene.m_metaModellList[childModelId] = childMd;
		}
	}

	// update active list in a group
	SceneSemGraph *currSSG = currScene->m_ssg;
	int anchorNodeId = currSSG->getNodeIdWithModelId(currModelId);
	SemNode &anchorNode = currSSG->m_nodes[anchorNodeId];
	if (!anchorNode.inEdgeNodeList.empty())
	{
		for (int ri = 0; ri < anchorNode.inEdgeNodeList.size(); ri++)
		{
			int relNodeId = anchorNode.inEdgeNodeList[ri];
			SemNode &relNode = currSSG->m_nodes[relNodeId];

			if (relNode.nodeType.contains("group"))
			{
				for (int i = 0; i < relNode.activeNodeList.size(); i++)
				{
					int actNodeId = relNode.activeNodeList[i];
					MetaModel &actMd = currSSG->getModelWithNodeId(actNodeId);
					actMd.updateWithTransform(transMat);

					int actModelId = currSSG->m_graphNodeToModelListIdMap[actNodeId];
					MetaModel &actMdInScene = currScene->getMetaModel(actModelId);
					actMdInScene = actMd;
				}
			}
		}
	}
}

void LayoutPlanner::initAlignmentOfChildren(SceneSemGraph *currSSG, int currModelId, mat4 transMat)
{
	// update support children
	std::vector<int> childrenList = currSSG->m_childListOfModel[currModelId];
	if (!childrenList.empty())
	{
		for (int i = 0; i < childrenList.size(); i++)
		{
			int childId = childrenList[i];
			MetaModel &childMd = currSSG->m_metaScene.m_metaModellList[childId];			
			childMd.updateWithTransform(transMat);
		}
	}

	// update active list in a group 
	int anchorNodeId = currSSG->getNodeIdWithModelId(currModelId);
	SemNode &anchorNode = currSSG->m_nodes[anchorNodeId];
	if (!anchorNode.inEdgeNodeList.empty())
	{
		for (int ri=0; ri < anchorNode.inEdgeNodeList.size(); ri++)
		{
			int relNodeId = anchorNode.inEdgeNodeList[ri];
			SemNode &relNode = currSSG->m_nodes[relNodeId];

			if (relNode.nodeType.contains("group"))
			{
				for (int i = 0; i < relNode.activeNodeList.size(); i++)
				{
					int actNodeId = relNode.activeNodeList[i];
					MetaModel &actMd = currSSG->getModelWithNodeId(actNodeId);
					actMd.updateWithTransform(transMat);
				}
			}
		}
	}
}

void LayoutPlanner::updatePlacementOfParent(TSScene *currScene, int currModelID, mat4 transMat)
{
	int parentModelId = currScene->m_ssg->m_parentOfModel[currModelID];

	if (parentModelId!=-1)
	{
		MetaModel &parentMd = currScene->getMetaModel(parentModelId);
		parentMd.updateWithTransform(transMat);

		// update MetaModel in SSG
		currScene->m_ssg->m_metaScene.m_metaModellList[parentModelId] = parentMd;
	}

	// TODO: update the rest children on current parent model
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

