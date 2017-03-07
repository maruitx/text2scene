#pragma once

#include "MetaData.h"
#include "box_bvh.h"
#include "mesh_bvh.h"

class Object;
class Model;
class SceneSemGraph;
class CollisionManager;

class TSScene
{
public:
	TSScene(unordered_map<string, Model*> &models); // empty scenes
	TSScene(unordered_map<string, Model*> &models, MetaScene &ms); // init with current loaded object DB
	TSScene(unordered_map<string, Model*> &models, const QString &fileName);    // init with current loaded object DB and scene file
	~TSScene();

	void loadSceneFile(const QString &filename);
	void render(const Transform &trans, bool applyShadow);
	void renderDepth(const Transform &trans);
    void renderSceneBB(const Transform &trans);

	void makeRandom();

	void countLoadedModelNum(); // count loaded model number by finding model in loaded modelDB
    void computeSceneBB();

	bool resolveCollision(int modelId);

	void loadModel(MetaModel m);

	void updateRoomModel(MetaModel m);
    void toggleRenderMode();

	MetaScene& getMetaScene() { return m_metaScene; };
	MetaModel& getMetaModel(int idx) { return m_metaScene.m_metaModellList[idx]; };
	int modelNum() { return m_modelNum; };

	Model* getModel(const string &name);

public:
	SceneSemGraph *m_ssg;
	bool m_isLoadFromFile;
	bool m_isRenderRoom;

	int m_previewId;

	CollisionManager *m_collisionManager;

private:
	unordered_map<string, Model*> &m_models;   // current loaded object DB
	
	MetaScene m_metaScene;

	BoundingBox m_sceneBB;
    vec3 m_camTrans;

	int m_modelNum;
	int m_frameCount;

	int m_loadedModelNum;
	bool m_isLoadingDone;

    int m_renderMode;    
};

