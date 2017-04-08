#pragma once

#include "MetaData.h"

class SceneSemGraph;
class TSScene;
class RelationModelManager;

class LayoutPlanner
{
public:
	LayoutPlanner(RelationModelManager *relManager);
	~LayoutPlanner();

	void initPlaceByAlignRelation(SceneSemGraph *matchedSg, SceneSemGraph *currSg);

	void computeLayout(TSScene *currScene);
	std::vector<int> makePlacementOrder(TSScene *currScene);

	void adjustPlacement(TSScene *currScene, int metaModelID, const std::vector<std::vector<vec3>> &collisonPositions);
	void adjustPlacementForSpecificModel(TSScene *currScene, const MetaModel &currMd, vec3 &pos);
	mat4 computeTransMat(TSScene *currScene, int anchorModelId, int currModelID, vec3 newPos, double newTheta);

	mat4 computeAlignTransMat(const MetaModel &fromModel, const MetaModel &toModel);

	void updateCollisionPostions(const std::vector<std::vector<vec3>> &collisionPositions);
	bool isPosCloseToInvalidPos(const vec3 &pos, int metaModelId);

public:

	RelationModelManager *m_relModelManager;  // pointer to the singleton; instance saved in SceneGenerator

	double m_closeSampleTh;  // threshold for avoiding close sample
	double m_sceneMetric;
	std::vector<std::vector<vec3>> m_collisionPositions;  // invalid positions including collision, over-hang
};

