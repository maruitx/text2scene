#include "Scene.h"
#include "NiceGrid.h"
#include "Light.h"
#include "Shader.h"
#include "VertexBufferObject.h"
#include "Mesh.h"
#include "CameraManager.h"
#include "Object.h"
#include "Geometry.h"
#include "TransformFeedback.h"
#include "TSScene.h"

Scene::Scene(CameraManager *camManager)
: m_cameraManager(camManager),
  m_activeIdx(-1)
{
    init();   
}

Scene::~Scene()
{
}

void Scene::init()
{
    m_lights.push_back(new Light(this, Light::SPOT_LIGHT, vec3(0.9f), vec3(14.0f, 20.0f, 12.0f),  vec3(), vec3(1.2f), vec3(), vec3(0.7f, 0.001f, 0.0001f)));   
	m_niceGrid = new NiceGrid(100.0f, 40.0f);      

    initObjects();
    initVariations();
	initSynScene();

    stats.record("test1", new Statistics::Item<float>(1.1f));
    stats.record("test2", new Statistics::Item<int>(5));
}

void Scene::renderWorld(const Transform &trans, bool applyShadow)
{
    m_niceGrid->render(trans, applyShadow);

    if(params::inst()->renderMisc)
    {	         
        for(int i=0; i<m_lights.size(); ++i)
        {   
            m_lights[i]->render(trans);
        }

        m_cameraManager->renderCameras(trans);
    }
}

void Scene::renderObjects(const Transform &trans)
{
    for(int i=0; i<1; ++i)
    {
        m_variations[i].render(trans);
    }
}

void Scene::renderVariation(const Transform &trans, int var, bool applyShadow)
{
    glEnable(GL_DEPTH_TEST);
    m_variations[var].render(trans, applyShadow);
}

void Scene::renderObjectsDepth(const Transform &trans)
{
}

void Scene::renderVariationDepth(const Transform &trans, int var)
{
    m_variations[var].renderDepth(trans);
}
 
void Scene::update(float delta)
{
    for(int i=0; i<m_lights.size(); ++i)
    {
        m_lights[i]->update(delta);
    }
}

void Scene::select(const Transform &trans, int sw, int sh, int mx, int my)
{
    //float minDist = math_maxfloat;
    //int idx = -1;
    //
    //Picking pick;
    //for(int i=0; i<m_objects.size(); ++i)
    //{
    //    float t = m_objects[i]->selected(pick, trans, sw, sh, mx, my);
    //    if( t > 0.0f && t<minDist)
    //    {
    //        minDist = t;
    //        idx = i;
    //    }
    //}

    //if(idx >= 0)
    //{
    //    m_activeIdx = idx;
    //    m_objects[m_activeIdx]->m_isSelected = true;
    //}
}

void Scene::move(const Transform &trans, int x, int y)
{
	//vec3 dir, right, up, pos;
 //   getCameraFrame(trans, dir, up, right, pos);

 //   if(m_activeIdx >= 0)
 //   {
 //       m_objects[m_activeIdx]->move(x, y, dir, up, right, pos);
 //   }
}

void Scene::resetSelection()
{
    //for(int i=0; i<m_objects.size(); ++i)
    //{
    //    m_objects[i]->m_isSelected = false;
    //}

    //m_activeIdx = -1;
}

void Scene::initObjects()
{
   //Object *obj1 = new Object("Data/Objs/cube1.obj", true, true, true, vec3(), vec3(1.0f));
   //string id1 = "cube1";

   //Object *obj2 = new Object("Data/Objs/cube2.obj", true, true, true, vec3(), vec3(1.0f));
   //string id2 = "cube2";


   //m_objects.insert(make_pair(id1, obj1));
   //m_objects.insert(make_pair(id2, obj2));
}

void Scene::initVariations()
{
    int nrVariations = 5;

    for(int i=0; i<=nrVariations; ++i)
    {
        Variation v(m_objects);

        int r = i;//rand() % nrVariations;

        if(r == 1)
            v.makeExplode(2.0f);

        if(r == 2)
            v.makeScale(0.3f);


        if(r == 4)
			v.makeScale(0.1f);

        m_variations.push_back(v);
    }
}

void Scene::initSynScene()
{
	m_synScene = new TSScene(m_objects);

	QString filename = "./SceneDB/StanfordSceneDB/scenes/scene00003.txt";
	m_synScene->loadSceneFile(filename);
}

void Scene::renderSynScene(const Transform &trans, bool applyShadow /*= true*/)
{
	glEnable(GL_DEPTH_TEST);
	
	if (params::inst()->polygonMode == 1)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	m_synScene->render(trans, applyShadow);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
