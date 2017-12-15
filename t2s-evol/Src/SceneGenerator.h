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
	void adjustTextSSGWithCurrSSG(TextSemGraph *textSSG, SceneSemGraph *currSSG);
	void resetNodes(SemanticGraph *sg);

	std::vector<TSScene*> generateTSScenes(int num);
	void executeCommandsToCurrentScene();

	// scene binding
	SceneSemGraph* accommodateToCurrTSScene(SceneSemGraph *matchedSg);
	RelationModelManager *getRelationManager() { return m_relModelManager; };

	RelationModelManager *m_relModelManager;  // singleton, only one instance
	LayoutPlanner *m_layoutPlanner; // singleton, only one instance

private:
	unordered_map<string, Model*> &m_models;
	SemGraphMatcher *m_semanticGraphMatcher;
	SceneSemGraphManager *m_sceneSemGraphManager;

	TSScene *m_currScene;
	SceneSemGraph *m_currUserSSG;
	TextSemGraph *m_textSSG;



	std::map<int, int> m_matchToNewUserSsgNodeMap;
};

