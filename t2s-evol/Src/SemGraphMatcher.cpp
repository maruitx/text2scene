#include "SemGraphMatcher.h"
#include "SceneSemGraphManager.h"
#include "SceneSemGraph.h"
#include "TextSemGraph.h"

const bool useContext = true;
const bool addSynthesizeNode = false;


SemGraphMatcher::SemGraphMatcher(SceneSemGraphManager *ssgManager)
	:m_sceneSemGraphManager(ssgManager)
{
	cout << "\nSemGraphMatcher: loading semantic graphs number " << m_sceneSemGraphManager->m_ssgNum<<"\n";
}

SemGraphMatcher::~SemGraphMatcher()
{

}

void SemGraphMatcher::updateQuerySG(SemanticGraph *sg)
{
	m_querySSG = sg;
}

vector<SceneSemGraph*> SemGraphMatcher::alignWithDatabaseSSGs(int topMatchNum)
{
	cout << "SemGraphMatcher: start graph matching.\n";

	vector<pair<double, SceneSemGraph *>> scoredDBSubSSGs;
	double exactMatchScore = m_querySSG->m_nodes.size();
	int exactMatchNum = 0;

	for (int i = 0; i < m_sceneSemGraphManager->m_ssgNum; i++)
	{
		SceneSemGraph *currDBSSG = m_sceneSemGraphManager->getGraph(i);
		currDBSSG->setNodesUnAligned();

		double matchingScore = 0;
		SceneSemGraph *subSSG = alignSSGWithDBSSG(m_querySSG, currDBSSG, matchingScore);

		if (matchingScore > 0)
		{
			scoredDBSubSSGs.push_back(make_pair(matchingScore, subSSG));
			if (matchingScore == exactMatchScore)
			{
				exactMatchNum++;
			}
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
			ssg->m_matchListId = i;
			matchedSubSSGs.push_back(ssg);

			int ssgId = 0;
			for (int j = 0; j < ssg->m_nodeNum; j++)
			{
				SemNode &currNode = ssg->m_nodes[j];
				if (currNode.nodeType == "object" && !currNode.isAligned)
				{
					qDebug() << QString("SemGraphMatcher: in Preview %1 entity - %2 is not matched").arg(ssgId).arg(currNode.nodeName);
				}

				if (currNode.nodeType == "pairwise_relationship" && !currNode.isAligned)
				{
					int inNodeId = currNode.inEdgeNodeList[0];
					int outNodeId = currNode.outEdgeNodeList[0];

					qDebug() << QString("SemGraphMatcher: in Preview %1 cannot match for relationship: %2 %3: %4").arg(ssgId).arg(ssg->m_nodes[inNodeId].nodeName).arg(currNode.nodeName).arg(ssg->m_nodes[outNodeId].nodeName);

					if (currNode.isSynthesized)
					{
						qDebug() << QString("   synthesize node %1 for %2:%3").arg(currNode.nodeName).arg(ssg->m_nodes[inNodeId].nodeName).arg(ssg->m_nodes[outNodeId].nodeName);
					}
				}

				if ((currNode.nodeType == "per_obj_attribute" || currNode.nodeType=="group_attribute") && !currNode.isAligned)
				{
					int outNodeId = currNode.outEdgeNodeList[0];

					qDebug() << QString("SemGraphMatcher: in Preview %1 cannot match for attribute %2:%3").arg(ssgId).arg(ssg->m_nodes[outNodeId].nodeName).arg(currNode.nodeName);
					
					if (currNode.isSynthesized)
					{
						qDebug() << QString("   synthesize node %1 for %2").arg(currNode.nodeName).arg(ssg->m_nodes[outNodeId].nodeName);
					}
				}				
			}
			ssgId++;
		}
	}

	cout << "SemGraphMatcher: graph matching done, found instance num " << matchedSSGNum << 
		", exact match num " << exactMatchNum
		<< ", shown instance num " << matchedSubSSGs.size() << ".\n";
	return matchedSubSSGs;
}

SceneSemGraph* SemGraphMatcher::alignSSGWithDBSSG(SemanticGraph *querySSG, SceneSemGraph *dbSSG, double &matchingScore)
{
	SceneSemGraph *matchedSubSSG;

	// align object nodes
	querySSG->m_toNewSgNodeIdMap.clear();
	querySSG->alignObjectNodesWithGraph(dbSSG, matchingScore);

	if (matchingScore == 0) return NULL;

	// align relationship nodes
	querySSG->alignRelationNodesWithGraph(dbSSG, matchingScore);

	// collect matched nodes and generate subgraph
	std::vector<int> matchedDBSsgNodeList;
	for (int ni = 0; ni < querySSG->m_nodeNum; ni++)
	{
		SemNode& sgNode = querySSG->m_nodes[ni];
		
		if (sgNode.isAligned && querySSG->m_toNewSgNodeIdMap.count(ni))
		{
			int matchedDBSsgId = querySSG->m_toNewSgNodeIdMap[ni];
			matchedDBSsgNodeList.push_back(matchedDBSsgId);
		}
	}

	// add nodes by exact match and scene context
	matchedSubSSG = dbSSG->getSubGraph(matchedDBSsgNodeList, useContext);

	if (addSynthesizeNode)
	{
		addSynthNodeToSubSSG(querySSG, matchedSubSSG);
	}

	return matchedSubSSG;
}

double SemGraphMatcher::computeSimilarity(SemanticGraph *tsg, SemanticGraph *ssg)
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

void SemGraphMatcher::addSynthNodeToSubSSG(SemanticGraph *querySSG, SceneSemGraph *matchedSubSSG)
{
	std::map<int, int> queryToSubSsgNodeMap;

	//collect matched nodes into the map
	for (int ni = 0; ni < querySSG->m_nodeNum; ni++)
	{
		SemNode& sgNode = querySSG->m_nodes[ni];

		if (sgNode.isAligned && querySSG->m_toNewSgNodeIdMap.count(ni))
		{
			// first find the DB-SSG node corresponding to the query node
			int dbNodeId = querySSG->m_toNewSgNodeIdMap[ni];

			// then find the Sub-SSG node for the query node
			queryToSubSsgNodeMap[ni] = matchedSubSSG->m_dbNodeToSubNodeMap[dbNodeId];
		}
	}

	// insert synthesized object node
	for (int ni = 0; ni < querySSG->m_nodeNum; ni++)
	{
		SemNode& sgNode = querySSG->m_nodes[ni];

		if (!sgNode.isAligned && !querySSG->m_toNewSgNodeIdMap.count(ni))
		{
			if (sgNode.nodeType == "object")
			{
				matchedSubSSG->addNode(sgNode);
				int currNodeId = matchedSubSSG->m_nodeNum - 1;
				matchedSubSSG->m_nodes[currNodeId].isSynthesized = 1;
				queryToSubSsgNodeMap[ni] = currNodeId;

				// set meta data
				//matchedSubSSG->m_objectGraphNodeIdToModelSceneIdMap[matchedSubSSG->m_nodeNum - 1] = ;

			}
		}
	}

	// insert synthesized attribute and relation node
	for (int ni = 0; ni < querySSG->m_nodeNum; ni++)
	{
		SemNode& sgNode = querySSG->m_nodes[ni];

		if (!sgNode.isAligned && (sgNode.nodeType == "per_obj_attribute" || sgNode.nodeType == "group_attribute"))
		{
			if (!sgNode.outEdgeNodeList.empty())
			{
				int refNodeId = sgNode.outEdgeNodeList[0];

				// only insert the relationship node for the object that is matched
				if (sgNode.isAligned&&queryToSubSsgNodeMap.count(refNodeId))
				{
					// insert node
					matchedSubSSG->addNode(sgNode);
					int currNodeId = matchedSubSSG->m_nodeNum - 1;
					matchedSubSSG->m_nodes[currNodeId].isSynthesized = 1;

					matchedSubSSG->addEdge(matchedSubSSG->m_nodeNum - 1, queryToSubSsgNodeMap[refNodeId]);
					queryToSubSsgNodeMap[ni] = matchedSubSSG->m_nodeNum - 1;
				}
			}
		}

		if (!sgNode.isAligned && (sgNode.nodeType == "pairwise_relationship"))
		{
			if (!sgNode.outEdgeNodeList.empty())
			{
				int refNodeId = sgNode.outEdgeNodeList[0];
				int actNodeId = sgNode.inEdgeNodeList[0];

				if (queryToSubSsgNodeMap.count(refNodeId) && queryToSubSsgNodeMap.count(actNodeId))
				{
					// insert node
					matchedSubSSG->addNode(sgNode);
					int currNodeId = matchedSubSSG->m_nodeNum - 1;
					matchedSubSSG->m_nodes[currNodeId].isSynthesized = 1;

					matchedSubSSG->addEdge(matchedSubSSG->m_nodeNum - 1, queryToSubSsgNodeMap[refNodeId]);
					matchedSubSSG->addEdge(queryToSubSsgNodeMap[actNodeId], matchedSubSSG->m_nodeNum - 1);

					queryToSubSsgNodeMap[ni] = matchedSubSSG->m_nodeNum - 1;
				}
			}
		}
	}
}





