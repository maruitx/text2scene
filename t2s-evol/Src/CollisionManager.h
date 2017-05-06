#pragma once

#include "box_bvh.h"

class TSScene;
class Model;

struct TriangleTrianglePredicate : public AbstractTriangleTrianglePredicate<SimpleBoxNodeData>
{
	Model* model[2];
	mat4 transform[2];

	TriangleTrianglePredicate(Model *first, mat4 &firstTransform, Model *second, mat4 &secondTransform);
	bool operator()(const SimpleBoxNodeData &a, const SimpleBoxNodeData &b) const;
};

class CollisionManager
{
public:
	CollisionManager(TSScene *s);
	~CollisionManager();

	bool checkCollision(Model *testModel, int testModelIdx);
	bool intersectAABB(const vec3 &miA, const vec3 &maA, const vec3 &miB, const vec3 &maB, double delta = 0);

	bool checkCollisionBVH(Model *testModel, int testMetaModelIdx);
	void rebuildBVH(Model *testModel, int testModelIdx);

	bool isRayIntersect(const Ray &ray, int parentModelId, double &z);
	std::vector<std::vector<vec3>> m_collisionPositions;  // invalid positions of models identified in previous collision detection

private:
	TSScene *m_scene;
	std::vector<BoxBvh<SimpleBoxNodeData>*> m_boxBVHs;  // box bvhs for model list
};

