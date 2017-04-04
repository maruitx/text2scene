#pragma once
#include "RelationModel.h"

class GroupRelationModel;
class TSScene;

struct RelationConstraint 
{
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

	void collectConstraintsForModel(int metaModelId);

public:
	std::map<QString, PairwiseRelationModel*> m_relativeModels;  // all relative models with general relations
	std::map<QString, PairwiseRelationModel*> m_pairwiseRelModels;  // all pairwise relation models
	std::map<QString, GroupRelationModel*> m_groupRelModels;  // all group relations

	RelationConstraint m_SemanticRelationConstraint;  // semantic pairwise or group constraint
	std::vector<RelationConstraint> m_RelariveRelationConstraints;  // implicit relative constraints

	TSScene *m_currScene;
};
