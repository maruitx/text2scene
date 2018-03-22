#include "RelationModel.h"


PairwiseRelationModel::PairwiseRelationModel(const QString &anchorName, const QString &actName, const QString &conditionName, const QString & relationName /*= "general"*/)
	:m_anchorObjName(anchorName), m_actObjName(actName), m_conditionName(conditionName), m_relationName(relationName)

{
	m_relationKey = m_anchorObjName + "_" + m_actObjName + "_" + m_conditionName + "_" + m_relationName;
	m_GMM = NULL;

	m_lastSampleInstanceId = -1;
}


PairwiseRelationModel::~PairwiseRelationModel()
{
	if (m_GMM != NULL)
	{
		delete m_GMM;
	}
}

void PairwiseRelationModel::loadFromStream(QTextStream &ifs)
{
	QString currLine;

	// gaussNum instanceNum
	currLine = ifs.readLine();
	std::vector<float> floatList = StringToFloatList(currLine.toStdString(), "");
	m_numGauss = (int) floatList[0];
	m_numInstance = (int) floatList[1];

	Eigen::VectorXd probTh(floatList.size() - 2);
	for (int i=0; i < probTh.rows(); i++)
	{
		probTh[i] = floatList[i + 2];
	}

	std::vector<string> parts;

	if (m_numGauss == 0)
	{
		currLine = ifs.readLine();
		parts = PartitionString(currLine.toStdString(), ",");

		m_instances.resize(m_numInstance);
		m_candidateInstanceIds.resize(m_numInstance);

		for (int t = 0; t < m_numInstance; t++)
		{
			// observations
			std::vector<float> floatList = StringToFloatList(parts[t], "");
			RelativePos *newRelPos = new RelativePos();
			newRelPos->pos = vec3(floatList[0], floatList[1], floatList[2]);
			newRelPos->theta = floatList[3];
			newRelPos->m_anchorObjName = m_anchorObjName;
			newRelPos->m_actObjName = m_actObjName;
			newRelPos->m_conditionName = m_conditionName;

			m_instances[t] = newRelPos;
			m_candidateInstanceIds[t] = t;
		}
	}
	else
	{
		m_GMM = new GaussianMixtureModel(m_numGauss);
		m_GMM->m_probTh = probTh;

		for (int g = 0; g < m_numGauss; g++)
		{
			currLine = ifs.readLine();

			parts = PartitionString(currLine.toStdString(), ",");
			int gaussDim = StringToInt(parts[0]);
			double gaussWeight = StringToFloat(parts[1]);

			Eigen::VectorXd mean(gaussDim);
			Eigen::MatrixXd covarMat(gaussDim, gaussDim);

			std::vector<float> floatList = StringToFloatList(parts[2], "");

			for (int d = 0; d < gaussDim; d++)
			{
				mean[d] = floatList[d];   // have to set the size before you can use the comma initializer.
			}

			floatList = StringToFloatList(parts[3], "");
			for (int c = 0; c < gaussDim; c++)
			{
				for (int r = 0; r < gaussDim; r++)
				{
					covarMat(r, c) = floatList[c*gaussDim + r];
				}
			}
			
			GaussianModel* newGauss = new GaussianModel(gaussDim, gaussWeight, mean, covarMat);
			m_GMM->m_gaussians[g] = newGauss;
		}
	}
}

Eigen::VectorXd PairwiseRelationModel::sample()
{
	Eigen::VectorXd newSample(4);
	newSample[0] = 0;
	newSample[1] = 0;
	newSample[2] = 0;
	newSample[3] = 0.5;
	
	if (m_GMM != NULL)
	{
		if (m_conditionName.contains("parent"))
		{
			double minB = -0.5 + ParentBoundWidthRatio;
			double maxB = 0.5 - ParentBoundWidthRatio;

			// make sure no overhang when sampling
			int count = 0;
			while (count < 1000)
			{
				newSample = m_GMM->sample(m_GMM->m_probTh[0]);

				if (newSample[0] > minB && newSample[0] < maxB 
					&& newSample[1] >minB && newSample[1] < maxB)
					break;

				count++;
			}
		}
		else
		{
			newSample = m_GMM->sample(m_GMM->m_probTh[0]);
		}
	}
	else
	{
		int randId = GenRandomInt(0, m_candidateInstanceIds.size());
		int randInstId = m_candidateInstanceIds[randId];

		RelativePos *relPos = m_instances[randInstId];

		newSample[0] = relPos->pos.x;
		newSample[1] = relPos->pos.y;
		newSample[2] = relPos->pos.z;
		newSample[3] = relPos->theta;

		// filter observation that is too large
		if (std::abs(relPos->pos.x > 10)) newSample[0] = 0;
		if (std::abs(relPos->pos.y > 10)) newSample[1] = 0;
		if (std::abs(relPos->pos.z > 10)) newSample[2] = -0.5;

		if (m_relationName == "under")
		{
			if (relPos->pos.z > 0 || std::abs(relPos->pos.x) > 0.5 || std::abs(relPos->pos.y) > 0.5)
			{
				newSample[0] = 0;
				newSample[1] = 0;
				newSample[2] = -0.5;
			}
		}

		if (m_relationName.contains("side"))
		{
			if (std::abs(relPos->pos.x > 2) || std::abs(relPos->pos.y > 2))
			{
				newSample[0] = 0;
				newSample[1] = 0;
			}
		}

		if (m_relationName == "near")
		{
			if (relPos->pos.x > 2) newSample[0] = GenRandomDouble(0.8, 1);
			if (relPos->pos.y > 2) newSample[1] = GenRandomDouble(0.8, 1);

			if (relPos->pos.x <-2) newSample[0] = GenRandomDouble(-1, -0.8);
			if (relPos->pos.y <-2) newSample[1] = GenRandomDouble(-1, -0.8);
		}

		m_lastSampleInstanceId = randInstId;
	}

	if (m_relationName == "under" && newSample[2] > -0.5)
	{
		newSample[2] = -0.5;
	}

	if (m_relationName == "pairaligned")
	{
		if(newSample[0] > 2) newSample[0] = 1.5;
		if (newSample[0] < -2) newSample[0] = -1.5;		

		newSample[2] = -0.5;
	}

	return newSample;
}

bool PairwiseRelationModel::hasCandiInstances()
{
	if (m_numGauss > 0)
	{
		return true;
	}
	else if (m_candidateInstanceIds.size())
	{
		return true;
	}

	return false;
}

void PairwiseRelationModel::updateCandiInstanceIds()
{
	EraseValueInVectorInt(m_candidateInstanceIds, m_lastSampleInstanceId);
}

OccurrenceModel::OccurrenceModel(const QString &objName, int objNum)
	:m_objName(objName), m_objNum(objNum)
{
	m_occurKey = QString("%1_%2").arg(m_objName).arg(m_objNum);
	m_numInstance = 0;
	m_occurProb = 0;
}

GroupRelationModel::GroupRelationModel(const QString &anchorObjName, const QString &relationName)
	:m_anchorObjName(anchorObjName), m_relationName(relationName)
{
	m_groupKey = m_relationName + "_" + m_anchorObjName;
	m_numInstance = 0;
	m_maxOccProb = 0;
}

GroupRelationModel::~GroupRelationModel()
{

}

void GroupRelationModel::computeMaxOccProbs()
{
	m_maxOccProb = 0;

	for (auto iter = m_occurModels.begin(); iter != m_occurModels.end(); iter++)
	{
		OccurrenceModel *occModel = iter->second;
		if (occModel->m_occurProb > m_maxOccProb)
		{
			m_maxOccProb = occModel->m_occurProb;
		}
	}
}

void GroupRelationModel::normalizeOccurrenceProbs(double tarMax, double tarMin)
{
	double maxProb(0), minProb(1);

	for (auto iter = m_occurModels.begin(); iter!= m_occurModels.end(); iter++)
	{
		OccurrenceModel *occModel = iter->second;
		if (occModel->m_occurProb > maxProb)
		{
			maxProb = occModel->m_occurProb;
		}

		if (occModel->m_occurProb < minProb)
		{
			minProb = occModel->m_occurProb;
		}
	}

	double currRange = maxProb - minProb;
	double tarRange = tarMax - tarMin;

	for (auto iter = m_occurModels.begin(); iter != m_occurModels.end(); iter++)
	{
		OccurrenceModel *occModel = iter->second;
		occModel->m_occurProb = tarMin + (occModel->m_occurProb - minProb)*tarRange / currRange;
	}
}

SupportRelation::SupportRelation()
{
	m_childProbGivenParent = 0;
	m_parentProbGivenChild = 0;
}

SupportRelation::SupportRelation(const QString &parentName, const QString &childName, const QString &supportType)
{
	m_suppRelKey = m_parentName + "_" + m_childName + "_" + m_supportType;
	m_childProbGivenParent = 0;
	m_parentProbGivenChild = 0;
}

SupportRelation::~SupportRelation()
{

}

void SupportRelation::loadFromStream(QTextStream &ifs)
{
	QString currLine;

	currLine = ifs.readLine();
	std::vector<std::string> parts = PartitionString(currLine.toStdString(), "_");

	m_parentName = toQString(parts[0]);
	m_childName = toQString(parts[1]);
	m_supportType = toQString(parts[2]);
	m_suppRelKey = m_parentName + "_" + m_childName + "_" + m_supportType;

	currLine = ifs.readLine();
	std::vector<float> floatList = StringToFloatList(currLine.toStdString(),"");
	m_childProbGivenParent = (double)floatList[0];
	m_parentProbGivenChild = (double)floatList[1];
}

CoOccurrenceModel::CoOccurrenceModel()
{
	m_firstObjNum = 0;
	m_secondObjNum = 0;
	m_coOccNum = 0;
	m_prob = 0;
}

CoOccurrenceModel::CoOccurrenceModel(const QString &firstObjName, const QString &secondObjName, const QString &anchorObjName, const QString &conditionName)
:m_firstObjName(firstObjName), m_secondObjName(secondObjName), m_conditionName(conditionName), m_anchorObjName(anchorObjName)
{
	m_coOccurKey = m_firstObjName + "_" + m_secondObjName + "_" + m_conditionName + "_" + m_anchorObjName;

	m_firstObjNum = 0;
	m_secondObjNum = 0;
	m_coOccNum = 0;
	m_prob = 0;
}

void CoOccurrenceModel::loadFromStream(QTextStream &ifs)
{
	QString currLine;

	currLine = ifs.readLine();
	std::vector<std::string> parts = PartitionString(currLine.toStdString(), ",");
	m_coOccurKey = toQString(parts[0]);
	m_prob = StringToFloat(parts[1]);

	std::vector<std::string> subParts = PartitionString(parts[0], "_");
	m_firstObjName = toQString(subParts[0]);
	m_secondObjName = toQString(subParts[1]);
	m_conditionName = toQString(subParts[2]);
	m_anchorObjName = toQString(subParts[3]);

	currLine = ifs.readLine();
	std::vector<int> intList = StringToIntegerList(currLine.toStdString(), "", ",");
	m_coOccNum = intList[0];
	m_firstObjNum = intList[1];
	m_secondObjNum = intList[2];
}
