#ifndef SCENE_H
#define SCENE_H

#include "Headers.h"
#include "Geometry.h"
#include "Variation.h"
#include "TSScene.h"

class NiceGrid;
class Light;
class VertexBufferObject;
class Shader;
class CameraManager;
class Object;
class TransformFeedback;
class TSScene;
class TextSemGraphManager;
class SceneGenerator;

class Scene
{
public:
    Scene(CameraManager *camManager);
    ~Scene();

	void update(float delta);
    void init();

    void renderWorld(const Transform &trans, bool applyShadow = true);  
	void renderSynScene(const Transform &trans, int var, bool applyShadow = true);
	void renderSynSceneDepth(const Transform &trans, int var);

    void select(const Transform &trans, int sw, int sh, int mx, int my);
    void move(const Transform &trans, int x, int y);
    void resetSelection();

	void initSynScene();
	void initTextures();

	void runOneEvolutionStep();
    void toggleRenderMode();


public:
    vector<Light *> m_lights;
    vector<TSScene *> m_variations;
	TextSemGraphManager *m_textSemGraphManager;
	SceneGenerator *m_sceneGenerator;

	int m_activeVarationId;

	MetaModel roomModel;

private:
	
    NiceGrid *m_niceGrid;
    CameraManager *m_cameraManager;
    
    int m_activeIdx;

    unordered_map<string, Model *> m_models;
	int m_previewNum;
};

#endif

 