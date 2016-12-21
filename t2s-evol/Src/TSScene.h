#pragma once

#include "MetaData.h"

class Object;

class Model;

class TSScene
{
public:
	TSScene(unordered_map<string, Model*> &models, MetaScene &ms); // init with current loaded object DB
	TSScene(unordered_map<string, Model*> &models, const QString &fileName);    // init with current loaded object DB and scene file
	~TSScene();

	void loadSceneFile(const QString filename, int obbOnly = false);
	void render(const Transform &trans, bool applyShadow);
	void renderDepth(const Transform &trans);

	void makeRandom();

private:
	unordered_map<string, Model*> &m_models;   // current loaded object DB
	
	MetaScene m_metaScene;

	BoundingBox m_sceneBB;

	int m_modelNum;
	int m_frameCount;
};

