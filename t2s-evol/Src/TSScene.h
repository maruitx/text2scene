#pragma once

#include "MetaData.h"
#include "box_bvh.h"
#include "mesh_bvh.h"
#include "RelationModelManager.h"

class Object;
class Model;
class SceneSemGraph;
class CollisionManager;
class LayoutPlanner;


class TSScene
{
public:
	TSScene(unordered_map<string, Model*> &models); // empty scenes
	TSScene(unordered_map<string, Model*> &models, SceneSemGraph *ssg); // init with current loaded object DB
	TSScene(unordered_map<string, Model*> &models, const QString &fileName);    // init with current loaded object DB and scene file
	~TSScene();

	void loadSceneFile(const QString &filename);
	void render(const Transform &trans, bool applyShadow);
	void renderDepth(const Transform &trans);
    void renderSceneBB(const Transform &trans);

	void makeRandom();

	void countLoadedModelNum(); // count loaded model number by finding model in loaded modelDB
    void computeSceneBB();

	void loadModel(MetaModel m);


	void updateFloorHeight();
	void updateRoomModel(MetaModel m);
    void toggleRenderMode();

	MetaScene& getMetaScene() { return m_metaScene; };
	MetaModel& getMetaModel(int idx) { return m_metaScene.m_metaModellList[idx]; };
	int modelNum() { return m_modelNum; };

	Model* getModel(const string &name);
	bool computeZForModel(int currModelId, int parentModelId, vec3 startPt, double &newZ);
	void adjustZForSpecificModel(const MetaModel &currMd, double &z);

	bool isLayoutDone();

public:
	SceneSemGraph *m_ssg;
	bool m_isLoadFromFile;
	bool m_isRenderRoom;

	double m_floorHeight;

	int m_previewId;

	CollisionManager *m_collisionManager;  // each scene has its own collision manager
	LayoutPlanner *m_layoutPlanner;   // pointer to the singleton; instance saved in SceneGenerator
	RelationModelManager *m_relModelManager;    // pointer to the singleton; instance saved in SceneGenerator

	bool m_allConstraintsExtracted;
	std::vector<std::vector<RelationConstraint>> m_explictConstraints;  // semantic pairwise or group constraint
	std::vector<std::vector<RelationConstraint>> m_implicitConstraints;  // implicit relative constraints
	
	std::vector<int> m_placedObjIds;
	std::vector<int> m_toPlaceModelIds;

	std::vector<std::vector<vec3>> m_overHangPositions;

	MetaScene m_metaScene;

	bool m_sceneLoadingDone;
	bool m_sceneLayoutDone;
private:
	unordered_map<string, Model*> &m_models;   // current loaded object DB	

	BoundingBox m_sceneBB;
    vec3 m_camTrans;

	int m_modelNum;
	int m_frameCount;

	int m_loadedModelNum;


    int m_renderMode;    
};

