#pragma once

#include "MetaData.h"
#include <Eigen/Dense>

class SceneSemGraph;
class TSScene;
class RelationModelManager;
class CollisionManager;

class LayoutPlanner
{
public:
	LayoutPlanner(RelationModelManager *relManager);
	~LayoutPlanner();

	void initPlaceByAlignRelation(SceneSemGraph *matchedSg, SceneSemGraph *currSg);
	void initPlaceUsingSynthesizedRelations(TSScene *currScene);

	void computeLayout(TSScene *currScene);
	void computeSingleObjLayout(TSScene *currScene, int metaModelId);
	void computeGroupObjLayoutSeq(TSScene *currScene, const std::vector<int> &toPlaceModelIds);
	bool doRollback(TSScene *currScene, std::vector<int> &tempPlacedIds, int currModelId);
	void adjustZForModel(TSScene *currScene, int metaModelId);

	std::vector<int> makeToPlaceModelIds(TSScene *currScene);
	std::vector<int> sortModelsByVolume(TSScene *currScene, const std::vector<int> &modelIds);

	void computeConstraintsForModels(TSScene *currScene, const std::vector<int> &toPlaceModelIds);
	void computeLayoutPassScoreForModels(TSScene *currScene, const std::vector<int> &toPlaceModelIds);

	Eigen::VectorXd computeNewPlacement(TSScene *currScene, int metaModelID, const std::vector<std::vector<vec3>> &collisonPositions, int &anchorModelId);
	void adjustPlacementForSpecificModel(TSScene *currScene, const MetaModel &currMd, vec3 &pos);

	void updateWithNewPlacement(TSScene *currScene, int anchorModelId, int currModelID, const Eigen::VectorXd &newPlacement);
	void updatePlacementOfChildren(TSScene *currScene, int currModelID, mat4 transMat);
	void initAlignmentOfChildren(SceneSemGraph *currSSG, int currModelId, mat4 transMat);
	void updatePlacementOfParent(TSScene *currScene, int currModelID, mat4 transMat);

	mat4 computeTransMatFromPos(TSScene *currScene, int anchorModelId, int currModelID, vec3 newPos, double newTheta);

	mat4 computeModelAlignTransMat(const MetaModel &fromModel, const MetaModel &toModel);

	Eigen::VectorXd makePlacementVec(vec3 pos, double theta);

public:

	RelationModelManager *m_relModelManager;  // pointer to the singleton; instance saved in SceneGenerator

	int m_trialNumLimit;

};

