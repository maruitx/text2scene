#pragma once
#include "RelationModel.h"

class GroupRelationModel;
class CScene;

class RelationModelManager
{
public:
	RelationModelManager();
	~RelationModelManager();

	void loadRelationModels();

public:
	std::map<QString, PairwiseRelationModel*> m_relativeModels;  // relative models with general relations
	std::map<QString, PairwiseRelationModel*> m_pairwiseRelModels;  // pairwise relation models
	std::map<QString, GroupRelationModel*> m_groupRelModels;


};

