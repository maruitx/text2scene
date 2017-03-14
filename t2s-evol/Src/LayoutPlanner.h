#pragma once

#include "MetaData.h"

class RelationModelManager;
class SceneSemGraph;

class LayoutPlanner
{
public:
	LayoutPlanner();
	~LayoutPlanner();

	void initPlaceByAlignRelation();


	mat4 computeTransMat(const MetaModel &fromModel, const MetaModel &toModel);

public:

	RelationModelManager *m_relModelManager;

	SceneSemGraph *m_matchedSg;
	SceneSemGraph *m_currSg;

};

