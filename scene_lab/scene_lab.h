#ifndef SCENE_LAB_H
#define SCENE_LAB_H

#include <QObject>

class scene_lab_widget;
class CScene;

class scene_lab : public QObject
{
	Q_OBJECT

public:
	scene_lab(QObject *parent = 0);
	~scene_lab();

	void create_widget();
	void destroy_widget();
	CScene *getScene() { return m_scene; };

public slots:
	void loadScene();
	void loadSceneList();

	void updateSceneRenderingOptions();

signals:
	void sceneLoaded();
	void sceneRenderingUpdated();

private:
	scene_lab_widget *m_widget;
	CScene *m_scene;	
};

#endif // SCENE_LAB_H
