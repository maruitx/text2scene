#pragma once

#include "Headers.h"

class SceneSemGraphManager;
class SemanticGraph;
class TextSemGraph;
class SceneSemGraph;
class GMTMatcher;
class Graph;  // GMT Graph
class InstanceData;

class SemGraphMatcher
{
public:
	SemGraphMatcher(SceneSemGraphManager *ssgManager);
	~SemGraphMatcher();

	void updateCurrentTextSemGraph(TextSemGraph *tsg);
	vector<SceneSemGraph*> testMatchTSGWithSSGs(int topMacthNum);
	vector<SceneSemGraph*> matchTSGWithSSGs(int topMacthNum);

	vector<SceneSemGraph*> alignmentTSGWithDatabaseSSGs(int topMatchNum);
	SceneSemGraph* alignTSGWithSSG(TextSemGraph *tsg, SceneSemGraph *databaseSSG, double &matchingScore);

	double computeSimilarity(TextSemGraph *tsg, SceneSemGraph *ssg);

	Graph* convertToGMTGraph(SemanticGraph *sg);
	SceneSemGraph* convertGMTInstanceToSSG(InstanceData *gmtInstance);

private:
	SceneSemGraphManager *m_sceneSemGraphManager;
	TextSemGraph *m_currTextSemGraph;

	GMTMatcher *m_gmtMatcher;

};

