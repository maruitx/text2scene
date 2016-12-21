#include "SemGraphMatcher.h"
#include "SceneSemGraphManager.h"
#include "SceneSemGraph.h"
#include "TextSemGraph.h"


SemGraphMatcher::SemGraphMatcher(SceneSemGraphManager *ssgManager)
	:m_sceneSemGraphManager(ssgManager)
{

}


SemGraphMatcher::~SemGraphMatcher()
{
}

void SemGraphMatcher::updateCurrentTextSemGraph(TextSemGraph *tsg)
{
	m_currTextSemGraph = tsg;
}

vector<SceneSemGraph*> SemGraphMatcher::matchTSGWithSSGs(int topMacthNum)
{
	vector<pair<double, SceneSemGraph *>> evaluatedSSGs;

	for (int i = 0; i < m_sceneSemGraphManager->m_ssgNum; i++)
	{
		SceneSemGraph *currSSG = m_sceneSemGraphManager->getGraph(i);

		//double simVal = rand();
		double simVal = computeSimilarity(m_currTextSemGraph, currSSG);
		evaluatedSSGs.push_back(make_pair(simVal, currSSG));
	}

	sort(evaluatedSSGs.begin(), evaluatedSSGs.end());  // ascending order
	reverse(evaluatedSSGs.begin(), evaluatedSSGs.end()); // decending order

	vector<SceneSemGraph*> topSSGs(topMacthNum);
	for (int i = 0; i < topMacthNum; i++)
	{
		topSSGs[i] = evaluatedSSGs[i].second;
	}

	return topSSGs;
}

double SemGraphMatcher::computeSimilarity(TextSemGraph *tsg, SceneSemGraph *ssg)
{
	double simVal = 0;

	for (int i = 0; i < tsg->m_nodeNum; i++ )
	{
		SemNode tsgNode = tsg->m_nodes[i];
		for (int j = 0; j < ssg->m_nodeNum; j++)
		{
			SemNode ssgNode = ssg->m_nodes[j];
			if (tsgNode.nodeName == ssgNode.nodeName)
			{
				simVal += 1;
				break;
			}
		}
	}

	return simVal;
}
