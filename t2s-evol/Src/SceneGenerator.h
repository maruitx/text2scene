#pragma once

#include "Headers.h"

class SemanticGraph;
class TextSemGraph;
class SceneSemGraph;
class SemGraphMatcher;
class SceneSemGraphManager;
class TSScene;
class Model;
class MetaModel;

class RelationModelManager;
class LayoutPlanner;

class SceneGenerator
{
public:
	SceneGenerator(unordered_map<string, Model*> &models);
	~SceneGenerator();

	void updateCurrentTextGraph(TextSemGraph *tsg);
	void updateCurrentTSScene(TSScene *ts);

	SemanticGraph* prepareQuerySG();
	std::vector<TSScene*> generateTSScenes(int num);

	// scene binding
	SceneSemGraph* bindToCurrTSScene(SceneSemGraph *matchedSg);
	void geometryAlignmentWithCurrScene(SceneSemGraph *matchedSg, SceneSemGraph *targetSg);


private:
	unordered_map<string, Model*> m_models;
	SemGraphMatcher *m_semanticGraphMatcher;
	SceneSemGraphManager *m_sceneSemGraphManager;

	SceneSemGraph *m_currUserSSG;
	TextSemGraph *m_textSSG;

	RelationModelManager *m_relModelManager;  // singleton, only one instance
	LayoutPlanner *m_layoutPlanner; // singleton, only one instance

	std::map<int, int> m_matchToNewUserSsgNodeMap;
};

