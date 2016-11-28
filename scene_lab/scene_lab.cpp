#include "scene_lab.h"
#include "scene_lab_widget.h"
#include "../common/geometry/Scene.h"

scene_lab::scene_lab(QObject *parent)
	: QObject(parent)
{
	m_widget = NULL;
	m_scene = NULL;
}

scene_lab::~scene_lab()
{
	if (m_widget != NULL)
	{
		delete m_widget;
	}
}

void scene_lab::create_widget()
{
	m_widget = new scene_lab_widget(this);
	m_widget->show();
	m_widget->move(0, 200);

	std::cout << "SceneLab: widget created.\n";
}

void scene_lab::loadScene()
{
	CScene *scene = new CScene();
	scene->loadSceneFile(m_widget->loadSceneName(), 0, 0);

	m_scene = scene;

	std::cout << "SceneLab: scene loaded.\n";

	emit sceneLoaded();
}

void scene_lab::loadSceneList()
{
	//CScene *scene = new CScene();
	//scene->loadSceneFile(m_widget->loadSceneName(), 0, 0);
}

void scene_lab::destroy_widget()
{
	if (m_widget != NULL)
	{
		delete m_widget;
	}
}

void scene_lab::updateSceneRenderingOptions()
{
	bool showModelOBB = m_widget->ui->showOBBCheckBox->isChecked();
	bool showSuppGraph = m_widget->ui->showGraphCheckBox->isChecked();

	if (m_scene != NULL)
	{
		m_scene->setShowModelOBB(showModelOBB);
		m_scene->setShowSceneGraph(showSuppGraph);

	}
	else
	{
		m_widget->ui->showOBBCheckBox->setChecked(false);
		m_widget->ui->showGraphCheckBox->setChecked(false);
	}

	emit sceneRenderingUpdated();
}
