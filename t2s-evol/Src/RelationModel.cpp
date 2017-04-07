#include "RelationModel.h"


PairwiseRelationModel::PairwiseRelationModel(const QString &anchorName, const QString &actName, const QString &conditionName, const QString & relationName /*= "general"*/)
	:m_anchorObjName(anchorName), m_actObjName(actName), m_conditionName(conditionName), m_relationName(relationName)

{
	m_relationKey = m_anchorObjName + "_" + m_actObjName + "_" + m_conditionName + "_" + m_relationName;
	m_GMM = NULL;
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
	if (m_GMM != NULL)
	{
		return m_GMM->sample();
	}
	else
	{
		int id = GenRandomInt(0, m_instances.size());
		RelativePos *relPos = m_instances[id];
		Eigen::VectorXd randObservation(4);
		randObservation[0] = relPos->pos.x;
		randObservation[1] = relPos->pos.y;
		randObservation[2] = relPos->pos.z;
		randObservation[3] = relPos->theta;

		return randObservation;
	}
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
}

GroupRelationModel::~GroupRelationModel()
{

}

void GroupRelationModel::loadModel()
{

}
