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
	void bindBySynthesizedRelationships(SceneSemGraph *targetSg);


	mat4 computeTransMat(const MetaModel &fromModel, const MetaModel &toModel);

private:
	unordered_map<string, Model*> m_models;
	SemGraphMatcher *m_semanticGraphMatcher;
	SceneSemGraphManager *m_sceneSemGraphManager;

	TSScene *m_currTSScene;
	SceneSemGraph *m_currUserSSG;
	TextSemGraph *m_textSSG;

	LayoutPlanner *m_layoutPlanner;

	std::map<int, int> m_matchToNewUserSsgNodeMap;
};

