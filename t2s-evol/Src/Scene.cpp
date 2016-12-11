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
    m_lights.push_back(new Light(this, Light::SPOT_LIGHT, vec3(0.9f), vec3(0.0f, 20.0f, 0.1f),  vec3(), vec3(1.2f), vec3(), vec3(0.7f, 0.001f, 0.0001f)));   
	m_niceGrid = new NiceGrid(100.0f, 40.0f);      

    initObjects();
    initVariations();

    stats.record("test1", new Statistics::Item<float>(1.1f));
    stats.record("test2", new Statistics::Item<int>(5));
}

void Scene::renderWorld(const Transform &trans)
{
    m_niceGrid->render(trans);

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
 //   mat4 model = mat4::translate(0.0f, 2.0f, 0.0f);

 //   if (params::inst()->renderObjects)
 //   {
	//}

 //   Shader *shader = shaders::inst()->default;
 //   shader->bind();
 //       //shader->set3f("lightPos", params::inst()->lightPos);
 //       shader->setMatrices(trans, model, true, true, true);
 //       
 //       //m_vbo->render();    

 //   shader->release();	

    for(int i=0; i<1; ++i)
    {
        m_variations[i].render(trans);
    }
}

void Scene::renderVariation(const Transform &trans, int var)
{
    m_variations[var].render(trans);
}

void Scene::renderObjectsDepth(const Transform &trans)
{
    //mat4 model = mat4::translate(0.0f, 2.0f, 0.0f);

    //if (params::inst()->renderObjects)
    //{
    //}

    //Shader *shader = shaders::inst()->default;
    //shader->bind();
    //    shader->setMatrices(trans, model, true, true, true);
    //    
    //    //m_vbo->render();    

    //shader->release();
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
   Object *obj1 = new Object("Data/Objs/elephant.obj", true, true, true, vec3(), vec3(1.0f));
   string id1 = "elephant";

   Object *obj2 = new Object("Data/Objs/chair.obj", true, true, true, vec3(), vec3(1.0f));
   string id2 = "chair";

   Object *obj3 = new Object("Data/Objs/elk.obj", true, true, true, vec3(), vec3(1.0f));
   string id3 = "elk";

   Object *obj4 = new Object("Data/Objs/bunny_simple.obj", true, true, true, vec3(), vec3(1.0f));
   string id4 = "bunny_simple";

   m_objects.insert(make_pair(id1, obj1));
   m_objects.insert(make_pair(id2, obj2));
   m_objects.insert(make_pair(id3, obj3));
   m_objects.insert(make_pair(id4, obj4));
}

void Scene::initVariations()
{
    int nrVariations = 4;

    for(int i=0; i<=nrVariations; ++i)
    {
        Variation v(m_objects);

        int r = i;//rand() % nrVariations;

        if(r == 1)
            v.makeExplode(2.0f);

        if(r == 2)
            v.makeScale(0.3f);

        if(r == 3)
            v.makeBright(0.2);

        if(r == 4)
            v.makeBright(1.5);

        m_variations.push_back(v);
    }
}