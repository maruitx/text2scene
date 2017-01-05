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

	void updateCurrentTextGraph(TextSemGraph *tsg);
	void updateCurrentTSScene(TSScene *ts);

	std::vector<TSScene*> generateTSScenes(int num);

	SceneSemGraph* semanticAlignToCurrTSScene(SceneSemGraph *matchedSg);
	void geometryAlignToCurrTSScene();

private:
	unordered_map<string, Model*> m_models;
	SemGraphMatcher *m_semanticGraphMatcher;
	SceneSemGraphManager *m_sceneSemGraphManager;

	TSScene *m_currTSScene;
};

