#pragma once

#include "Headers.h"

class SceneSemGraphManager;
class TextSemGraph;
class SceneSemGraph;

class SemGraphMatcher
{
public:
	SemGraphMatcher(SceneSemGraphManager *ssgManager);
	~SemGraphMatcher();

	void updateCurrentTextSemGraph(TextSemGraph *tsg);
	vector<SceneSemGraph*> matchTSGWithSSGs(int topMacthNum);

	double computeSimilarity(TextSemGraph *tsg, SceneSemGraph *ssg);

private:
	SceneSemGraphManager *m_sceneSemGraphManager;
	TextSemGraph *m_currTextSemGraph;



};

