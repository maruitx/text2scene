#pragma once

#include "Headers.h"

class SceneSemGraphManager;
class SemanticGraph;
class SceneSemGraph;

class SemGraphMatcher
{
public:
	SemGraphMatcher(SceneSemGraphManager *ssgManager);
	~SemGraphMatcher();

	void updateQuerySSG(SemanticGraph *sg);

	vector<SceneSemGraph*> alignmentSSGWithDatabaseSSGs(int topMatchNum);

	SceneSemGraph* alignSSGWithDBSSG(SemanticGraph *querySSG, SceneSemGraph *dbSSG, double &matchingScore);
	void alignObjectNodes(SemanticGraph *querySSG, SceneSemGraph *dbSSG, double &matchingScore);
	void alignRelationshipNodes(SemanticGraph *querySSG, SceneSemGraph *dbSSG, double &matchingScore);
	void addSynthNodeToSubSSG(SemanticGraph *querySSG, SceneSemGraph *matchedSubSSG);  // add unmatched nodes as synth nodes to Sub-SSG

	double computeSimilarity(SemanticGraph *tsg, SemanticGraph *ssg); // debug: simple compute similarity by matching node name

private:
	SceneSemGraphManager *m_sceneSemGraphManager;
	SemanticGraph *m_querySSG;

	std::map<int, int> m_queryToDBSsgNodeIdMap;

};

