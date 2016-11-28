#include "text2scene_mode.h"
#include "ModePluginDockWidget.h"

#include "../scene_lab/scene_lab.h"
#include "../common/geometry/Scene.h"
#include "SynScene.h"


text2scene_mode::text2scene_mode()
{
	m_text_scene_widget = NULL;
	m_scenelab = NULL;

	m_decorateScene = NULL;
	m_synScene = NULL;
}

text2scene_mode::~text2scene_mode()
{
	this->destory();
}

void text2scene_mode::create()
{
	if (!m_text_scene_widget)
	{
		ModePluginDockWidget * dockwidget = new ModePluginDockWidget("Text to Scene", mainWindow());
		m_text_scene_widget = new text_scene_widget(this);

		dockwidget->setWidget(m_text_scene_widget);
		mainWindow()->addDockWidget(Qt::BottomDockWidgetArea, dockwidget);

		drawArea()->setBackgroundColor(QColor(255, 255, 255, 255));
		drawArea()->setShortcut(QGLViewer::DRAW_AXIS, Qt::Key_A);
	}

	m_scenelab = new scene_lab();
	connect(m_scenelab, SIGNAL(sceneLoaded()), this, SLOT(resetDecorateScene()));
	connect(m_scenelab, SIGNAL(sceneRenderingUpdated()), this, SLOT(updateDecorateScene()));

	m_scenelab->create_widget();
}

void text2scene_mode::destory()
{
	if (!m_text_scene_widget)
	{
		delete m_text_scene_widget;
	}
}

void text2scene_mode::decorate()
{
	if (m_decorateScene != NULL)
	{
		m_decorateScene->draw();
	}
}


void text2scene_mode::showSceneLab(bool isShow)
{
	if (isShow)
	{
		m_scenelab->create_widget();
	}
	else
	{
		m_scenelab->destroy_widget();
	}
}

void text2scene_mode::setSceneBounds()
{
	if (m_decorateScene == NULL)
	{
		drawArea()->setSceneRadius(2);
		drawArea()->setSceneCenter(qglviewer::Vec(0, 0, 0));
		drawArea()->setSceneBoundingBox(qglviewer::Vec(-1, -1, -1), qglviewer::Vec(1, 1, 1));
		drawArea()->camera()->setPosition(qglviewer::Vec(0, -3, 2));
		drawArea()->showEntireScene();
		drawArea()->updateGL();
		return;
	}

	else
	{
		MathLib::Vector3 a = m_decorateScene->getMinVert();
		MathLib::Vector3 b = m_decorateScene->getMaxVert();

		qglviewer::Vec vecA(a[0], a[1], a[2]);
		qglviewer::Vec vecB(b[0], b[1], b[2]);

		drawArea()->setSceneCenter((vecA + vecB) * 0.5);
		drawArea()->setSceneRadius((vecB - vecA).norm()*0.5);
		drawArea()->setSceneBoundingBox(vecA, vecB);

		drawArea()->showEntireScene();
		drawArea()->updateGL();
	}
}

void text2scene_mode::updateDecorateScene()
{
	m_decorateScene = m_scenelab->getScene();
	drawArea()->updateGL();

	std::cout << "Text2Scene: scene updated.\n";
}

void text2scene_mode::resetDecorateScene()
{
	m_decorateScene = m_scenelab->getScene();

	drawArea()->camera()->setViewDirection(qglviewer::Vec(0, 1, 0));
	setSceneBounds();

	std::cout << "Text2Scene: scene view reset.\n";
}