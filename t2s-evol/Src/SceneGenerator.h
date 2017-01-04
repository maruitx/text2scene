#pragma once

#include "Headers.h"

class SemGraphMatcher;
class TextSemGraph;
class SceneSemGraph;
class SceneSemGraphManager;
class TSScene;
class Model;

class SceneGenerator
{
public:
	SceneGenerator(const QString &sceneDBPath, unordered_map<string, Model*> &models);
	~SceneGenerator();

	void setCurrentTextGraph(TextSemGraph *tsg);
	std::vector<TSScene*> generateTSScenes(int num);

private:
	unordered_map<string, Model*> m_models;
	SemGraphMatcher *m_semanticGraphMatcher;
	SceneSemGraphManager *m_sceneSemGraphManager;
};

