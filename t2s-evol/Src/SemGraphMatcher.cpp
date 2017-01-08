#include "SemGraphMatcher.h"
#include "SceneSemGraphManager.h"
#include "SceneSemGraph.h"
#include "TextSemGraph.h"


SemGraphMatcher::SemGraphMatcher(SceneSemGraphManager *ssgManager)
	:m_sceneSemGraphManager(ssgManager)
{
	//m_gmtMatcher = new GMTMatcher();

	//cout << "SemGraphMatcher: start converting ssg to gmt graph...\n";

	//// add scene ssg to GTM graph database
	//for (int i = 0; i < m_sceneSemGraphManager->m_ssgNum; i++)
	//{
	//	SemanticGraph *sg = m_sceneSemGraphManager->getGraph(i);
	//	Graph *gmtGraph = convertToGMTGraph(sg);
	//	m_gmtMatcher->m_graphDatabase->addModel(gmtGraph, 0);

	//	cout << "\t finish converting " << i +1 << "/" << m_sceneSemGraphManager->m_ssgNum<<" graphs\r";
	//}

	cout << "\nSemGraphMatcher: loading semantic graphs number " << m_sceneSemGraphManager->m_ssgNum<<"\n";
}

SemGraphMatcher::~SemGraphMatcher()
{

}

void SemGraphMatcher::updateCurrentTextSemGraph(TextSemGraph *tsg)
{
	m_currTextSemGraph = tsg;
}


vector<SceneSemGraph*> SemGraphMatcher::alignmentTSGWithDatabaseSSGs(int topMatchNum)
{
	cout << "SemGraphMatcher: start graph matching.\n";

	vector<pair<double, SceneSemGraph *>> scoredDBSubSSGs;

	for (int i = 0; i < m_sceneSemGraphManager->m_ssgNum; i++)
	{
		SceneSemGraph *currDBSSG = m_sceneSemGraphManager->getGraph(i);

		double matchingScore = 0;
		SceneSemGraph *subSSG = alignTSGWithSSG(m_currTextSemGraph, currDBSSG, matchingScore);

		if (matchingScore > 0)
		{
			scoredDBSubSSGs.push_back(make_pair(matchingScore, subSSG));
		}
	}

	sort(scoredDBSubSSGs.begin(), scoredDBSubSSGs.end()); // ascending order
	reverse(scoredDBSubSSGs.begin(), scoredDBSubSSGs.end()); // descending order

	int matchedSSGNum = scoredDBSubSSGs.size();
	vector<SceneSemGraph*> matchedSubSSGs;

	int ssgNum = min(topMatchNum, matchedSSGNum);

	for (int i = 0; i < ssgNum; i++)
	{
		SceneSemGraph *ssg = scoredDBSubSSGs[i].second;

		if (ssg != NULL)
		{
			matchedSubSSGs.push_back(ssg);
		}
	}

	cout << "SemGraphMatcher: graph matching done, found instance number " << matchedSSGNum << ", shown instance number " << matchedSubSSGs.size() << ".\n";
	return matchedSubSSGs;
}


SceneSemGraph* SemGraphMatcher::alignTSGWithSSG(TextSemGraph *tsg, SceneSemGraph *databaseSSG, double &matchingScore)
{
	std::map<int, int> mapFromTsgToDBSsgNodeId;
	SceneSemGraph *matchedSubSSG;

	//	
	for (int i = 0; i < databaseSSG->m_nodeNum; i++)
	{
		databaseSSG->m_nodes[i].isMatched = false;
	}

	// first match object node
	for (int tni = 0; tni < tsg->m_nodeNum; tni++)
	{
		SemNode& tsgNode = tsg->m_nodes[tni];

		if (tsgNode.nodeType == "object")
		{
			for (int dni = 0; dni < databaseSSG->m_nodeNum; dni++)
			{
				SemNode& dbSgNode = databaseSSG->m_nodes[dni];
				// skip the aligned nodes
				if (!dbSgNode.isMatched && dbSgNode.nodeType == "object")
				{
					if (tsgNode.nodeName == dbSgNode.nodeName)
					{
						tsgNode.isMatched = true;
						dbSgNode.isMatched = true;
						mapFromTsgToDBSsgNodeId[tni] = dni; // save aligned object node map
						matchingScore += 1;
						break;
					}
				}
			}
		}
	}

	if (matchingScore == 0)
	{
		return NULL; 
	}

	// align per-object attribute node


	// align pair-wise relationship node
	for (int tni = 0; tni < tsg->m_nodeNum; tni++)
	{
		SemNode& tsgNode = tsg->m_nodes[tni];

		if (tsgNode.nodeType == "pairwise_relationship")
		{
			// To test whether in and out node exist
			if (tsgNode.inEdgeNodeList.empty() || tsgNode.outEdgeNodeList.empty())
			{
				break;
			}
			 
			int tInNodeId = tsgNode.inEdgeNodeList[0];
			int tOutNodeId = tsgNode.outEdgeNodeList[0];

			// if any object node is not in the matched map (not matched), then break
			if (!mapFromTsgToDBSsgNodeId.count(tInNodeId) || !mapFromTsgToDBSsgNodeId.count(tOutNodeId))
			{
				break;
			}

			for (int dni = 0; dni < databaseSSG->m_nodeNum; dni++)
			{
				SemNode& dbSgNode = databaseSSG->m_nodes[dni];
				// skip the aligned nodes
				if (!dbSgNode.isMatched && dbSgNode.nodeType == "pairwise_relationship")
				{
					if (dbSgNode.inEdgeNodeList[0] == mapFromTsgToDBSsgNodeId[tInNodeId]
						&& dbSgNode.outEdgeNodeList[0] == mapFromTsgToDBSsgNodeId[tOutNodeId])
					{
						tsgNode.isMatched = true;
						dbSgNode.isMatched = true;
						mapFromTsgToDBSsgNodeId[tni] = dni;  // save aligned pairwise relationship node map

						matchingScore += 1;
						break;
					}
				}
			}
		}
	}

	// collect matched nodes and generate subgraph
	std::vector<int> matchedDBSsgNodeList;
	for (int tni = 0; tni < tsg->m_nodeNum; tni++)
	{
		SemNode& tsgNode = tsg->m_nodes[tni];
		
		if (tsgNode.isMatched && mapFromTsgToDBSsgNodeId.count(tni))
		{
			int matchedDBSsgId = mapFromTsgToDBSsgNodeId[tni];
			matchedDBSsgNodeList.push_back(matchedDBSsgId);
		}
	}



	matchedSubSSG = databaseSSG->getSubGraph(matchedDBSsgNodeList);

	return matchedSubSSG;
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


