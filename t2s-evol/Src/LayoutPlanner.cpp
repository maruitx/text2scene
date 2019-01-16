#include "LayoutPlanner.h"
#include "RelationModelManager.h"
#include "SceneSemGraphManager.h"
#include "RelationModel.h"
#include "SceneSemGraph.h"
#include "TextSemGraph.h"
#include "TSScene.h"
#include "Model.h"
#include "CollisionManager.h"
#include "Utility.h"


LayoutPlanner::LayoutPlanner(RelationModelManager *relManager, SceneSemGraphManager *ssgManager)
	:m_relModelManager(relManager), m_sceneSemGraphManager(ssgManager)
{
	loadSpecialModels();
	m_trialNumLimit = 100;
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
					double u = mUVH.x, v = mUVH.y;

					if (mRefMd.catName == "desk" &&mActiveMd.catName == "computermouse")
					{
						if (u < 0.6 || u>0.8) u = 0.7;
						if (v < 0.1 || v>0.5) v = 0.3;
					}

					vec3 targetPosition = tarRefSuppPlane.getPointByUV(u, v); // position in the current scene, support plane is already transformed

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

				if (adjustInitTransfromSpecialModel(currRefMd, currActiveMd))
				{
					finalTransMat = currActiveMd.transformation;  // use the adjusted transform
				}
				
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
				PairwiseRelationModel *pairModel = m_relModelManager->retrievePairwiseModel(currScene, refNodeId, activeNodeId, relNode.nodeName);

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

					vec3 translateVec;
					translateVec = newPos - actMd.position;
					transMat = transMat.translate(translateVec);
				}

				updateMetaModelTransformInScene(currScene, activeModelId, transMat);
				std::vector<int> ignoreList;
				updatePlacementOfChildren(currScene, activeModelId, transMat, ignoreList);

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
					updatePlacementOfParent(currScene, activeModelId, transMat, ignoreList);
				}

				if (isnan(transMat.a11))
				{
					qDebug();
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

	// extract constrains once
	if (!currScene->m_allConstraintsExtracted)
	{
		computeConstraintsForModels(currScene, currScene->m_toPlaceModelIds);
	}

	// compute layout pass score once
	if (!currScene->m_layoutPassScoreComputed)
	{
		computeLayoutPassScoreForModels(currScene, currScene->m_toPlaceModelIds);
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


	if (md.trialNum >= m_trialNumLimit)
	{
		cout << QString("   Preview %1 %2 Reach test trial limit; Model will be skipped for placement").arg(currScene->m_previewId).arg(toQString(md.catName)).toStdString();
		md.isSkipped = true;
		return;
	}

	// update the z value before collision or relation test
	if (!md.zAdjusted && !md.isAlreadyPlaced) 	adjustZForModel(currScene, metaModelId);
	
	// compute an initial layout score for current object
	if (md.layoutScore == -100 && !md.isAlreadyPlaced)
	{
		md.layoutScore = m_relModelManager->computeRelationScore(currScene, metaModelId, makePlacementVec(md.position, md.theta), mat4::identitiy());
	}

	bool isModelCollideWithScene = currCM->checkCollisionBVH(currModel, metaModelId);
	if (isModelCollideWithScene)
	{
		// test if the parent support plane is too crowd
		int parentId = currScene->m_ssg->m_parentOfModel[metaModelId];
		int placedObjNum = currScene->m_placedObjIdsOnParent[parentId].size();
		if (placedObjNum >= 10 && md.trialNum >= 20)
		{
			cout << QString("   Parent plane is too crowd; Model will be skipped for placement").arg(currScene->m_previewId).arg(toQString(md.catName)).toStdString();
			md.isSkipped = true;
			return;
		}

		// sample another position based on the relational model
		int anchorModelId;
		Eigen::VectorXd newPlacement = samplePosWithHigherRelScore(currScene, metaModelId, 10, anchorModelId);
		updateWithNewPlacement(currScene, anchorModelId, metaModelId, newPlacement);
		md.trialNum++;
	}
	else
	{
		if (md.layoutPassScore == 0 || md.layoutScore > md.layoutPassScore)
		{
			md.isAlreadyPlaced = true;
			int parentId = currScene->m_ssg->m_parentOfModel[metaModelId];
			currScene->m_placedObjIdsOnParent[parentId].push_back(metaModelId); // update the occupied region if the object is placed
		}
		else
		{			
			int anchorModelId;
			Eigen::VectorXd newPlacement = samplePosWithHigherRelScore(currScene, metaModelId, 10, anchorModelId);
			updateWithNewPlacement(currScene, anchorModelId, metaModelId, newPlacement);
			md.trialNum++;
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

		if (currModel == NULL || !currModel->m_loadingDone) return;

		if (md.isAlreadyPlaced)
		{
			tempPlacedModelIds.push_back(metaModelId);
			continue;
		}

		if(md.isSkipped) continue;

		restToPlaceModelIds.push_back(metaModelId);
	}

	if (restToPlaceModelIds.empty()) return;

	int metaModelId = restToPlaceModelIds[0];
	MetaModel &md = currScene->getMetaModel(metaModelId);
	Model *currModel = currScene->getModel(md.name);

	if (md.trialNum >= m_trialNumLimit)
	{
		if (md.isJustRollbacked || !doRollback(currScene, tempPlacedModelIds, metaModelId))
		{
			qDebug() << QString("   Preview %1 %2 Reach test trial limit; Model will be skipped for placement").arg(currScene->m_previewId).arg(toQString(md.catName));
			md.isSkipped = true; 
			updateStatesForRestModels(currScene, restToPlaceModelIds);
			return;
		}
	}

	// update the z value before collision or relation test
	if (!md.zAdjusted && !md.isAlreadyPlaced) 	adjustZForModel(currScene, metaModelId);

	// initialize layout score for all temporary placed objects from the group
	if (md.layoutScore == -100 && !md.isAlreadyPlaced)
	{
		md.layoutScore = m_relModelManager->computeRelationScore(currScene, metaModelId, makePlacementVec(md.position, md.theta), mat4::identitiy());
	}
	
	isModelCollideWithScene = currCM->checkCollisionBVH(currModel, metaModelId);
	if (isModelCollideWithScene)
	{
		// test if the parent support plane is too crowd
		int parentId = currScene->m_ssg->m_parentOfModel[metaModelId];
		int placedObjNum = currScene->m_placedObjIdsOnParent[parentId].size();
		if (placedObjNum >= 10 && md.trialNum >= 20)
		{
			cout << QString("   Parent plane is too crowd; Model will be skipped for placement").arg(currScene->m_previewId).arg(toQString(md.catName)).toStdString();
			md.isSkipped = true;
			return;
		}

		int anchorModelId;
		Eigen::VectorXd newPlacement = samplePosWithHigherRelScore(currScene, metaModelId, 10, anchorModelId);
		updateWithNewPlacement(currScene, anchorModelId, metaModelId, newPlacement);
		md.trialNum++;
	}
	else
	{
		if (md.layoutPassScore == 0 || md.layoutScore > md.layoutPassScore)
		{
			md.isAlreadyPlaced = true;
			int parentId = currScene->m_ssg->m_parentOfModel[metaModelId];
			currScene->m_placedObjIdsOnParent[parentId].push_back(metaModelId); // update the occupied region if the object is placed

			updateStatesForRestModels(currScene, restToPlaceModelIds);
		}
		else if(md.trialNum < m_trialNumLimit)
		{
			int anchorModelId;
			Eigen::VectorXd newPlacement = samplePosWithHigherRelScore(currScene, metaModelId, 10, anchorModelId);
			updateWithNewPlacement(currScene, anchorModelId, metaModelId, newPlacement);
			md.trialNum++;
		}
	}
}

bool LayoutPlanner::doRollback(TSScene *currScene, std::vector<int> &tempPlacedIds, int currModelId)
{
	if (tempPlacedIds.empty()) return false;

	int lastPlaceModelId = tempPlacedIds.back();
	MetaModel &lastMd = currScene->getMetaModel(lastPlaceModelId);

	int parentModelId = currScene->m_ssg->m_parentOfModel[lastPlaceModelId];
	if (parentModelId == -1) return false;

	if (lastMd.isJustRollbacked || lastMd.trialNum == m_trialNumLimit) return false;

	lastMd.isAlreadyPlaced = false;
	lastMd.trialNum = 0;
	lastMd.isJustRollbacked = true;
	tempPlacedIds.pop_back();

	// update last placement 
	CollisionManager *currCM = currScene->m_collisionManager;
	currCM->m_collisionPositions[lastPlaceModelId].push_back(lastMd.position);
	int anchorModelId;
	Eigen::VectorXd newPlacement = computeNewPlacement(currScene, lastPlaceModelId, anchorModelId);
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
				updateMetaModelTransformInScene(currScene, metaModelId, transMat);
			}
		}
	}

	else
	{
		if (md.position.z < currScene->m_floorHeight)
		{
			md.zAdjusted = true;
			vec3 translateVec;
			translateVec = vec3(0, 0, currScene->m_floorHeight - md.position.z);
			mat4 transMat = mat4::translate(translateVec);
			updateMetaModelTransformInScene(currScene, metaModelId, transMat);
		}
	}
}

bool LayoutPlanner::adjustInitTransfromSpecialModel(const MetaModel &anchorMd, MetaModel &actMd)
{
	bool isAdjusted = false;

	if (anchorMd.catName == "couch" && actMd.catName == "table")
	{
		double currAngle = GetRotAngleR(anchorMd.frontDir, actMd.frontDir, vec3(0, 0, 1));
		double targetAngle;
		bool needAdjust = false;
		if (currAngle > 0 && currAngle < 0.5*math_pi || currAngle < 0 && currAngle > -0.5*math_pi)
		{
			needAdjust = true;
			targetAngle = 0;
		}

		if(needAdjust)
		{
			mat4 rotMat = GetRotationMatrix(vec3(0, 0, 1), targetAngle - currAngle);
			vec3 targetPosition = actMd.position; // save current position
			vec3 translationVec = targetPosition - rotMat*actMd.position; // pull the model back after rotation
			mat4 translateMat = mat4::translate(translationVec);
			mat4 adjustedMat = translateMat*rotMat;
			actMd.updateWithTransform(adjustedMat);

			isAdjusted = true;
		}
	}

	if (anchorMd.catName == "desk" && (actMd.catName == "computermouse"))
	{
		double currAngle = GetRotAngleR(anchorMd.frontDir, actMd.frontDir, vec3(0, 0, 1));
		double targetAngle;
		bool needAdjust = false;
		if (currAngle > 0 || currAngle < -0.8*math_pi)
		{
			needAdjust = true;
			targetAngle = GenRandomDouble(-math_pi, -0.8*math_pi);
		}

		if (needAdjust)
		{
			// adjust angle
			mat4 rotMat = GetRotationMatrix(vec3(0, 0, 1), targetAngle - currAngle);
			vec3 targetPosition = actMd.position; // save current position
			vec3 translationVec = targetPosition - rotMat*actMd.position; // pull the model back after rotation
			mat4 translateMat = mat4::translate(translationVec);
			mat4 adjustedMat = translateMat*rotMat;
			actMd.updateWithTransform(adjustedMat);

			isAdjusted = true;
		}
	}

	if (anchorMd.catName == "bed" && (actMd.catName == "nightstand"))
	{
		double currAngle = GetRotAngleR(anchorMd.frontDir, actMd.frontDir, vec3(0, 0, 1));
		double targetAngle;
		bool needAdjust = false;
		if (currAngle > 0.1*math_pi || currAngle < -0.8*math_pi)
		{
			needAdjust = true;
			targetAngle = 0;
		}

		if (needAdjust)
		{
			// adjust angle
			mat4 rotMat = GetRotationMatrix(vec3(0, 0, 1), targetAngle - currAngle);
			vec3 targetPosition = actMd.position; // save current position
			vec3 translationVec = targetPosition - rotMat*actMd.position; // pull the model back after rotation
			mat4 translateMat = mat4::translate(translationVec);
			mat4 adjustedMat = translateMat*rotMat;
			actMd.updateWithTransform(adjustedMat);

			isAdjusted = true;
		}
	}

	if (anchorMd.catName == "chair" && (actMd.catName == "fork" || actMd.catName=="plate" || actMd.catName=="knife"))
	{
		double currAngle = GetRotAngleR(anchorMd.frontDir, actMd.frontDir, vec3(0, 0, 1));
		double targetAngle;
		bool needAdjust = false;
		if (currAngle > 0 && currAngle < 0.5*math_pi || currAngle < 0 && currAngle > -0.5*math_pi)
		{
			needAdjust = true;
			targetAngle = 0;
		}

		if (needAdjust)
		{
			// adjust angle
			mat4 rotMat = GetRotationMatrix(vec3(0, 0, 1), targetAngle - currAngle);
			vec3 targetPosition = actMd.position; // save current position
			vec3 translationVec = targetPosition - rotMat*actMd.position; // pull the model back after rotation
			mat4 translateMat = mat4::translate(translationVec);
			mat4 adjustedMat = translateMat*rotMat;
			actMd.updateWithTransform(adjustedMat);

			isAdjusted = true;
		}

		double currXYDist = std::sqrt(std::pow(anchorMd.position.x - actMd.position.x, 2) + std::pow(anchorMd.position.y - actMd.position.y, 2));
		double sceneMetric = params::inst()->globalSceneUnitScale;
		if (currXYDist > 0.6/sceneMetric || currXYDist < 0.5/sceneMetric)
		{
			double transDist = 0.6/sceneMetric-currXYDist;
			vec3 transVec = anchorMd.frontDir*transDist;
			mat4 adjustedMat = mat4::translate(transVec);

			actMd.updateWithTransform(adjustedMat);
			isAdjusted = true;
		}
	}

	return isAdjusted;
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
			sortExplictModelIds = sortModelsByAnchorAct(currScene, sortExplictModelIds);
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

std::vector<int> LayoutPlanner::sortModelsByAnchorAct(TSScene *currScene, const std::vector<int> &modelIds)
{
	std::vector<int> sortedIds;
	std::vector<int> tempIds;

	SceneSemGraph *currSSG = currScene->m_ssg;

	for (int i=0; i < modelIds.size(); i++)
	{
		int currModelId = modelIds[i];
		int currNodeId = currSSG->getNodeIdWithModelId(currModelId);

		if (currSSG->isAnchor(currNodeId))
		{
			sortedIds.push_back(currModelId);
		}
		else
		{
			tempIds.push_back(currModelId);
		}
	}

	sortedIds.insert(sortedIds.end(), tempIds.begin(), tempIds.end());
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
		computeLayoutPassScoreForModel(currScene, metaModelId);
	}

	currScene->m_layoutPassScoreComputed = true;
}

void LayoutPlanner::computeLayoutPassScoreForModel(TSScene *currScene, int metaModelId)
{
	MetaModel &md = currScene->getMetaModel(metaModelId);

	std::vector<RelationConstraint> &exConstraints = currScene->m_explictConstraints[metaModelId];
	std::vector<RelationConstraint> &imConstraints = currScene->m_implicitConstraints[metaModelId];

	double score = 0;
	double ExWeight = m_relModelManager->ExWeight;
	double ratio = 0.1;

	for (int i = 0; i < exConstraints.size(); i++)
	{
		RelationConstraint &relConstraint = exConstraints[i];
		if (relConstraint.relModel->m_GMM != NULL)
		{
			score += ExWeight*relConstraint.relModel->m_GMM->m_probTh[0]* ratio; 
		}
	}

	for (int i = 0; i < imConstraints.size(); i++)
	{
		RelationConstraint &relConstraint = imConstraints[i];
		if (relConstraint.relModel->m_GMM != NULL)
		{
			score += (1 - ExWeight)*relConstraint.relModel->m_GMM->m_probTh[0]* ratio;
		}
	}

	if (score < 1e-3)
	{
		score = 0;
	}

	md.layoutPassScore = score;
}

void LayoutPlanner::updateStatesForRestModels(TSScene *currScene, const std::vector<int> &restModelIds)
{
	// the first model in the vector of restModelIds is the just placed model
	int refModelId = restModelIds[0];
	MetaModel &refMetaModel = currScene->getMetaModel(refModelId);

	// return if refMetaModel has been placed before; meaning the constraint and the pass scores has been re-computed
	// this will happen in the roll-back case
	if (refMetaModel.isJustRollbacked || refMetaModel.isSkipped)
		return;

	// updates states of the rest models to the first model
	for (int i = 1; i < restModelIds.size(); i++)
	{
		int metaModelId = restModelIds[i];

		// update the constraint for the rest model
		m_relModelManager->addConstraintToModel(currScene, metaModelId, refModelId);

		// re-compute the pass score using new constraints
		computeLayoutPassScoreForModel(currScene, metaModelId);
	}
}

Eigen::VectorXd LayoutPlanner::computeNewPlacement(TSScene *currScene, int metaModelID, int &anchorModelId)
{
	// propose position from the explicit constraint
	// candidate position is in world frame
	return m_relModelManager->sampleNewPosFromConstraints(currScene, metaModelID, anchorModelId);
}

Eigen::VectorXd LayoutPlanner::samplePosWithHigherRelScore(TSScene *currScene, int metaModelId, int relTrialNum, int &anchorModelId)
{
	Eigen::VectorXd newPlacement;	
	int relation_trial_num = 0;
	bool is_pos_sampled = false;

	MetaModel &md = currScene->getMetaModel(metaModelId);

	float maxLayoutScore = -1e6;
	Eigen::VectorXd maxPlacement;
	int maxAnchorModelId;

	// search for a new position that will increase the layout score; break the search if the score pass the threshold
	while (relation_trial_num < relTrialNum)
	{
		int tempAnchorModelId;
		Eigen::VectorXd tempNewPlacement = computeNewPlacement(currScene, metaModelId, tempAnchorModelId);
		mat4 newTransMat = computeTransMatFromPos(currScene, tempAnchorModelId, metaModelId,
			vec3(tempNewPlacement[0], tempNewPlacement[1], tempNewPlacement[2]), tempNewPlacement[3]);

		double newPlacementScore = m_relModelManager->computeRelationScore(currScene, metaModelId, tempNewPlacement, newTransMat);

		// for object with only a few observations, the layout pass score will be zero
		if (newPlacementScore >= 0 && md.layoutPassScore == 0)
		{
			//qDebug() << QString("Resample-new score! Preview %1, %2, curr_score:%3, new_score:%4, pass_score:%5").arg(currScene->m_previewId)
			//	.arg(QString(md.catName.c_str())).arg(md.layoutScore).arg(newPlacementScore).arg(md.layoutPassScore);

			newPlacement = tempNewPlacement;
			anchorModelId = tempAnchorModelId;
			md.layoutScore = newPlacementScore;
			is_pos_sampled = true;
			break;
		}

		// keep the placement with higher relation score
		if (newPlacementScore > md.layoutScore)
		{
			//qDebug() << QString("Resample-higher score! Preview %1, %2, curr_score:%3, higher_score:%4, pass_score:%5").arg(currScene->m_previewId)
			//	.arg(QString(md.catName.c_str())).arg(md.layoutScore).arg(newPlacementScore).arg(md.layoutPassScore);

			newPlacement = tempNewPlacement;
			anchorModelId = tempAnchorModelId;
			md.layoutScore = newPlacementScore;

			is_pos_sampled = true;
			break;
		}

		// find the pos for the max score
		if (newPlacementScore >= maxLayoutScore)
		{
			maxPlacement = tempNewPlacement;
			maxAnchorModelId = tempAnchorModelId;
			maxLayoutScore = newPlacementScore;
		}

		relation_trial_num++;
	}

	if (!is_pos_sampled)
	{
		qDebug() << QString("Resample-local max! Preview %1, %2, curr_score:%3, local_max_score:%4, pass_score:%5").arg(currScene->m_previewId)
			.arg(QString(md.catName.c_str())).arg(md.layoutScore).arg(maxLayoutScore).arg(md.layoutPassScore);

		newPlacement = maxPlacement;
		anchorModelId = maxAnchorModelId;
		md.layoutScore = maxLayoutScore;
	}

	return newPlacement;
}

void LayoutPlanner::updateWithNewPlacement(TSScene *currScene, int anchorModelId, int currModelID, const Eigen::VectorXd &newPlacement)
{
	MetaModel &currMd = currScene->getMetaModel(currModelID);
	currMd.theta = newPlacement[3];

	vec3 newPos(newPlacement[0], newPlacement[1], newPlacement[2]);
	mat4 transMat = computeTransMatFromPos(currScene, anchorModelId, currModelID, newPos, newPlacement[3]);

	updateMetaModelTransformInScene(currScene, currModelID, transMat);

	// lift model in case the Z position is not accurate
	Model *currModel = currScene->getModel(currMd.name);
	if (currModel!= NULL)
	{
		vec3 bbMin = TransformPoint(currMd.transformation, currModel->m_bb.mi());
		double d = currMd.position.z - bbMin.z;
		if (d > 0)
		{
			mat4 liftMat = mat4::translate(0, 0, d);
			updateMetaModelTransformInScene(currScene, currModelID, liftMat);
		}
	}

	// update all supported children
	std::vector<int> ignoreList;
	updatePlacementOfChildren(currScene, currModelID, transMat, ignoreList);
}

void LayoutPlanner::updatePlacementOfChildren(TSScene *currScene, int currModelId, mat4 transMat, int ignoreChildId)
{
	std::vector<int> childrenList = currScene->m_ssg->m_childListOfModel[currModelId];
	if (!childrenList.empty())
	{
		for (int i = 0; i < childrenList.size(); i++)
		{
			int childModelId = childrenList[i];
			if (childModelId == ignoreChildId) continue;
			updateMetaModelTransformInScene(currScene, childModelId, transMat);
			updatePlacementOfChildren(currScene, childModelId, transMat);
		}
	}

	SceneSemGraph *currSSG = currScene->m_ssg;
	int anchorNodeId = currSSG->getNodeIdWithModelId(currModelId);
	SemNode &anchorNode = currSSG->m_nodes[anchorNodeId];
	if (!anchorNode.inEdgeNodeList.empty())
	{
		for (int ri = 0; ri < anchorNode.inEdgeNodeList.size(); ri++)
		{
			int relNodeId = anchorNode.inEdgeNodeList[ri];
			SemNode &relNode = currSSG->m_nodes[relNodeId];

			// update obj in active list that is not a support child; may be the act objs in the same group
			if (relNode.nodeType.contains("group"))
			{
				for (int i = 0; i < relNode.activeNodeList.size(); i++)
				{
					int actNodeId = relNode.activeNodeList[i];
					int actModelId = currSSG->m_graphNodeToModelListIdMap[actNodeId];

					if (std::find(childrenList.begin(), childrenList.end(), actModelId) == childrenList.end())
					{
						if (actModelId == ignoreChildId) continue;
						updateMetaModelTransformInScene(currScene, actModelId, transMat);
						updatePlacementOfChildren(currScene, actModelId, transMat);
					}
				}
			}
		}
	}
}

void LayoutPlanner::updatePlacementOfChildren(TSScene *currScene, int currModelId, mat4 transMat, std::vector<int> &ignoreModelList)
{
	SceneSemGraph *currSSG = currScene->m_ssg;
	int anchorNodeId = currSSG->getNodeIdWithModelId(currModelId);
	SemNode &anchorNode = currSSG->m_nodes[anchorNodeId];

	if (!anchorNode.inEdgeNodeList.empty())
	{
		for (int ri = 0; ri < anchorNode.inEdgeNodeList.size(); ri++)
		{
			int relNodeId = anchorNode.inEdgeNodeList[ri];
			SemNode &relNode = currSSG->m_nodes[relNodeId];

			// update obj in active list that is not a support child; may be the act objs in the same group
			//if (relNode.nodeType.contains("group"))
			{
				for (int i = 0; i < relNode.activeNodeList.size(); i++)
				{
					int actNodeId = relNode.activeNodeList[i];
					int actModelId = currSSG->m_graphNodeToModelListIdMap[actNodeId];

					if (std::find(ignoreModelList.begin(), ignoreModelList.end(), actModelId) == ignoreModelList.end())
					{
						ignoreModelList.push_back(actModelId); // model can only be transformed by one relation once
						updateMetaModelTransformInScene(currScene, actModelId, transMat);
						updatePlacementOfChildren(currScene, actModelId, transMat, ignoreModelList);
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
	MetaModel &currMd = currSSG->m_graphMetaScene.m_metaModellList[currModelId];

	if (!childrenList.empty())
	{
		for (int i = 0; i < childrenList.size(); i++)
		{
			int childId = childrenList[i];
			MetaModel &childMd = currSSG->m_graphMetaScene.m_metaModellList[childId];			
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

			// update obj in active list that is not a support child; may be the act objs in the same group
			if (relNode.nodeType.contains("group"))
			{
				for (int i = 0; i < relNode.activeNodeList.size(); i++)
				{
					int actNodeId = relNode.activeNodeList[i];
					int actModelId = currSSG->m_graphNodeToModelListIdMap[actNodeId];

					if (std::find(childrenList.begin(), childrenList.end(), actModelId) == childrenList.end())
					{
						MetaModel &anchorMd = currSSG->getModelWithNodeId(anchorNodeId);
						MetaModel &actMd = currSSG->getModelWithNodeId(actNodeId);
						actMd.updateWithTransform(transMat);
					}
				}
			}
		}
	}
}

void LayoutPlanner::updatePlacementOfParent(TSScene *currScene, int currModelID, mat4 transMat, std::vector<int> &ignoreModelList)
{
	int parentModelId = currScene->m_ssg->m_parentOfModel[currModelID];

	if (parentModelId!=-1)
	{
		MetaModel &parentMd = currScene->getMetaModel(parentModelId);
		if (parentMd.catName == "table") return;

		updateMetaModelTransformInScene(currScene, parentModelId, transMat);

		// update the rest children on current parent model
		ignoreModelList.push_back(currModelID);
		updatePlacementOfChildren(currScene, parentModelId, transMat, ignoreModelList);
		updatePlacementOfParent(currScene, parentModelId, transMat, ignoreModelList);
	}
}

void LayoutPlanner::updateMetaModelTransformInScene(TSScene * currScene, int currModelID, mat4 transMat)
{
	MetaModel &md = currScene->getMetaModel(currModelID);
	
	// do not relocate already placed object
	if (md.isAlreadyPlaced && !md.isTransformedByCommand) return; 

	md.updateWithTransform(transMat);

	// also need to update meta model in SSG
	currScene->m_ssg->m_graphMetaScene.m_metaModellList[currModelID] = md;
	md.isTransformedByCommand = false;
}

void LayoutPlanner::updateMetaModelInstanceInScene(TSScene *currScene, int currModelID, MetaModel newMd)
{
	currScene->m_toPlaceModelIds.push_back(currModelID);

	// compute children's relPos to the original anchor
	MetaModel &currMd = currScene->getMetaModel(currModelID);
	Model *currModel = currScene->getModel(currMd.name);
	
	SceneSemGraph *currSSG = currScene->m_ssg;
	int currNodeId = currSSG->getNodeIdWithModelId(currModelID);
	SemNode &anchorNode = currSSG->m_nodes[currNodeId];

	if (!anchorNode.inEdgeNodeList.empty())
	{
		for (int ri = 0; ri < anchorNode.inEdgeNodeList.size(); ri++)
		{
			int relNodeId = anchorNode.inEdgeNodeList[ri];
			SemNode &relNode = currSSG->m_nodes[relNodeId];
			{
				for (int i = 0; i < relNode.activeNodeList.size(); i++)
				{
					int childNodeId = relNode.activeNodeList[i];
					int childModelId = currSSG->m_graphNodeToModelListIdMap[childNodeId];
					MetaModel &childMd = currScene->getMetaModel(childModelId);

					double theta = GetRotAngleR(currMd.frontDir, childMd.frontDir, vec3());
					
					// compute the rel pos to the original anchor
					RelativePos *newRelPos = new RelativePos();
					m_relModelManager->extractRelPosToAnchor(currScene, currMd, makePlacementVec(childMd.position, theta), newRelPos);

					// convert relative pos to world pos
					mat4 alignMat = m_relModelManager->getModelToUnitboxMat(currModel, currMd);
					mat4 transMat = alignMat.inverse();  // transform from unit frame to world frame
					vec3 newWorldPos = TransformPoint(transMat, newRelPos->pos);

					// update sampled position 
					double newZ = m_relModelManager->findClosestSuppPlaneZ(currScene, childModelId, newWorldPos);
					newWorldPos.z = newZ;
					double newWolrdTheta = newRelPos->theta* math_pi;  // theta in relational model is normalized by pi

					//transMat = computeTransMatFromPos(currScene, currModelID, actModelId, newWorldPos, newWolrdTheta);
					transMat = computeTransMatFromPos(currScene, currMd, childMd, newWorldPos, newWolrdTheta);
					updateMetaModelTransformInScene(currScene, childModelId, transMat);

					childMd.isAlreadyPlaced = false;
					childMd.isBvhReady = false;
					childMd.zAdjusted = false;

					currScene->m_toPlaceModelIds.push_back(childModelId);
				}
			}
		}
	}

	newMd.isAlreadyPlaced = false;
	newMd.isBvhReady = false;
	newMd.isJustReplaced = true;

	currScene->m_sceneLayoutDone = false;
	currScene->m_sceneLoadingDone = false;

	currScene->m_metaScene.m_metaModellList[currModelID] = newMd;
	currScene->m_ssg->m_graphMetaScene.m_metaModellList[currModelID] = newMd;
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

		double rotTheta = newTheta - GetRotAngleR(anchorFront, currMd.frontDir, vec3(0, 0, 1));
		rotMat = GetRotationMatrix(vec3(0, 0, 1), rotTheta);

		if (isnan(rotMat.a11))
		{
			qDebug();
		}
	}
	else
	{
		rotMat = mat4::identitiy();
	}

	// test whether candidate pos is valid for all implicit constraint

	double newZ = newPos.z;
	currScene->adjustZForSpecificModel(currMd, newZ);
	newPos.z = newZ;

	transMat = rotMat;

	vec3 translateVec = newPos - TransformPoint(rotMat, currMd.position);
	transMat.a14 = translateVec.x;
	transMat.a24 = translateVec.y;
	transMat.a34 = translateVec.z;

	if (isnan(transMat.a11))
	{
		qDebug();
	}

	return transMat;
}

mat4 LayoutPlanner::computeTransMatFromPos(TSScene *currScene, MetaModel &anchorMd, MetaModel &currMd, vec3 newPos, double newTheta)
{
	mat4 transMat;
	mat4 rotMat = mat4::identitiy();

	vec3 anchorFront = anchorMd.frontDir;

	// test whether candidate pos is valid for all implicit constraint

	double newZ = newPos.z;
	currScene->adjustZForSpecificModel(currMd, newZ);
	newPos.z = newZ;

	transMat = rotMat;

	vec3 translateVec = newPos - TransformPoint(rotMat, currMd.position);
	transMat.a14 = translateVec.x;
	transMat.a24 = translateVec.y;
	transMat.a34 = translateVec.z;

	if (isnan(transMat.a11))
	{
		qDebug();
	}

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

void LayoutPlanner::executeCommand(TSScene *currScene, const QString &commandName, int directObjNodeId, int targetObjNodeId/*=-1*/)
{
	SceneSemGraph *currSSG = currScene->m_ssg;

	if (commandName == CommandNames[CommandType::Replace])
	{
		int directModelId = currSSG->m_graphNodeToModelListIdMap[directObjNodeId];
		MetaModel &dirMd = currScene->getMetaModel(directModelId);

		// use the same instance 
		std::vector<int> instanceIds = currSSG->findExistingInstanceIds(toQString(dirMd.catName));

		// find the one that is just replaced
		int justReplacedModelId = -1;
		for (int i=0; i < instanceIds.size(); i++)
		{
			int modelId = instanceIds[i];
			MetaModel &currMd = currSSG->m_graphMetaScene.m_metaModellList[modelId];

			if (currMd.isJustReplaced)
			{
				justReplacedModelId = modelId;
				break;
			}
		}

		if (justReplacedModelId!=-1)
		{
			MetaModel& newMd = currSSG->m_graphMetaScene.m_metaModellList[justReplacedModelId];

			// move newMd to the position of current direct object, and rotate its front dir
			vec3 tarPos = dirMd.position;
			double rotTheta = GetRotAngleR(newMd.frontDir, dirMd.frontDir, vec3(0, 0, 1));

			mat4 rotMat = GetRotationMatrix(vec3(0, 0, 1), rotTheta);
			vec3 translationVec = tarPos - rotMat*newMd.position; // pull the model back after rotation
			mat4 translateMat = mat4::translate(translationVec);
			mat4 transMat = translateMat*rotMat;

			newMd.updateWithTransform(transMat);
			updateMetaModelInstanceInScene(currScene, directModelId, newMd);
		}
		else
		{
			MetaModel &newMd = m_sceneSemGraphManager->retrieveForModelInstance(toQString(dirMd.catName));

			// move newMd to the position of current direct object, and rotate its front dir
			vec3 tarPos = dirMd.position;
			double rotTheta = GetRotAngleR(newMd.frontDir, dirMd.frontDir, vec3(0, 0, 1));

			mat4 rotMat = GetRotationMatrix(vec3(0, 0, 1), rotTheta);
			vec3 translationVec = tarPos - rotMat*newMd.position; // pull the model back after rotation
			mat4 translateMat = mat4::translate(translationVec);
			mat4 transMat = translateMat*rotMat;

			newMd.updateWithTransform(transMat);
			updateMetaModelInstanceInScene(currScene, directModelId, newMd);
		}

		//currScene->m_toPlaceModelIds.clear();
	}

	if (commandName == CommandNames[CommandType::MoveApart])
	{
		double dist = 0.2 / params::inst()->globalSceneUnitScale;
		int directModelId = currSSG->m_graphNodeToModelListIdMap[directObjNodeId];
		MetaModel &dirMd = currScene->getMetaModel(directModelId);

		vec3 transVec = -dirMd.frontDir*dist;
		mat4 transMat = mat4::translate(transVec);
		dirMd.isTransformedByCommand = true;

		updateMetaModelTransformInScene(currScene, directModelId, transMat);
	}

	if (commandName == CommandNames[CommandType::MoveCloser])
	{
		double dist = -0.1 / params::inst()->globalSceneUnitScale;
		int directModelId = currSSG->m_graphNodeToModelListIdMap[directObjNodeId];
		MetaModel &dirMd = currScene->getMetaModel(directModelId);

		vec3 transVec = -dirMd.frontDir*dist;
		mat4 transMat = mat4::translate(transVec);
		dirMd.isTransformedByCommand = true;

		updateMetaModelTransformInScene(currScene, directModelId, transMat);
	}
}

