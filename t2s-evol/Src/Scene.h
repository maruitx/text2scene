#ifndef SCENE_H
#define SCENE_H

#include "Headers.h"
#include "Geometry.h"
#include "Variation.h"

class NiceGrid;
class Light;
class VertexBufferObject;
class Shader;
class CameraManager;
class Object;
class TransformFeedback;

class Scene
{
public:
    Scene(CameraManager *camManager);
    ~Scene();

	void update(float delta);
    void init();

    void renderWorld(const Transform &trans);  
    void renderObjects(const Transform &trans);  
    void renderObjectsDepth(const Transform &trans);
    void renderVariation(const Transform &trans, int var);
    void renderVariationDepth(const Transform &trans, int var);

    void select(const Transform &trans, int sw, int sh, int mx, int my);
    void move(const Transform &trans, int x, int y);
    void resetSelection();

    void initObjects();
    void initVariations();

public:
    vector<Light *> m_lights;
    vector<Variation> m_variations;

private:
	
    NiceGrid *m_niceGrid;
    CameraManager *m_cameraManager;
    
    VertexBufferObject *m_vbo;
    vector<VertexBufferObject*> m_vbos;

    int m_activeIdx;

    unordered_map<string, Object *> m_objects;    
};

#endif

 