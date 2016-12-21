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
#include "TextSemGraphManager.h"
#include "SceneSemGraphManager.h"
#include "SceneSemGraph.h"

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
    m_lights.push_back(new Light(this, Light::SPOT_LIGHT, vec3(0.9f), vec3(14.0f, 35.0f, 12.0f),  vec3(), vec3(1.2f), vec3(), vec3(0.7f, 0.001f, 0.0001f)));   
	m_niceGrid = new NiceGrid(100.0f, 40.0f);      
	
	initSynScene();
	//initTextures();

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

void Scene::initTextures()
{
	QDir dir(QString(params::inst()->textureDirectory.c_str()));

	QStringList filters;
	filters << "*.jpg";

	QFileInfoList list = dir.entryInfoList(filters);

	for (const auto &i : list)
	{
		Texture *tex = new Texture(i.absoluteFilePath());
		tex->setEnvMode(GL_REPLACE);
		tex->setWrapMode(GL_REPEAT);
		tex->setFilter(GL_LINEAR, GL_LINEAR);
		params::inst()->textures.insert(make_pair(i.baseName().toStdString(), tex));
	}
}

void Scene::initSynScene()
{
	//const string directory = "./SceneDB/StanfordSceneDB/";  
	////const string directory = "L:/sceneSynthesisDatabase/databaseFull/";
	//const string scene = "scene00003.txt";

	//params::inst()->sceneDirectory   = directory + "scenes/";
	//params::inst()->modelDirectory   = directory + "models/";
	//params::inst()->textureDirectory = directory + "textures/";

	//for (int i = 0; i < 15; ++i)
	//{
	//	TSScene *s = new TSScene(m_models, QString((params::inst()->sceneDirectory + scene).c_str()));
	//	if (i > 0)
	//		s->makeRandom();

	//	m_variations.push_back(s);
	//}

	// SceneDB is just outside of the git-root folder
	const string mainSceneDBDirectory = "../../SceneDB";

	m_textSemGraphManager = new TextSemGraphManager();
	m_sceneSemGraphManager = new SceneSemGraphManager(QString(mainSceneDBDirectory.c_str()));

	// temporary setting
	params::inst()->sceneDirectory = mainSceneDBDirectory + "/StanfordSceneDB/scenes/";
	params::inst()->modelDirectory = mainSceneDBDirectory + "/StanfordSceneDB/models/";
	params::inst()->textureDirectory = mainSceneDBDirectory + "/StanfordSceneDB/textures/";


	for (int i = 0; i < 4; ++i)
	{
		SceneSemGraph *ssg = m_sceneSemGraphManager->getGraph(i);
		TSScene *s = ssg->covertToTSScene(m_models);

		m_variations.push_back(s);
	}
}

void Scene::renderSynScene(const Transform &trans, int var, bool applyShadow)
{
	glEnable(GL_DEPTH_TEST);
	
	if (params::inst()->polygonMode == 1)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	m_variations[var]->render(trans, applyShadow);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Scene::renderSynSceneDepth(const Transform &trans, int var)
{
	glEnable(GL_DEPTH_TEST);

	if (params::inst()->polygonMode == 1)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	m_variations[var]->renderDepth(trans);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Scene::runOneEvolutionStep()
{
	QString filename = "../matt-SEL-playground/out.txt";
	m_textSemGraphManager->loadSELFromOutput(filename);

	TextSemGraph* activeTextSemGraph = m_textSemGraphManager->getActiveGraph();


	m_textSemGraphManager->updateActiveGraphId();

}

