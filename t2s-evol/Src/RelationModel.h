#pragma once
#include "Headers.h"
#include "GaussianMixtureModel.h"

class RelativePos
{
public:
	RelativePos() {};
	~RelativePos() {};

	vec3 pos;  // rel pos of actObj in anchor's unit frame
	double theta;
	mat4 anchorAlignMat;  // transformation of anchorObj to unit frame
	mat4 actAlignMat;  // transformation of actObj to anchorObj's unit frame,  anchorAlignMat*actInitTransMat
	double unitFactor;

	QString m_actObjName;
	QString m_anchorObjName;
	QString m_conditionName;
	QString m_instanceNameHash;  // anchorObjName_actObjName_conditionName


	int m_actObjId;
	int m_anchorObjId;
	QString m_sceneName;
	QString m_instanceIdHash;  // sceneName_anchorObjId_actObjId


	bool isValid;
};


class PairwiseRelationModel
{
public:
	PairwiseRelationModel(const QString &anchorName, const QString &actName, const QString &conditionName, const QString & relationName = "general");
	~PairwiseRelationModel();

	void loadFromStream(QTextStream &ifs);
	Eigen::VectorXd sample();
	bool hasCandiInstances();
	void updateCandiInstanceIds();

	int m_numGauss;
	GaussianMixtureModel *m_GMM;

	int m_numInstance;
	std::vector<RelativePos*> m_instances;

	QString m_relationKey;  // anchorName_actName_conditionName_relationName

	QString m_actObjName;
	QString m_anchorObjName;

	QString m_conditionName; // parent-child, sibling, proximity, or text-specified relation
	QString m_relationName;  // none, left, right, etc.

	std::vector<int> m_candidateInstanceIds;
	int m_lastSampleInstanceId;

	int m_modelId;
	std::vector<int> m_simModelIds; // list of similar relation model ids, sorted from high sim to low

	std::vector<std::vector<float>> m_avgObjFeatures;  // heightToFloor, modelHeight, modelVolume of anchor and act obj
	std::vector<std::vector<float>> m_maxObjFeatures;
};

class OccurrenceModel
{
public:
	OccurrenceModel(const QString &objName, int objNum);

	int m_objNum;
	int m_numInstance;   // number of observations
	double m_occurProb;

	QString m_objName; // active object name
					   //QString m_conditionName; // condition to the anchor object

	QString m_occurKey; // objName_objNum
};

class GroupRelationModel
{
public:
	GroupRelationModel(const QString &anchorObjName, const QString &relationName);
	~GroupRelationModel();

public:
	std::map<QString, PairwiseRelationModel*> m_pairwiseModels;  // relation-conditioned relative model
	std::vector<QString> m_pairwiseModelKeys;
	std::map<QString, OccurrenceModel*> m_occurModels;

	QString m_anchorObjName;
	QString m_relationName;

	QString m_groupKey;  // relationName_anchorObjName
	int m_numInstance;


};

