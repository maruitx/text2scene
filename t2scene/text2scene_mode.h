#pragma once

#include "StarlabDrawArea.h"
#include "ModePlugin.h"

#include "text_scene_widget.h"

class scene_lab;
class CScene;
class CModel;
class SynScene;

class text2scene_mode : public ModePlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "text2scene_mode_plugin")
	Q_INTERFACES(ModePlugin)
	
public:	
	text2scene_mode();
	~text2scene_mode();

	QIcon icon(){ return QIcon(":/t2s_icon.png"); }
	void create();
	void destory();
	void decorate();

	void setDecorateScene(CScene *s) { m_decorateScene = s; };
	Starlab::DrawArea* getDrawArea() { return drawArea(); };

public slots:
	void showSceneLab(bool isShow);

	void setSceneBounds();
	void resetDecorateScene();
	void updateDecorateScene();

private:
	bool isApplicable() { return true; }



private:
	text_scene_widget *m_text_scene_widget;
	scene_lab *m_scenelab;

	SynScene *m_synScene;
	CScene *m_decorateScene;
};

