#pragma once

#include "Headers.h"

class SceneSemGraphManager;
class SemanticGraph;
class TextSemGraph;
class SceneSemGraph;

class SemGraphMatcher
{
public:
	SemGraphMatcher(SceneSemGraphManager *ssgManager);
	~SemGraphMatcher();

	void updateCurrentTextSemGraph(TextSemGraph *tsg);

	vector<SceneSemGraph*> alignmentTSGWithDatabaseSSGs(int topMatchNum);
	SceneSemGraph* alignTSGWithSSG(TextSemGraph *tsg, SceneSemGraph *databaseSSG, double &matchingScore);

	double computeSimilarity(TextSemGraph *tsg, SceneSemGraph *ssg);

private:
	SceneSemGraphManager *m_sceneSemGraphManager;
	TextSemGraph *m_currTextSemGraph;
};

