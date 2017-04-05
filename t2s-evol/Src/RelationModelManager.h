#pragma once
#include "RelationModel.h"

class GroupRelationModel;
class TSScene;

const QString ConditionName[] = { "parentchild", "sibling", "proximity" };

struct RelationConstraint
{
	RelationConstraint(PairwiseRelationModel *m, const QString t) { relModel = m; relationType = t; };

	PairwiseRelationModel *relModel;
	QString relationType; // constraint belong to relative, pairwise or group
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

	bool isRelationViolated(int metaModelId);
	mat4 sampleTransformByRelation(int metaModelId);

	void collectConstraintsForModel(TSScene *currScene, int metaModelId);


public:
	std::map<QString, PairwiseRelationModel*> m_relativeModels;  // all relative models with general relations
	std::map<QString, PairwiseRelationModel*> m_pairwiseRelModels;  // all pairwise relation models
	std::map<QString, GroupRelationModel*> m_groupRelModels;  // all group relations
};
