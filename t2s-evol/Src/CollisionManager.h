#pragma once

#include "box_bvh.h"
//#include "mesh_bvh.h"

class TSScene;
class Model;

class CollisionManager
{
public:
	CollisionManager(TSScene *s);
	~CollisionManager();

	bool checkCollision(Model *testModel, int testModelIdx);
	bool intersectAABB(const vec3 &miA, const vec3 &maA, const vec3 &miB, const vec3 &maB, double delta = 0);

	bool checkCollisionBVH(Model *testModel, int testMetaModelIdx);
	void rebuildBVH(Model *testModel, int testModelIdx);

	bool resolveCollision(int metaModelID);

	bool isPosCloseToInvalidPos(const vec3 &pos, int metaModelId);

	int m_trialNumLimit;

private:
	TSScene *m_scene;
	double m_sceneMetric;
	double m_closeSampleTh;  // threshold for avoiding close sample

	std::vector<BoxBvh<SimpleBoxNodeData>*> m_boxBVHs;  // box bvhs for model list

	std::vector<std::vector<vec3>> m_invalidPositions;  // invalid positions of models identified in previous collision detection

};

