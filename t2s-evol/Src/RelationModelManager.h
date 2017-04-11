#pragma once
#include "RelationModel.h"
#include <Eigen/Dense>

class GroupRelationModel;
class TSScene;
class MetaModel;
class Model;

const QString ConditionName[] = { "parentchild", "sibling", "proximity" };

class RelationConstraint
{
public:
	RelationConstraint(PairwiseRelationModel *m, const QString t, int anchorId);
	~RelationConstraint() {};

	PairwiseRelationModel *relModel;
	QString relationType; // constraint belong to relative, pairwise or group

	int anchorObjId; // id of anchor obj in modelList
};


class RelationModelManager
{
public:
	RelationModelManager();
	~RelationModelManager();

	void loadRelationModels();
	void loadRelativeRelationModels();
	void loadPairwiseRelationModels();
	void loadGroupRelationModels();

	bool isRelationsViolated(TSScene *currScene, int metaModelId);
	bool isConstraintViolated(TSScene *currScene, const MetaModel &md, const RelationConstraint &relConstraint);

	double computeRelationScore(TSScene *currScene, int metaModelId, const Eigen::VectorXd &currPlacement);
	double computeScoreForConstraint(TSScene *currScene, const RelationConstraint &relConstraint, const Eigen::VectorXd &currPlacement);

	double computeRelationScoreForGroup(TSScene *currScene, std::vector<int> metaModelIds, const std::vector<Eigen::VectorXd> &currPlacements);

	double computeLayoutPassScore(TSScene *currScene, int metaModelId);

	Eigen::VectorXd sampleNewPosFromConstraints(TSScene *currScene, int metaModelId, int &anchorModelId);

	void sampleFromRelationModel(TSScene *currScene, const RelationConstraint &relConstraint, int metaModelId, 
		int &anchorModelId, vec3 &newPos, double &newTheta);
	void randomSampleOnParent(TSScene *currScene, int metaModelId, vec3 &newPos, int &parentMetaModelId);
	double findClosestSuppPlaneZ(TSScene *currScene, int metaModelId, const vec3 &newPos);

	void collectConstraintsForModel(TSScene *currScene, int metaModelId);
	PairwiseRelationModel* retrievePairwiseModel(const QString &anchorObjName, const QString &actObjName, const QString &relationName);

	void extractRelPosToAnchor(TSScene *currScene, const MetaModel &anchorMd, const Eigen::VectorXd &currPlacement, RelativePos *relPos);
	void extractRelPosForModelPair(TSScene *currScene, const MetaModel &anchorMd, const MetaModel &actMd, RelativePos *relPos);
	mat4 getModelToUnitboxMat(Model *m, const MetaModel &md);

	void updateCollisionPostions(const std::vector<std::vector<vec3>> &collisionPositions);
	bool isPosValid(TSScene *currScene, const vec3 &pos, int metaModelId);
	bool isPosCloseToInvalidPos(const vec3 &pos, int metaModelId);

public:
	std::map<QString, PairwiseRelationModel*> m_relativeModels;  // all relative models with general relations
	std::map<QString, PairwiseRelationModel*> m_pairwiseRelModels;  // all pairwise relation models
	std::map<QString, GroupRelationModel*> m_groupRelModels;  // all group relations

	std::map<std::string, mat4> m_loadModelToUnitboxMat;

	std::vector<std::vector<vec3>> m_collisionPositions;  // invalid positions including collision, over-hang
	
	double m_closeSampleTh;  // threshold for avoiding close sample
	double m_sceneMetric;
};
