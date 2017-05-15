#include "RelationModelManager.h"
#include "TSScene.h"
#include "CollisionManager.h"
#include "Model.h"
#include "SceneSemGraph.h"
#include <Eigen/Dense>

const double ExWeight = 0.7;

RelationModelManager::RelationModelManager()
{
	loadRelationModels();

	m_closeSampleTh = 0.03;
	m_sceneMetric = params::inst()->globalSceneUnitScale;

	m_adjustFrontObjs.push_back("desk");
	m_adjustFrontObjs.push_back("bookcase");
	m_adjustFrontObjs.push_back("cabinet");
	m_adjustFrontObjs.push_back("dresser");
	m_adjustFrontObjs.push_back("monitor");
	m_adjustFrontObjs.push_back("tv");
	m_adjustFrontObjs.push_back("bed");
}

RelationModelManager::~RelationModelManager()
{

}

void RelationModelManager::loadRelationModels()
{
	loadRelativeRelationModels();
	loadPairwiseRelationModels();
	loadGroupRelationModels();
	loadSupportRelationModels();
}

void RelationModelManager::loadRelativeRelationModels()
{
	QString sceneDBPath = "./SceneDB";
	QString filename = sceneDBPath + QString("/Relative_%1.model").arg(params::inst()->sceneDBType);

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
	QString filename = QString(sceneDBPath + "/Pairwise_%1.model").arg(params::inst()->sceneDBType);

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

	loadPairModelSim();
}

void RelationModelManager::loadGroupRelationModels()
{
	QString sceneDBPath = "./SceneDB";
	QString filename = sceneDBPath + QString("/Group_%1.model").arg(params::inst()->sceneDBType);

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
				QString objName = toQString(subParts[0]);

				if(isToSkipModelCats(objName)) continue;

				OccurrenceModel *newOccurModel = new OccurrenceModel(objName, StringToInt(subParts[1]));
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

		newGroupModel->normalizeOccurrenceProbs(0.2, 0.8);
	}

	inFile.close();

	loadGroupPariModelSim();
}

void RelationModelManager::loadPairModelSim()
{
	QString sceneDBPath = "./SceneDB";
	QString filename = sceneDBPath + QString("/Pairwise_%1.sim").arg(params::inst()->sceneDBType);

	QFile inFile(filename);
	QTextStream ifs(&inFile);

	if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;
	
	m_pairwiseModelKeys.resize(m_pairwiseRelModels.size());
	for (int i=0; i < m_pairwiseRelModels.size(); i++)
	{
		QString currLine = ifs.readLine();
		std::vector<std::string> parts = PartitionString(currLine.toStdString(), ",");
		std::vector<std::string> subparts = PartitionString(parts[1], "_");
		PairwiseRelationModel *relModel = m_pairwiseRelModels[toQString(parts[0])];
		relModel->m_modelId = StringToInt(subparts[1]);
		m_pairwiseModelKeys[i] = relModel->m_relationKey;

		currLine = ifs.readLine();
		parts = PartitionString(currLine.toStdString(), ",");
		int simModelNum = StringToInt(parts[0]);
		std::vector<int> intList = StringToIntegerList(parts[1], "");
		relModel->m_simModelIds.resize(simModelNum);

		for (int k=0; k < simModelNum; k++)
		{
			relModel->m_simModelIds[k] = intList[k];
		}

		relModel->m_maxObjFeatures.resize(2);
		relModel->m_avgObjFeatures.resize(2);

		for (int m=0; m < 2; m++)
		{
			currLine = ifs.readLine();
			relModel->m_maxObjFeatures[m] = StringToFloatList(currLine.toStdString(), "");
		}

		for (int m = 0; m < 2; m++)
		{
			currLine = ifs.readLine();
			relModel->m_avgObjFeatures[m] = StringToFloatList(currLine.toStdString(), "");
		}	
	}

	inFile.close();
}

void RelationModelManager::loadGroupPariModelSim()
{
	QString sceneDBPath = "./SceneDB";
	QString filename = sceneDBPath + QString("/Group_%1.sim").arg(params::inst()->sceneDBType);

	QFile inFile(filename);
	QTextStream ifs(&inFile);

	if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;

	for (int i=0; i < m_groupRelModels.size(); i++)
	{
		QString currLine = ifs.readLine();
		GroupRelationModel *groupModel = m_groupRelModels[currLine];

		groupModel->m_pairwiseModelKeys.resize(groupModel->m_pairwiseModels.size());
		for (int p=0; p <groupModel->m_pairwiseModels.size(); p++)
		{
			currLine = ifs.readLine();
			std::vector<std::string> parts = PartitionString(currLine.toStdString(), ",");
			std::vector<std::string> subparts = PartitionString(parts[1], "_");
			PairwiseRelationModel *relModel = groupModel->m_pairwiseModels[toQString(parts[0])];
			relModel->m_modelId = StringToInt(subparts[1]);
			groupModel->m_pairwiseModelKeys[p] = relModel->m_relationKey;

			currLine = ifs.readLine();
			parts = PartitionString(currLine.toStdString(), ",");
			int simModelNum = StringToInt(parts[0]);
			std::vector<int> intList = StringToIntegerList(parts[1], "");
			relModel->m_simModelIds.resize(simModelNum);

			for (int k = 0; k < simModelNum; k++)
			{
				relModel->m_simModelIds[k] = intList[k];
			}
		}
	}

	inFile.close();
}

void RelationModelManager::loadSupportRelationModels()
{
	QString sceneDBPath = "./SceneDB";
	QString filename = sceneDBPath + QString("/SupportRelation_%1.model").arg(params::inst()->sceneDBType);

	QFile inFile(filename);
	QTextStream ifs(&inFile);

	if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;

	while (!ifs.atEnd())
	{
		SupportRelation *newSuppRelation = new SupportRelation();
		newSuppRelation->loadFromStream(ifs);

		m_supportRelations[newSuppRelation->m_suppRelKey] = newSuppRelation;
	}
	inFile.close();

	filename = sceneDBPath + QString("/SupportParent_%1.prob").arg(params::inst()->sceneDBType);
	inFile.setFileName(filename);
	if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;

	while (!ifs.atEnd())
	{
		QString  currLine = ifs.readLine();
		std::vector<std::string> parts = PartitionString(currLine.toStdString(), ",");
		std::vector<float> floatList = StringToFloatList(parts[1], "");

		SupportProb newSuppProb((double)floatList[0], (double)floatList[1]);
		m_suppProbs[toQString(parts[0])] = newSuppProb;
	}

	inFile.close();
}

bool RelationModelManager::isRelationsViolated(TSScene *currScene, int metaModelId)
{
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

	for (int i = 0; i < imConstraints.size(); i++)
	{
		if (isConstraintViolated(currScene, md, imConstraints[i]))
		{
			return true;
		}
	}

	return false;
}

bool RelationModelManager::isConstraintViolated(TSScene *currScene, const MetaModel &md, const RelationConstraint &relConstraint)
{
	PairwiseRelationModel *pairModel = relConstraint.relModel;
	if (pairModel->m_numGauss == 0) 
		return false;

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
		delete newRelPos;
		return true;
	}

	delete newRelPos;
	return false;
}

double RelationModelManager::computeLayoutPassScore(TSScene *currScene, int metaModelId)
{
	MetaModel &md = currScene->getMetaModel(metaModelId);

	std::vector<RelationConstraint> &exConstraints = currScene->m_explictConstraints[metaModelId];
	std::vector<RelationConstraint> &imConstraints = currScene->m_implicitConstraints[metaModelId];

	double score = 0;

	for (int i = 0; i < exConstraints.size(); i++)
	{
		RelationConstraint &relConstraint = exConstraints[i];
		if (relConstraint.relModel->m_GMM != NULL)
		{
			score += ExWeight*relConstraint.relModel->m_GMM->m_probTh[0]; // use 20% percentile
		}
	}

	for (int i = 0; i < imConstraints.size(); i++)
	{
		RelationConstraint &relConstraint = imConstraints[i];
		if (relConstraint.relModel->m_GMM != NULL)
		{
			score += (1 - ExWeight)*relConstraint.relModel->m_GMM->m_probTh[0]; // use 20% percentile
		}
	}

	return score;
}
double RelationModelManager::computeRelationScore(TSScene *currScene, int metaModelId, const Eigen::VectorXd &currPlacement)
{
	MetaModel &md = currScene->getMetaModel(metaModelId);

	std::vector<RelationConstraint> &exConstraints = currScene->m_explictConstraints[metaModelId];
	std::vector<RelationConstraint> &imConstraints = currScene->m_implicitConstraints[metaModelId];

	double score = 0;

	for (int i = 0; i < exConstraints.size(); i++)
	{
		RelationConstraint &relConstraint = exConstraints[i];
		score += ExWeight*computeScoreForConstraint(currScene, relConstraint, currPlacement);
	}

	for (int i = 0; i < imConstraints.size(); i++)
	{
		RelationConstraint &relConstraint = imConstraints[i];
		score += (1-ExWeight)*computeScoreForConstraint(currScene, relConstraint, currPlacement);
	}

	return score;
}

double RelationModelManager::computeRelationScoreForGroup(TSScene *currScene, std::vector<int> metaModelIds, const std::vector<Eigen::VectorXd> &currPlacements)
{
	double groupScore = 0;

	for (int i=0; i < metaModelIds.size(); i++)
	{
		int metaModelId = metaModelIds[i];
		MetaModel &md = currScene->getMetaModel(metaModelId);
		md.layoutScore = computeRelationScore(currScene, metaModelIds[i], currPlacements[i]);
		groupScore += md.layoutScore;
	}

	return groupScore;
}

double RelationModelManager::computeScoreForConstraint(TSScene *currScene, const RelationConstraint &relConstraint, const Eigen::VectorXd &currPlacement)
{
	PairwiseRelationModel *pairModel = relConstraint.relModel;

	if (pairModel == NULL || pairModel->m_numGauss == 0)
	{
		return 0;
	}

	int anchorObjId = relConstraint.anchorObjId;
	MetaModel &anchorMd = currScene->getMetaModel(anchorObjId);

	RelativePos *newRelPos = new RelativePos();

	extractRelPosToAnchor(currScene, anchorMd, currPlacement, newRelPos);

	Eigen::VectorXd observation(4);
	observation[0] = newRelPos->pos.x;
	observation[1] = newRelPos->pos.y;
	observation[2] = newRelPos->pos.z;
	observation[3] = newRelPos->theta;

	double prob = pairModel->m_GMM->probability(observation);
	
	delete newRelPos;
	return prob;
}

void RelationModelManager::extractRelPosToAnchor(TSScene *currScene, const MetaModel &anchorMd, const Eigen::VectorXd &currPlacement, RelativePos *relPos)
{
	// get unitize transform for anchor model
	Model *loadedAnchorModel = currScene->getModel(anchorMd.name);
	mat4 alignMat = getModelToUnitboxMat(loadedAnchorModel, anchorMd);

	vec3 actPos = vec3(currPlacement[0], currPlacement[1], currPlacement[2]);

	relPos->pos = TransformPoint(alignMat, actPos);
	relPos->theta = currPlacement[3]/math_pi;  // theta in relational model is normalized by pi
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
	relPos->theta = GetRotAngleR(anchorFront, actFront, vec3(0, 0, 1))/math_pi;    // theta in relational model is normalized by pi
}

mat4 RelationModelManager::getModelToUnitboxMat(Model *m, const MetaModel &md)
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

	return alignMat;
}

void RelationModelManager::updateCollisionPostions(const std::vector<std::vector<vec3>> &collisionPositions)
{
	m_collisionPositions.clear();
	m_collisionPositions = collisionPositions;
}

bool RelationModelManager::isPosValid(TSScene *currScene, const vec3 &pos, int metaModelId)
{
	if (isPosCloseToInvalidPos(pos, metaModelId))
	{
		return false;
	}

	return true;
}

bool RelationModelManager::isPosCloseToInvalidPos(const vec3 &pos, int metaModelId)
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

PairwiseRelationModel* RelationModelManager::getPairModelById(int id)
{
	if (m_pairwiseRelModels.count(m_pairwiseModelKeys[id]))
	{
		return m_pairwiseRelModels[m_pairwiseModelKeys[id]];
	}
	else
	{
		return NULL;
	}
}

PairwiseRelationModel* RelationModelManager::getPairModelInGroupById(GroupRelationModel *groupModel, int id)
{
	if (groupModel->m_pairwiseModels.count(groupModel->m_pairwiseModelKeys[id]))
	{
		return groupModel->m_pairwiseModels[groupModel->m_pairwiseModelKeys[id]];
	}
	else
	{
		return NULL;
	}
}

bool RelationModelManager::isToSkipModelCats(const QString &objName)
{
	if (objName == "rug")
	{
		return true;
	}

	return false;
}

Eigen::VectorXd RelationModelManager::sampleNewPosFromConstraints(TSScene *currScene, int metaModelId, int &anchorModelId)
{
	MetaModel &md = currScene->getMetaModel(metaModelId);
	vec3 newWorldPos= vec3();
	double newWorldTheta = 0;

	QString constraintType;

	int count = 0; 

	while (count < 1000)
	{
		PairwiseRelationModel *currPairModel = NULL;
		bool isPairModelAvailable = false;
		if (!currScene->m_explictConstraints[metaModelId].empty())
		{
			currPairModel = findAvailablePairModels(currScene, currScene->m_explictConstraints[metaModelId][0], isPairModelAvailable);
		}
			
		if (!isPairModelAvailable)
		{
				randomSampleOnParent(currScene, metaModelId, newWorldPos, anchorModelId);
				newWorldTheta = 0;
				constraintType = "random";
		}
		else
		{
			RelationConstraint &exConstraint = currScene->m_explictConstraints[metaModelId][0];
			anchorModelId = exConstraint.anchorObjId;

			sampleFromRelationModel(currScene, currPairModel, metaModelId, anchorModelId, newWorldPos, newWorldTheta);
			constraintType = "explicit";

			//if ((newWorldPos - md.position).length() < 1.0 / m_sceneMetric)
			//{
			//	continue;
			//}
		}

		if (isPosValid(currScene, newWorldPos, metaModelId))
		{
			break;
		}
		else
		{
			if (constraintType == "explicit" &&currPairModel->m_lastSampleInstanceId != -1)
			{
				currPairModel->updateCandiInstanceIds();
			}
		}

		count++;
	}

	// convert to world frame
	Eigen::VectorXd  worldPos(4);
	worldPos[0] = newWorldPos.x;
	worldPos[1] = newWorldPos.y;
	worldPos[2] = newWorldPos.z;  
	worldPos[3] = newWorldTheta;   // theta is not affected by transformation	

	if (isnan(newWorldPos.x))
	{
		qDebug();
	}

	return worldPos;
}

PairwiseRelationModel*  RelationModelManager::findAvailablePairModels(TSScene *currScene, const RelationConstraint &relConstraint, bool &isPairModelAvailable)
{
	if (relConstraint.relModel->hasCandiInstances())
	{
		isPairModelAvailable = true;
		return relConstraint.relModel;
	}
	else
	{
		std::vector<int> simModelIds = relConstraint.relModel->m_simModelIds;

		for (int i=0; i<simModelIds.size(); i++)
		{
			int pariModelId = simModelIds[i];
			PairwiseRelationModel *currRelModel = NULL;
			
			if (relConstraint.relationType=="pairwise")
			{
				QString pairModelKey = m_pairwiseModelKeys[pariModelId];
				currRelModel = m_pairwiseRelModels[pairModelKey];
			}
			else if(relConstraint.relationType == "group")
			{
				QString groupKey = relConstraint.groupKey;
				if (m_groupRelModels.count(groupKey))
				{
					QString pairModelKey = m_groupRelModels[groupKey]->m_pairwiseModelKeys[pariModelId];
					currRelModel = m_groupRelModels[groupKey]->m_pairwiseModels[pairModelKey];
				}			
			}

			if (currRelModel!= NULL && currRelModel->hasCandiInstances())
			{
				isPairModelAvailable = true;
				return currRelModel;
			}
		}

		isPairModelAvailable = false;
		return NULL;
	}
}

void RelationModelManager::sampleFromRelationModel(TSScene *currScene, PairwiseRelationModel *pairModel, const int metaModelId, const int anchorModelId, vec3 &newWorldPos, double &newWolrdTheta)
{
	// sample in the unit frame
	Eigen::VectorXd newSample = pairModel->sample();
	vec3 relPos = vec3(newSample[0], newSample[1], newSample[2]);
	double relTheta = newSample[3];

	// anchor obj																					 
	MetaModel &anchorMd = currScene->getMetaModel(anchorModelId);
	Model *anchorModel = currScene->getModel(anchorMd.name);

	if (!isAnchorFrontDirConsistent(toQString(anchorMd.catName), pairModel->m_anchorObjName))
	{
		relPos.x *= -1;
	}

	MetaModel &actMd = currScene->getMetaModel(metaModelId);
	adjustSamplingForSpecialModels(toQString(anchorMd.catName), toQString(actMd.catName), relPos, relTheta);

	mat4 alignMat = getModelToUnitboxMat(anchorModel, anchorMd);
	mat4 transMat = alignMat.inverse();  // transform from unit frame to world frame

	newWorldPos = TransformPoint(transMat, relPos);

	// update sampled position 
	double newZ = findClosestSuppPlaneZ(currScene, metaModelId, newWorldPos);
	newWorldPos.z = newZ;

	newWolrdTheta = relTheta* math_pi;  // theta in relational model is normalized by pi

	if (isnan(newWorldPos.x))
	{
		qDebug();
	}
}

void RelationModelManager::adjustSamplingForSpecialModels(const QString &anchorObjName, const QString &actObjName, vec3 &relPos, double &relTheta)
{
	if (anchorObjName == "bookcase" && actObjName == "standbooks")
	{
		relTheta = 0;
	}

	if (anchorObjName == "couch" && actObjName == "table")
	{
		relTheta = 1;
	}

	if (anchorObjName == "bed" && actObjName == "nightstand")
	{
		if (relPos.y >-0.2 || relPos.y < -0.4)
		{
			relPos.y = -0.35;
		}

		relTheta = 0;
	}

	if (anchorObjName == "desk" && actObjName == "couch")
	{
		if (relPos.y < 3)
		{
			relPos.y = 4;
		}
	}

	if (anchorObjName == "desk" && actObjName == "bookcase")
	{
		if (relPos.x > -0.8)
		{
			relPos.x = -1.5;
		}
	}


	if (anchorObjName == "bed" && actObjName == "desk")
	{
		if (relPos.y > -0.2)
		{
			relPos.y = -0.35;
		}

		if (relPos.x > 0 && relPos.x < 0.6)
		{
			relPos.x = 1.1;
		}

		if (relPos.x < 0 && relPos.x > -0.6)
		{
			relPos.x = -1.1;
		}

		relTheta = 0;
	}

	if (anchorObjName == "bed" && actObjName == "dresser")
	{
		if (relPos.y > -0.2)
		{
			relPos.y = -0.3;
		}
	}

	if (anchorObjName == "couch" && actObjName == "chair")
	{
		// right
		if (relPos.x >0 && (relTheta < 0 || relTheta > 0.5))
		{
			relTheta = 0.3;
		}

		// left
		if (relPos.x < 0 && (relTheta < -0.5 || relTheta >0))
		{
			relTheta = -0.3;
		}

		if (relPos.y < -0.2 || relPos.y > 1)
		{
			relPos.y = 0.8;
		}

		// do not occlude the couch
		if (relPos.x > 0 && relPos.x< 0.5)
		{
			relPos.x = 1;				 
		}

		if (relPos.x < 0 && relPos.x > -0.5)
		{
			relPos.x = -1;
		}
	}

	if (anchorObjName == "table" && actObjName == "chair")
	{
		// right
		if (relPos.x > 0 && (relTheta < 0.25 || relTheta >0.75))
		{
			relTheta = 0.5;
		}

		// left
		if (relPos.x < 0 && (relTheta > -0.25 || relTheta < -0.75))
		{
			relTheta = -0.5;
		}
	}
}

double RelationModelManager::findClosestSuppPlaneZ(TSScene *currScene, int metaModelId, const vec3 &newPos)
{
	MetaModel &md = currScene->getMetaModel(metaModelId);
	int parentNodeId = currScene->m_ssg->findParentNodeIdForModel(metaModelId);
	double sceneMetric = params::inst()->globalSceneUnitScale;

	// use z of parent's support plane if parent exists
	if (parentNodeId != -1)
	{
		int parentMetaModelId = currScene->m_ssg->m_graphNodeToModelListIdMap[parentNodeId];
		if (parentMetaModelId != -1)
		{
			MetaModel &parentMd = currScene->getMetaModel(parentMetaModelId);
			double closestSuppPlaneZ = parentMd.bbTopPlane.getZ();
			double minDist = 1e6;

			if (parentMd.suppPlaneManager.hasSuppPlane())
			{
				for (int i = 0; i < parentMd.suppPlaneManager.m_suppPlanes.size(); i++)
				{
					SuppPlane &parentSuppPlane = parentMd.suppPlaneManager.m_suppPlanes[i];

					double planeZ = parentSuppPlane.getZ();
					double d = std::abs(newPos.z - planeZ);
					if (d < minDist)
					{
						minDist = d;
						closestSuppPlaneZ = planeZ;
					}
				}
			}

			// adjust Z val since suppPlane Z might be not accurate
			double newZ;
			if (currScene->computeZForModel(metaModelId, parentMetaModelId, vec3(newPos.x, newPos.y, closestSuppPlaneZ), newZ))
			{
				if (newZ > md.position.z || md.position.z - newZ > 0.05 / sceneMetric)
				{
					closestSuppPlaneZ = newZ;
				}
				else
				{
					closestSuppPlaneZ = md.position.z;
				}
			}

			return closestSuppPlaneZ;
		}
	}

	// use z of anchor's position if parent does not exist
	int currNodeId = currScene->m_ssg->getNodeIdWithModelId(metaModelId);
	SemNode &currNode = currScene->m_ssg->m_nodes[currNodeId];
	for (int i=0; i<currNode.outEdgeNodeList.size(); i++)
	{
		int relNodeId = currNode.outEdgeNodeList[i];
		SemNode &relNode = currScene->m_ssg->m_nodes[relNodeId];

		if (relNode.anchorNodeList.size()>0)
		{
			int anchorNodeId = relNode.anchorNodeList[0];
			int anchorModeId = currScene->m_ssg->m_graphNodeToModelListIdMap[anchorNodeId];
			MetaModel &anchorModel = currScene->getMetaModel(anchorModeId);
			
			if (anchorModel.position.z - currScene->m_floorHeight < 0.25/ sceneMetric)
			{
				return anchorModel.position.z;
			}
		}
	}

	return currScene->m_floorHeight;
}

void RelationModelManager::randomSampleOnParent(TSScene *currScene, int metaModelId, vec3 &newPos, int &parentMetaModelId)
{
	double m_sceneMetric = params::inst()->globalSceneUnitScale;
	MetaModel &md = currScene->getMetaModel(metaModelId);

	int parentNodeId = currScene->m_ssg->findParentNodeIdForModel(metaModelId);

	QString sampleType;

	if (parentNodeId != -1)
	{
		parentMetaModelId = currScene->m_ssg->m_graphNodeToModelListIdMap[parentNodeId];
		MetaModel &parentMd = currScene->getMetaModel(parentMetaModelId);
		sampleType = " on parent-" + currScene->m_ssg->m_nodes[parentNodeId].nodeName;

		if (parentMd.suppPlaneManager.hasSuppPlane())
		{
			SuppPlane &parentSuppPlane = parentMd.suppPlaneManager.getLargestAreaSuppPlane();
			newPos = parentSuppPlane.randomSamplePoint(0.1);
		}
		else
		{
			SuppPlane &parentSuppPlane = parentMd.bbTopPlane;
			newPos = parentSuppPlane.randomSamplePoint(0.1);
		}

		double newZ = findClosestSuppPlaneZ(currScene, metaModelId, newPos);
		newPos.z = newZ;
	}
	else
	{
		parentMetaModelId = -1;
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

	if (currNode.nodeType == "object" &&!currNode.outEdgeNodeList.empty())
	{
		for (int r = 0; r < currNode.outEdgeNodeList.size(); r++)
		{
			int relationNodeId = currNode.outEdgeNodeList[r];
			SemNode &relationNode = currSSG->m_nodes[relationNodeId];

			//if (!relationNode.isAligned)
			if (relationNode.matchingStatus == SemNode::ExplicitNode)
			{
				if (relationNode.anchorNodeList.empty()) return;

				int anchorObjNodeId = relationNode.anchorNodeList[0];
				SemNode &anchorObjNode = currSSG->m_nodes[anchorObjNodeId];

				QString relationName = relationNode.nodeName;
				MetaModel &anchorMd = currScene->getMetaModel(currSSG->m_graphNodeToModelListIdMap[anchorObjNodeId]);
				MetaModel &actMd = currScene->getMetaModel(currSSG->m_graphNodeToModelListIdMap[currNodeId]);

				Model *refModel = currScene->getModel(anchorMd.name);
				Model *actModel = currScene->getModel(actMd.name);
				if (!refModel->m_loadingDone || !actModel->m_loadingDone) return;

				// find explicit constraints specified in the SSG
				PairwiseRelationModel *pairwiseModel;
				QString relationType;
				QString groupKey;
				if (relationNode.nodeType.contains("group"))
				{
					pairwiseModel = retrievePairModelFromGroup(currScene, anchorObjNodeId, currNodeId, relationNode.nodeName);
					relationType = "group";
					groupKey = relationNode.nodeName + "_" + toQString(anchorMd.catName);
				}
				else
				{
					pairwiseModel = retrievePairwiseModel(currScene, anchorObjNodeId, currNodeId, relationName);
					relationType = "pairwise";
				}

				if (pairwiseModel != NULL)
				{
					int anchorModelId = currSSG->m_graphNodeToModelListIdMap[anchorObjNodeId];
					currScene->m_explictConstraints[metaModelId].push_back(RelationConstraint(pairwiseModel, relationType, anchorModelId, groupKey));

					MetaModel &md = currScene->getMetaModel(metaModelId);
					md.explicitAnchorId = anchorModelId;
				}

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

							QString imRelationKey = sibActNode.nodeName + "_" + currNode.nodeName + "_" + "sibling" + "_general";
							if (m_relativeModels.count(imRelationKey))
							{
								currScene->m_implicitConstraints[metaModelId].push_back(RelationConstraint(m_relativeModels[imRelationKey], "relative", sibModelId));
							}
						}
					}
				}

				actMd.isConstraintExtracted = true;
			}
		}
	}
	//// add near if there is no relation specified
	//else if(currNode.nodeType == "object" && currNode.outEdgeNodeList.empty() && currNode.matchingStatus == SemNode::ExplicitNode)
	//{
	//	if (currScene->m_explictConstraints[metaModelId].empty())
	//	{
	//		// find another object which has diff cat and use it as anchor
	//		int anchorObjNodeId = -1;
	//		for (int j = 0; j < currSSG->m_nodes.size(); j++)
	//		{
	//			SemNode &anchorNode = currSSG->m_nodes[j];
	//			if (anchorNode.nodeType == "object" && anchorNode.matchingStatus == SemNode::ExplicitNode
	//				&& anchorNode.nodeName != currNode.nodeName)
	//			{
	//				anchorObjNodeId = j;
	//				break;
	//			}
	//		}

	//		if (anchorObjNodeId != -1)
	//		{
	//			SemNode &anchorObjNode = currSSG->m_nodes[anchorObjNodeId];

	//			QString relationName = "near";
	//			MetaModel &anchorMd = currScene->getMetaModel(currSSG->m_graphNodeToModelListIdMap[anchorObjNodeId]);
	//			MetaModel &actMd = currScene->getMetaModel(currSSG->m_graphNodeToModelListIdMap[currNodeId]);

	//			Model *refModel = currScene->getModel(anchorMd.name);
	//			Model *actModel = currScene->getModel(actMd.name);
	//			if (!refModel->m_loadingDone || !actModel->m_loadingDone) return;

	//			// find explicit constraints specified in the SSG
	//			PairwiseRelationModel *pairwiseModel;
	//			QString relationType;

	//			pairwiseModel = retrievePairwiseModel(currScene, anchorMd, actMd, relationName);
	//			relationType = "pairwise";

	//			if (pairwiseModel != NULL)
	//			{
	//				int anchorModelId = currSSG->m_graphNodeToModelListIdMap[anchorObjNodeId];
	//				currScene->m_explictConstraints[metaModelId].push_back(RelationConstraint(pairwiseModel, relationType, anchorModelId, ""));

	//				MetaModel &md = currScene->getMetaModel(metaModelId);
	//				md.explicitAnchorId = anchorModelId;
	//			}
	//		}
	//	}
	//}
}

PairwiseRelationModel* RelationModelManager::retrievePairwiseModel(TSScene *currScene, int anchorNodeId, int actNodeId, const QString &relationName)
{
	SceneSemGraph *currSSG = currScene->m_ssg;
	MetaModel &anchorMd = currScene->getMetaModel(currSSG->m_graphNodeToModelListIdMap[anchorNodeId]);
	MetaModel &actMd = currScene->getMetaModel(currSSG->m_graphNodeToModelListIdMap[actNodeId]);

	QString anchorObjName = toQString(anchorMd.catName);
	QString actObjName = toQString(actMd.catName);

	//QString conditionType;
	QString conditionType = getConditionType(currScene, anchorNodeId, actNodeId);

	for (auto iter = m_pairwiseRelModels.begin(); iter != m_pairwiseRelModels.end(); iter++)
	{
		QString relationKey = iter->first;

		if (relationKey.contains(anchorObjName + "_" + actObjName))
		{
			if (!conditionType.isEmpty())
			{
				if (relationKey.contains(relationName) && relationKey.contains(conditionType))
				{
					PairwiseRelationModel *pairModel = m_pairwiseRelModels[relationKey];
					if (pairModel->hasCandiInstances())
					{
						return pairModel;
					}
				}
			}
			else
			{
				if (relationKey.contains(relationName))
				{
					PairwiseRelationModel *pairModel = m_pairwiseRelModels[relationKey];
					if (pairModel->hasCandiInstances())
					{
						return pairModel;
					}
				}
			}
		}
	}

	// retrieve similar models
	Model *anchorModel = currScene->getModel(anchorMd.name);
	Model *actModel = currScene->getModel(actMd.name);

	std::vector<std::vector<double>> currBBFeatures(2);
	currBBFeatures[0]= anchorModel->computeBBFeature(anchorMd.transformation);
	currBBFeatures[1] = actModel->computeBBFeature(actMd.transformation);

	int featureDim = currBBFeatures[0].size();

	std::vector<std::pair<double, PairwiseRelationModel*>> simPairModels;
	for (auto iter = m_pairwiseRelModels.begin(); iter != m_pairwiseRelModels.end(); iter++)
	{
		QString relationKey = iter->first;
		if (relationKey.contains(relationName) && relationKey.contains(conditionType))
		{
			PairwiseRelationModel *dbPairModel = iter->second;

			double score = 0;

			// check whether the anchor's front is consistent
			//if (isAnchorFrontDirConsistent(anchorObjName, dbPairModel->m_anchorObjName))
			{
				std::vector<double> simVal(2, 0);
				std::vector<double> geoSim(2, 0);

				std::vector<double> catSim(2, 0);
				if (dbPairModel->m_anchorObjName == anchorObjName) catSim[0] = 1;
				if (dbPairModel->m_actObjName == actObjName) catSim[1] = 1;


				double catWeight = 0.3;

				for (int m = 0; m < 2; m++)
				{
					for (int d = 0; d < featureDim; d++)
					{
						geoSim[m] += exp(-pow((dbPairModel->m_avgObjFeatures[m][d] - currBBFeatures[m][d]) / (dbPairModel->m_maxObjFeatures[m][d] + 1e-3), 2));
					}
					geoSim[m] /= featureDim;
					simVal[m] = catWeight*catSim[m] + (1 - catWeight)*geoSim[m];
				}

				score = simVal[0] * simVal[1];
			}
			//else
			//{
			//	score = 0;
			//}

			simPairModels.push_back(std::make_pair(score, dbPairModel));
		}
	}

	std::sort(simPairModels.begin(), simPairModels.end());  // ascending order
	std::reverse(simPairModels.begin(), simPairModels.end()); // descending order

	if (!simPairModels.empty())
	{
		for (int i=0; i < simPairModels.size(); i++)
		{
			PairwiseRelationModel *pairModel = simPairModels[i].second;
			if (pairModel->hasCandiInstances())
			{
				return pairModel;
			}
		}
	}
	else
	{
		return NULL;
	}
}

bool RelationModelManager::isAnchorFrontDirConsistent(const QString &currAnchorName, const QString &dbAnchorName)
{
	if (std::find(m_adjustFrontObjs.begin(), m_adjustFrontObjs.end(), dbAnchorName) != m_adjustFrontObjs.end()
		&& std::find(m_adjustFrontObjs.begin(), m_adjustFrontObjs.end(), currAnchorName) == m_adjustFrontObjs.end())
	{
		return false;
	}

	if (std::find(m_adjustFrontObjs.begin(), m_adjustFrontObjs.end(), dbAnchorName) == m_adjustFrontObjs.end()
		&& std::find(m_adjustFrontObjs.begin(), m_adjustFrontObjs.end(), currAnchorName) != m_adjustFrontObjs.end())
	{
		return false;
	}

	return true;
}

QString RelationModelManager::getConditionType(TSScene *currScene, int anchorNodeId, int actNodeId)
{
	QString conditionType="";
	SceneSemGraph *currSSG = currScene->m_ssg;

	if (currSSG->m_graphNodeToModelListIdMap.count(anchorNodeId) && currSSG->m_graphNodeToModelListIdMap.count(actNodeId))
	{
		int anchorModelId = currSSG->m_graphNodeToModelListIdMap[anchorNodeId];
		int actModelId = currSSG->m_graphNodeToModelListIdMap[actNodeId];

		if (currSSG->m_parentOfModel[anchorModelId] == currSSG->m_parentOfModel[actModelId])
		{
			return "sibling";
		}

		if (std::find(currSSG->m_childListOfModel[anchorModelId].begin(), currSSG->m_childListOfModel[anchorModelId].end(), actModelId)
			!= currSSG->m_childListOfModel[anchorModelId].end())
		{
			return "parentchild";
		}
	}

	return conditionType;
}

PairwiseRelationModel* RelationModelManager::retrievePairModelFromGroup(TSScene *currScene, int anchorNodeId, int actNodeId, const QString &groupRelationName)
{
	SceneSemGraph *currSSG = currScene->m_ssg;
	MetaModel &anchorMd = currScene->getMetaModel(currSSG->m_graphNodeToModelListIdMap[anchorNodeId]);
	MetaModel &actMd = currScene->getMetaModel(currSSG->m_graphNodeToModelListIdMap[actNodeId]);

	QString anchorObjName = toQString(anchorMd.catName);
	QString actObjName = toQString(actMd.catName);

	QString conditionType = getConditionType(currScene, anchorNodeId, actNodeId);
	return retrievePairModelFromGroup(anchorObjName, actObjName, groupRelationName, conditionType);
}

PairwiseRelationModel* RelationModelManager::retrievePairModelFromGroup(const QString &anchorObjName, const QString &actObjName, const QString &groupRelationName, const QString &conditionName/*=""*/)
{
	QString groupKey = groupRelationName + "_" + anchorObjName;

	if (!m_groupRelModels.count(groupKey)) return NULL;

	GroupRelationModel *groupModel = m_groupRelModels[groupKey];

	for (auto iter = groupModel->m_pairwiseModels.begin(); iter != groupModel->m_pairwiseModels.end(); iter++)
	{
		QString relationKey = iter->first;

		if (relationKey.contains(anchorObjName + "_" + actObjName + "_" + conditionName))
		{
			return groupModel->m_pairwiseModels[relationKey];
		}
	}

	return NULL;
}

RelationConstraint::RelationConstraint(PairwiseRelationModel *m, const QString t, int anchorId, const QString &gK)
{
	relModel = m; relationType = t; anchorObjId = anchorId;
	groupKey = gK;
}
