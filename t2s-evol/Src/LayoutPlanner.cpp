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
	loadSpecialModels();
	m_trialNumLimit = 200;
}

LayoutPlanner::~LayoutPlanner()
{
}

void LayoutPlanner::loadSpecialModels()
{
	QString filename = "./SceneDB/special_models.txt";
	QFile inFile(filename);
	QTextStream ifs(&inFile);

	if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;

	while (!ifs.atEnd())
	{
		QString currLine = ifs.readLine();
		QStringList parts = currLine.split(",");
		m_specialModels[parts[0]] = parts[1].toDouble();
	}
	inFile.close();
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

				MetaModel &currActiveMd = currSg->getModelWithNodeId(currActiveNodeId);
				MetaModel &mActiveMd = matchedSg->getModelWithNodeId(mActiveNodeId);

				if (currActiveMd.isAlreadyPlaced) continue;

				// find ref models
				MetaModel &mRefMd = matchedSg->getModelWithNodeId(mRefNodeId);
				MetaModel &currRefMd = currSg->getModelWithNodeId(currRefNodeId);

				// compute dir alignment matrix based on the ref models
				mat4 dirRotMat = GetRotationMatrix(mRefMd.frontDir, currRefMd.frontDir);

				// find the target position on new ref obj using the U, V w.r.t the original parent
				mat4 translateMat = mat4::identitiy();
				if (relNode.nodeName.contains("support") || (relNode.nodeName.contains("on")&& relNode.nodeName!="front"))
				{
					vec3 mUVH = mActiveMd.parentPlaneUVH;
					SuppPlane& tarRefSuppPlane = currRefMd.bbTopPlane;

					vec3 targetPosition = tarRefSuppPlane.getPointByUV(mUVH.x, mUVH.y); // position in the current scene, support plane is already transformed

					// TODO: adjust the Z value 
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
							SuppPlane& tarRefSuppPlane = currRefMd.bbTopPlane;
							vec3 targetPosition = tarRefSuppPlane.getPointByUV(mUVH.x, mUVH.y); // position in the current scene, support plane is already transformed

							// TODO: adjust the Z value 
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
				int currRelNodeId = matchedSg->m_nodeAlignMap[mi];
				SemNode &currRelNode = currSg->m_nodes[currRelNodeId];
				currRelNode.isAligned = true;
			}	
		}
	}
}

// only can be called when all models are loaded
void LayoutPlanner::initPlaceUsingSynthesizedRelations(TSScene *currScene)
{
	SceneSemGraph *currSSG = currScene->m_ssg;

	// all unaligned node will be treated as synthesized
	for (int mi = 0; mi < currSSG->m_nodeNum; mi++)
	{
		SemNode& relNode = currSSG->m_nodes[mi];
		if (!relNode.isAligned && relNode.nodeType == "pair_relation" && !relNode.isSynthesized)
		{
			relNode.isAligned = true;   // for case of inserting existing support relation node into an empty scene
			relNode.isSynthesized = true;
		}
	}

	// collect relation node ids; for model with multiple relations, prefer to use support for initial placement
	std::vector<int> relNodeIds;
	for (int mi = 0; mi < currSSG->m_nodeNum; mi++)
	{
		SemNode& sgNode = currSSG->m_nodes[mi];

		if (sgNode.nodeType=="object")
		{
			MetaModel &md = currSSG->getModelWithNodeId(mi);
			if (md.isAlreadyPlaced) continue;						

			int outRelationNum = sgNode.outEdgeNodeList.size();
			if (outRelationNum == 1)
			{
				relNodeIds.push_back(sgNode.outEdgeNodeList[0]);
			}

			if (outRelationNum >1)
			{
				for (int r=0; r< outRelationNum; r++)
				{
					int relId = sgNode.outEdgeNodeList[r];
					SemNode &relNode = currSSG->m_nodes[relId];
					if (relNode.nodeName.contains("support"))
					{
						relNodeIds.push_back(relId);
						//break;
					}
				}

				for (int r=0; r < outRelationNum; r++)
				{
					int relId = sgNode.outEdgeNodeList[r];
					SemNode &relNode = currSSG->m_nodes[relId];

					if (std::find(relNodeIds.begin(), relNodeIds.end(), relId) == relNodeIds.end())
					{
						relNodeIds.push_back(relId);
					}
				}
			}
		}
	}

	int relNum = relNodeIds.size();
	for (int i = 0; i < relNum; i++)
	{
		int relId = relNodeIds[i];
		SemNode& relNode = currSSG->m_nodes[relId];
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

					SuppPlane &suppPlane = refMd.bbTopPlane;
					vec3 uvh = actMd.parentPlaneUVH;
					vec3 newPos = suppPlane.getPointByUV(uvh.x, uvh.y);

					// TODO: adjust the Z value
					vec3 translateVec;
					translateVec = newPos - actMd.position;
					transMat = transMat.translate(translateVec);
				}

				updateMetaModelInScene(currScene, activeModelId, transMat);
				updatePlacementOfChildren(currScene, activeModelId, transMat);

				int actModelParentId = currScene->m_ssg->m_parentOfModel[activeModelId];
				int refModelParentId = currScene->m_ssg->m_parentOfModel[refModelId];
				// for relation other than support, also move parents of current obj
				// e.g., tv in front of sofa, should move tv stand along with tv
				if (!relNode.nodeName.contains("support") 
					&& relNode.nodeName != "onleft"
					&& relNode.nodeName != "onright"
					&& relNode.nodeName != "oncenter"
					&& actModelParentId!= refModelParentId)
				{
					// only transform parent in xy plane
					transMat.a34 = 0;
					updatePlacementOfParent(currScene, activeModelId, transMat);
				}
			}

			relNode.isAligned = true;
		}
	}

	currScene->m_ssg->m_allSynthNodesInited = true;
}

void LayoutPlanner::computeLayout(TSScene *currScene)
{
	// init placement of synthesized nodes and only init once
	if (currScene->m_ssg != NULL && !currScene->m_ssg->m_allSynthNodesInited)
	{
		initPlaceUsingSynthesizedRelations(currScene);  // init placement of synthesized relations
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

	if (!md.zAdjusted && !md.isAlreadyPlaced) 	adjustZForModel(currScene, metaModelId);
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

	if (!md.zAdjusted && !md.isAlreadyPlaced) 	adjustZForModel(currScene, metaModelId);
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

void LayoutPlanner::adjustZForModel(TSScene *currScene, int metaModelId)
{
	MetaModel &md = currScene->getMetaModel(metaModelId);

	int parentModelId = currScene->m_ssg->m_parentOfModel[metaModelId];
	if (parentModelId != -1)
	{
		double newZ;
		if (currScene->computeZForModel(metaModelId, parentModelId, md.position, newZ))
		{
			// if new Z is higher than previous z or is far low from the original z, e.g., move from high shelf to low shelf
			if (newZ > md.position.z || md.position.z - newZ > 0.05 / params::inst()->globalSceneUnitScale)
			{
				md.zAdjusted = true;
				vec3 translateVec;
				translateVec = vec3(0, 0, newZ - md.position.z);
				mat4 transMat = mat4::translate(translateVec);
				updateMetaModelInScene(currScene, metaModelId, transMat);
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

	for (int i = 0; i < currSSG->m_levelOfObjs.size(); i++)
	{
		// collect explicit models for current level
		std::vector<int> explictModelIds;
		std::vector<int> sortExplictModelIds;
		for (int j = 0; j < currSSG->m_levelOfObjs[i].size(); j++)
		{
			int modelId = currSSG->m_levelOfObjs[i][j];
			MetaModel& md = currScene->getMetaModel(modelId);

			int nodeId = currSSG->getNodeIdWithModelId(modelId);
			SemNode &sgNode = currSSG->m_nodes[nodeId];

			if (!md.isAlreadyPlaced && (sgNode.matchingStatus == SemNode::ExplicitNode || sgNode.isInferred == true))
			{
				explictModelIds.push_back(currSSG->m_levelOfObjs[i][j]);
			}
			sortExplictModelIds = sortModelsByVolume(currScene, explictModelIds);
		}

		orderedIds.insert(orderedIds.end(), sortExplictModelIds.begin(), sortExplictModelIds.end());

		// collect and order implicit models for current level
		std::vector<int> implicitModelIds;
		std::vector<int> sortImlictModelIds;
		for (int j = 0; j < currSSG->m_levelOfObjs[i].size(); j++)
		{
			int modelId = currSSG->m_levelOfObjs[i][j];
			MetaModel &md = currScene->getMetaModel(modelId);

			if (!md.isAlreadyPlaced && std::find(orderedIds.begin(), orderedIds.end(), modelId) == orderedIds.end())
			{
				implicitModelIds.push_back(modelId);
			}
		}
		sortImlictModelIds = sortModelsByVolume(currScene, implicitModelIds);
		orderedIds.insert(orderedIds.end(), sortImlictModelIds.begin(), sortImlictModelIds.end());
	}

	return orderedIds;
}

std::vector<int> LayoutPlanner::sortModelsByVolume(TSScene *currScene, const std::vector<int> &modelIds)
{
	std::vector<std::pair<double, int>> volumeIdPairs;
	int modelNum = modelIds.size();
	for (int i=0; i < modelNum; i++)
	{
		int currModelId = modelIds[i];
		MetaModel &md = currScene->getMetaModel(currModelId);
		Model *currModel = currScene->getModel(md.name);

		double modelVolume = currModel->getVolume(md.transformation);
		volumeIdPairs.push_back(std::make_pair(modelVolume, currModelId));
	}

	std::sort(volumeIdPairs.begin(), volumeIdPairs.end()); // ascending order
	std::reverse(volumeIdPairs.begin(), volumeIdPairs.end());

	std::vector<int> sortedIds(modelNum);

	for (int i=0; i< modelNum; i++)
	{
		sortedIds[i] = volumeIdPairs[i].second;
	}

	return sortedIds;
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
	currMd.theta = newPlacement[3];

	vec3 newPos(newPlacement[0], newPlacement[1], newPlacement[2]);
	mat4 transMat = computeTransMatFromPos(currScene, anchorModelId, currModelID, newPos, newPlacement[3]);
	updateMetaModelInScene(currScene, currModelID, transMat);

	// lift model in case the Z position is not accurate
	Model *currModel = currScene->getModel(currMd.name);
	if (currModel!= NULL)
	{
		vec3 bbMin = TransformPoint(currMd.transformation, currModel->m_bb.mi());
		double d = currMd.position.z - bbMin.z;
		if (d > 0)
		{
			mat4 liftMat = mat4::translate(0, 0, d);
			updateMetaModelInScene(currScene, currModelID, liftMat);
		}
	}

	//// update meta model in SSG
	//currScene->m_ssg->m_metaScene.m_metaModellList[currModelID] = currMd;

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
			updateMetaModelInScene(currScene, childModelId, transMat);
		}
	}

	// update active list that is not a child in a group
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
					int actModelId = currSSG->m_graphNodeToModelListIdMap[actNodeId];

					if (std::find(childrenList.begin(), childrenList.end(), actModelId) == childrenList.end())
					{
						updateMetaModelInScene(currScene, actModelId, transMat);
					}
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
		updateMetaModelInScene(currScene, parentModelId, transMat);
	}

	// TODO: update the rest children on current parent model
}

void LayoutPlanner::updateMetaModelInScene(TSScene * currScene, int currModelID, mat4 transMat)
{
	MetaModel &md = currScene->getMetaModel(currModelID);
	md.updateWithTransform(transMat);

	// also need to update meta model in SSG
	currScene->m_ssg->m_metaScene.m_metaModellList[currModelID] = md;
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

	double newZ = newPos.z;
	//adjustZForSpecificModel(currScene, currMd, newZ);
	currScene->adjustZForSpecificModel(currMd, newZ);
	newPos.z = newZ;

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

