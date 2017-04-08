#include "RelationModelManager.h"
#include "TSScene.h"
#include "Model.h"
#include "SceneSemGraph.h"
#include <Eigen/Dense>

RelationModelManager::RelationModelManager()
{
	loadRelationModels();

	m_trialNumLimit = 100;
}

RelationModelManager::~RelationModelManager()
{

}

void RelationModelManager::loadRelationModels()
{
	loadRelativeRelationModels();
	loadPairwiseRelationModels();
	loadGroupRelationModels();
}

void RelationModelManager::loadRelativeRelationModels()
{
	QString sceneDBPath = "./SceneDB";
	QString filename = sceneDBPath + "/Relative.model";

	QFile inFile(filename);
	QTextStream ifs(&inFile);

	if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;

	while (!ifs.atEnd())
	{
		QString currLine = ifs.readLine();
		std::vector<std::string> parts = PartitionString(currLine.toStdString(), "_");

		QString anchorObjName = toQString(parts[0]);
		QString actObjName = toQString(parts[1]);
		QString conditionName = toQString(parts[2]);
		QString relationName = toQString(parts[3]);

		PairwiseRelationModel *newRelModel = new PairwiseRelationModel(anchorObjName, actObjName, conditionName, relationName);
		m_relativeModels[newRelModel->m_relationKey] = newRelModel;

		newRelModel->loadFromStream(ifs);
	}
}

void RelationModelManager::loadPairwiseRelationModels()
{
	QString sceneDBPath = "./SceneDB";
	QString filename = sceneDBPath + "/Pairwise.model";

	QFile inFile(filename);
	QTextStream ifs(&inFile);

	if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;

	while (!ifs.atEnd())
	{
		QString currLine = ifs.readLine();
		std::vector<std::string> parts = PartitionString(currLine.toStdString(), "_");

		QString anchorObjName = toQString(parts[0]);
		QString actObjName = toQString(parts[1]);
		QString conditionName = toQString(parts[2]);
		QString relationName = toQString(parts[3]);

		PairwiseRelationModel *newRelModel = new PairwiseRelationModel(anchorObjName, actObjName, conditionName, relationName);
		m_pairwiseRelModels[newRelModel->m_relationKey] = newRelModel;

		newRelModel->loadFromStream(ifs);
	}
}

void RelationModelManager::loadGroupRelationModels()
{
	QString sceneDBPath = "./SceneDB";
	QString filename = sceneDBPath + "/Group.model";

	QFile inFile(filename);
	QTextStream ifs(&inFile);

	if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;

	while (!ifs.atEnd())
	{
		QString currLine = ifs.readLine();
		std::vector<std::string> parts = PartitionString(currLine.toStdString(), ",");
		std::vector<std::string> subParts = PartitionString(parts[0], "_");

		QString relationName = toQString(subParts[0]);
		QString anchorObjName = toQString(subParts[1]);
		GroupRelationModel *newGroupModel = new GroupRelationModel(anchorObjName, relationName);
		newGroupModel->m_numInstance = StringToInt(parts[1]);
		m_groupRelModels[newGroupModel->m_groupKey] = newGroupModel;

		currLine = ifs.readLine();
		if (currLine.contains("occurrence"))
		{
			int occurModelNum = StringToIntegerList(currLine.toStdString(), "occurrence ")[0];
			for (int i=0; i < occurModelNum; i++)
			{
				currLine = ifs.readLine();
				parts = PartitionString(currLine.toStdString(), ",");
				subParts = PartitionString(parts[0], "_");

				OccurrenceModel *newOccurModel = new OccurrenceModel(toQString(subParts[0]), StringToInt(subParts[1]));
				newOccurModel->m_occurProb = StringToFloat(parts[1]);

				newGroupModel->m_occurModels[newOccurModel->m_occurKey] = newOccurModel;
			}
		}

		currLine = ifs.readLine();
		if (currLine.contains("pairwise"))
		{
			int pairModelNum = StringToIntegerList(currLine.toStdString(), "pairwise ")[0];
			
			for (int i=0; i < pairModelNum; i++)
			{
				currLine = ifs.readLine();   // relation key
				parts = PartitionString(currLine.toStdString(), ",");
				subParts = PartitionString(parts[1], "_");

				QString anchorObjName = toQString(subParts[0]);
				QString actObjName = toQString(subParts[1]);
				QString conditionName = toQString(subParts[2]);
				QString relationName = toQString(subParts[3]);

				PairwiseRelationModel *newRelModel = new PairwiseRelationModel(anchorObjName, actObjName, conditionName, relationName);
				newGroupModel->m_pairwiseModels[newRelModel->m_relationKey] = newRelModel;

				newRelModel->loadFromStream(ifs);
			} 
		}
	}

	inFile.close();
}

bool RelationModelManager::isRelationsViolated(TSScene *currScene, int metaModelId)
{
	return false;
	MetaModel &md = currScene->getMetaModel(metaModelId);

	std::vector<RelationConstraint> &exConstraints = currScene->m_explictConstraints[metaModelId];
	std::vector<RelationConstraint> &imConstraints = currScene->m_implicitConstraints[metaModelId];

	for (int i=0;  i < exConstraints.size(); i++)
	{
		if (isConstraintViolated(currScene, md, exConstraints[i]))
		{
			return true;
		}
	}

	//for (int i=0; i< imConstraints.size(); i++)
	//{
	//	if (isConstraintViolated(currScene, md, imConstraints[i]))
	//	{
	//		return true;
	//	}
	//}

	return false;
}

bool RelationModelManager::isConstraintViolated(TSScene *currScene, const MetaModel &md, const RelationConstraint &relConstraint)
{
	PairwiseRelationModel *pairModel = relConstraint.relModel;
	if (pairModel->m_numGauss == 0) return false;

	int anchorObjId = relConstraint.anchorObjId;
	MetaModel &anchorMd = currScene->getMetaModel(anchorObjId);

	RelativePos *newRelPos = new RelativePos();

	extractRelPosForModelPair(currScene, anchorMd, md, newRelPos);

	Eigen::VectorXd observation(4);
	observation[0] = newRelPos->pos.x;
	observation[1] = newRelPos->pos.y;
	observation[2] = newRelPos->pos.z;
	observation[3] = newRelPos->theta;

	double prob = pairModel->m_GMM->probability(observation);
	double exTh = pairModel->m_GMM->m_probTh[0]; // use 20% percentile

	if (prob < exTh)
	{
		return true;
	}

	return false;
}

void RelationModelManager::extractRelPosForModelPair(TSScene *currScene, const MetaModel &anchorMd, const MetaModel &actMd, RelativePos *relPos)
{	
	// get unitize transform for anchor model
	Model *loadedAnchorModel = currScene->getModel(anchorMd.name);
	mat4 alignMat = getModelToUnitboxMat(loadedAnchorModel, anchorMd);

	vec3 actPos = actMd.position;
	vec3 actFront = actMd.frontDir;
	vec3 anchorFront = anchorMd.frontDir;

	relPos->pos = TransformPoint(alignMat, actPos);
	relPos->theta = GetRotAngleR(anchorFront, actFront, vec3(0, 0, 1));
}

mat4 RelationModelManager::getModelToUnitboxMat(Model *m, const MetaModel &md)
{	
	if (!m_loadModelToUnitboxMat.count(md.name))
	{
		mat4 transMat = md.transformation;
		mat4 invTransMat = transMat.inverse();

		vec3 initPos = TransformPoint(invTransMat, md.position);
		vec3 initFront = TransformVector(invTransMat, md.frontDir);

		vec3 bbRange = m->getBBRange();

		mat4 translateMat, scaleMat, rotMat;
		translateMat = mat4::translate(vec3(0, 0, -0.5*bbRange.z) - initPos);
		scaleMat = mat4::scale(1 / bbRange.x, 1 / bbRange.y, 1 / bbRange.z);
		rotMat = GetRotationMatrix(initFront, vec3(0, 1, 0));

		mat4 alignMat = rotMat*scaleMat*translateMat*invTransMat;
		m_loadModelToUnitboxMat[md.name] = alignMat;

		return alignMat; 
	}
	else
	{
		return m_loadModelToUnitboxMat[md.name];
	}
}

Eigen::VectorXd RelationModelManager::sampleNewPosFromConstraints(TSScene *currScene, int metaModelId, int &anchorModelId)
{
	vec3 newPos;
	double newTheta;

	MetaModel &md = currScene->getMetaModel(metaModelId);

	if (!currScene->m_explictConstraints[metaModelId].empty())
	{
		// use the first explicit constraint
		sampleFromRelationModel(currScene, currScene->m_explictConstraints[metaModelId][0], metaModelId, 
			anchorModelId, newPos, newTheta);
	}
	else if(!currScene->m_implicitConstraints[metaModelId].empty())
	{
		sampleFromRelationModel(currScene, currScene->m_implicitConstraints[metaModelId][0], metaModelId,
			anchorModelId, newPos, newTheta);
	}
	else
	{
		randomSampleOnParent(currScene, metaModelId, newPos);
		
		newTheta = 0;
		anchorModelId = -1;  // no anchor obj
	}


	// convert to world frame
	Eigen::VectorXd  relPos(4);
	relPos[0] = newPos.x;
	relPos[1] = newPos.y;
	relPos[2] = newPos.z;  // keep Z value
	relPos[3] = newTheta;   // theta is not affected by transformation	

	return relPos;
}

void RelationModelManager::sampleFromRelationModel(TSScene *currScene, const RelationConstraint &relConstraint, int metaModelId,
	int &anchorModelId, vec3 &newPos, double &newTheta)
{
	// anchor obj																					 
	anchorModelId = relConstraint.anchorObjId;
	MetaModel &anchorMd = currScene->getMetaModel(anchorModelId);
	Model *anchorModel = currScene->getModel(anchorMd.name);
	mat4 alignMat = getModelToUnitboxMat(anchorModel, anchorMd);
	mat4 transMat = alignMat.inverse();  // transform from unit frame to world frame

	// sample in the unit frame
	Eigen::VectorXd newSample = relConstraint.relModel->sample();

	newPos = vec3(newSample[0], newSample[1], newSample[2]);
	newPos = TransformPoint(transMat, newPos);

	// update sampled position 
	double newZ = findClosestSuppPlaneZ(currScene, metaModelId, newPos);
	newPos.z = newZ;

	newTheta = newSample[3];

}

double RelationModelManager::findClosestSuppPlaneZ(TSScene *currScene, int metaModelId, const vec3 &newPos)
{
	int parentNodeId = currScene->m_ssg->findParentNodeIdForModel(metaModelId);
	int parentMetaModelId = currScene->m_ssg->m_graphNodeToModelListIdMap[parentNodeId];

	if (parentMetaModelId != -1)
	{
		MetaModel &parentMd = currScene->getMetaModel(parentMetaModelId);
		SuppPlane &parentSuppPlane = parentMd.suppPlane;
		if (parentSuppPlane.m_isInited)
		{
			return parentSuppPlane.getZ();
		}
	}

	return newPos.z;
}

void RelationModelManager::randomSampleOnParent(TSScene *currScene, int metaModelId, vec3 &newPos)
{
	double m_sceneMetric = params::inst()->globalSceneUnitScale;
	MetaModel &md = currScene->getMetaModel(metaModelId);

	int parentNodeId = currScene->m_ssg->findParentNodeIdForModel(metaModelId);
	int parentMetaModelId = currScene->m_ssg->m_graphNodeToModelListIdMap[parentNodeId];

	QString sampleType;

	if (parentNodeId != -1)
	{
		MetaModel &parentMd = currScene->getMetaModel(parentMetaModelId);
		SuppPlane &parentSuppPlane = parentMd.suppPlane;
		if (parentSuppPlane.m_isInited)
		{
			sampleType = " on parent-" + currScene->m_ssg->m_nodes[parentNodeId].nodeName;
			newPos = parentSuppPlane.randomSamplePoint(0.2);
		}
	}
	else
	{
		sampleType = "on floor";

		std::vector<double> shiftVals;
		std::vector<double> dMeans(2, 0); // set mean to be (0,0)
		std::vector<double> stdDevs(2, 0.2);

		GenNNormalDistribution(dMeans, stdDevs, shiftVals);
		vec3 translateVec = vec3(shiftVals[0] / m_sceneMetric, shiftVals[1] / m_sceneMetric, 0);
		newPos = md.position + translateVec;
	}
}

void RelationModelManager::collectConstraintsForModel(TSScene *currScene, int metaModelId)
{

	SceneSemGraph *currSSG = currScene->m_ssg;

	int currNodeId = currSSG->getNodeIdWithModelId(metaModelId);

	SemNode &currNode = currSSG->m_nodes[currNodeId];

	if (!currNode.outEdgeNodeList.empty())
	{
		int relationNodeId = currNode.outEdgeNodeList[0];
		SemNode &relationNode = currSSG->m_nodes[relationNodeId];

		if (!relationNode.isAligned)
		{
			int anchorObjId = relationNode.anchorNodeList[0];
			SemNode &anchorObjNode = currSSG->m_nodes[anchorObjId];

			QString relationName = relationNode.nodeName;
			QString anchorObjName = anchorObjNode.nodeName;
			QString actObjName = currNode.nodeName;

			// find explicit constraints specified in the SSG
			PairwiseRelationModel *pairwiseModel = retrievePairwiseModel(anchorObjName, actObjName, relationName);
			if (pairwiseModel!=NULL)
			{
				currScene->m_explictConstraints[metaModelId].push_back(RelationConstraint(pairwiseModel, "pairwise", anchorObjId));
			}

			QString conditionName;

			// find condition type
			if (relationName.contains("support") || relationName.contains("on") || relationName.contains("with"))
			{
				// find sibling objs supported by the same parent
				for (int i = 0; i < anchorObjNode.inEdgeNodeList.size(); i++)
				{
					int anchorRelNodeId = anchorObjNode.inEdgeNodeList[i];
					if (anchorRelNodeId == relationNodeId) continue;					
					SemNode &anchorRelNode = currSSG->m_nodes[anchorRelNodeId];
					if (anchorRelNode.nodeName == relationName)
					{
						int sibActNodeId = anchorRelNode.activeNodeList[0];
						SemNode &sibActNode = currSSG->m_nodes[sibActNodeId];

						// use sibling obj as anchor in relative constraints if the sibling has been placed
						int sibModelId = currSSG->m_graphNodeToModelListIdMap[sibActNodeId];
						MetaModel& sibModel = currScene->getMetaModel(sibModelId);

						if (!sibModel.isAlreadyPlaced) continue;
					
						QString imRelationKey = sibActNode.nodeName + "_" + actObjName + "_" + "sibling" + "_general";
						if (m_relativeModels.count(imRelationKey))
						{
							currScene->m_implicitConstraints[metaModelId].push_back(RelationConstraint(m_relativeModels[imRelationKey], "relative", sibModelId));
						}
					}
				}
			}
			//else if (currSSG->findParentNodeIdForModel(currNodeId) ==
			//	currSSG->findParentNodeIdForNode(anchorObjId))
			//{
			//	conditionName = ConditionName[1];

			//	// find other sibling objs 


			//}
			//else if (relationName.contains("near") || relationName.contains("next") ||
			//	relationName.contains("close"))
			//{
			//	conditionName = ConditionName[2];
			//}
		}
	}	
}

PairwiseRelationModel* RelationModelManager::retrievePairwiseModel(const QString &anchorObjName, const QString &actObjName, const QString &relationName)
{
	for (auto iter = m_pairwiseRelModels.begin(); iter != m_pairwiseRelModels.end(); iter++)
	{
		QString relationKey = iter->first;

		if (relationKey.contains(anchorObjName + "_" + actObjName))
		{
			if (relationKey.contains(relationName))
			{
				return m_pairwiseRelModels[relationKey];
			}
		}
	}

	return NULL;
}

