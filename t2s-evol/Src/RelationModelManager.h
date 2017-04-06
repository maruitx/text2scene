#pragma once
#include "RelationModel.h"

class GroupRelationModel;
class TSScene;
class MetaModel;

const QString ConditionName[] = { "parentchild", "sibling", "proximity" };

struct RelationConstraint
{
	RelationConstraint(PairwiseRelationModel *m, const QString t, int anchorId) { relModel = m; relationType = t; anchorObjId = anchorId; };

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

	bool isRelationViolated(TSScene *currScene, int metaModelId);
	mat4 sampleFromExplicitRelation(TSScene *currScene, int metaModelId);

	void collectConstraintsForModel(TSScene *currScene, int metaModelId);
	PairwiseRelationModel* retrievePairwiseModel(const QString &anchorObjName, const QString &actObjName, const QString &relationName);

	void extractRelPosForModelPair(TSScene *currScene, const MetaModel &anchorModel, const MetaModel &actModel, RelativePos *relPos);


public:
	std::map<QString, PairwiseRelationModel*> m_relativeModels;  // all relative models with general relations
	std::map<QString, PairwiseRelationModel*> m_pairwiseRelModels;  // all pairwise relation models
	std::map<QString, GroupRelationModel*> m_groupRelModels;  // all group relations
};
