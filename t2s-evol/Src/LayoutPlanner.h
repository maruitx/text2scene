#pragma once

#include "MetaData.h"

class RelationModelManager;
class SceneSemGraph;
class TSScene;

class LayoutPlanner
{
public:
	LayoutPlanner();
	~LayoutPlanner();

	void initPlaceByAlignRelation();

	void adjustPlacement(int metaModelID);
	void adjustPlacementForSpecialModel(const MetaModel &currMd, vec3 &pos);


	mat4 computeTransMat(const MetaModel &fromModel, const MetaModel &toModel);

	void updateCollisionPostions(const std::vector<std::vector<vec3>> &collisionPositions);
	bool isPosCloseToInvalidPos(const vec3 &pos, int metaModelId);

public:

	RelationModelManager *m_relModelManager;

	// SSG used for alignment
	SceneSemGraph *m_matchedSg;
	SceneSemGraph *m_currSg;

	TSScene *m_currScene;

	double m_closeSampleTh;  // threshold for avoiding close sample
	double m_sceneMetric;
	std::vector<std::vector<vec3>> m_collisionPositions;  // invalid positions including collision, over-hang
};

