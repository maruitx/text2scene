#pragma once

#include "Headers.h"

class SemGraphMatcher;
class TextSemGraph;
class SceneSemGraph;
class SceneSemGraphManager;
class TSScene;
class Model;
class MetaModel;

class SceneGenerator
{
public:
	SceneGenerator(unordered_map<string, Model*> &models);
	~SceneGenerator();

	void updateCurrentTextGraph(TextSemGraph *tsg);
	void updateCurrentTSScene(TSScene *ts);

	std::vector<TSScene*> generateTSScenes(int num);

	// alignment
	SceneSemGraph* bindToCurrTSScene(SceneSemGraph *matchedSg);
	void geometryAlignmentWithCurrScene(SceneSemGraph *matchedSg, SceneSemGraph *targetSg);
	void alignBySynthesizedRelationships(SceneSemGraph *targetSg);


	mat4 computeTransMat(const MetaModel &fromModel, const MetaModel &toModel);

private:
	unordered_map<string, Model*> m_models;
	SemGraphMatcher *m_semanticGraphMatcher;
	SceneSemGraphManager *m_sceneSemGraphManager;

	TSScene *m_currTSScene;
	SceneSemGraph *m_currUserSSG;
	TextSemGraph *m_textSSG;

	std::map<int, int> m_mapFromMatchToNewNodeId;
};

