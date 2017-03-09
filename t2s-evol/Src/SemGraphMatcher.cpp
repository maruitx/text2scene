#include "SemGraphMatcher.h"
#include "SceneSemGraphManager.h"
#include "SceneSemGraph.h"
#include "TextSemGraph.h"

const bool useContext = true;
const bool addSynthesizeNode = false;


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

void SemGraphMatcher::updateQuerySSG(SemanticGraph *sg)
{
	m_querySSG = sg;
}

vector<SceneSemGraph*> SemGraphMatcher::alignmentSSGWithDatabaseSSGs(int topMatchNum)
{
	cout << "SemGraphMatcher: start graph matching.\n";

	vector<pair<double, SceneSemGraph *>> scoredDBSubSSGs;
	double exactMatchScore = m_querySSG->m_nodes.size();
	int exactMatchNum = 0;

	for (int i = 0; i < m_sceneSemGraphManager->m_ssgNum; i++)
	{
		SceneSemGraph *currDBSSG = m_sceneSemGraphManager->getGraph(i);

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
			matchedSubSSGs.push_back(ssg);

			int ssgId = 0;
			for (int j = 0; j < ssg->m_nodeNum; j++)
			{
				SemNode &currNode = ssg->m_nodes[j];
				if (currNode.nodeType == "object" && !currNode.isMatched)
				{
					qDebug() << QString("SemGraphMatcher: in Preview %1 entity - %2 is not matched").arg(ssgId).arg(currNode.nodeName);
				}

				if (currNode.nodeType == "pairwise_relationship" && !currNode.isMatched)
				{
					int inNodeId = currNode.inEdgeNodeList[0];
					int outNodeId = currNode.outEdgeNodeList[0];

					qDebug() << QString("SemGraphMatcher: in Preview %1 cannot match for relationship: %2 %3: %4").arg(ssgId).arg(ssg->m_nodes[inNodeId].nodeName).arg(currNode.nodeName).arg(ssg->m_nodes[outNodeId].nodeName);

					if (currNode.isSynthesized)
					{
						qDebug() << QString("   synthesize node %1 for %2:%3").arg(currNode.nodeName).arg(ssg->m_nodes[inNodeId].nodeName).arg(ssg->m_nodes[outNodeId].nodeName);
					}
				}

				if ((currNode.nodeType == "per_obj_attribute" || currNode.nodeType=="group_attribute") && !currNode.isMatched)
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

	m_queryToDBSsgNodeIdMap.clear();

	//	init
	for (int i = 0; i < dbSSG->m_nodeNum; i++)
	{
		dbSSG->m_nodes[i].isMatched = false;
	}

	// align object nodes
	alignObjectNodes(querySSG, dbSSG, matchingScore);

	if (matchingScore == 0)
	{
		return NULL; 
	}

	// align relationship nodes
	alignRelationshipNodes(querySSG, dbSSG, matchingScore);

	// collect matched nodes and generate subgraph
	std::vector<int> matchedDBSsgNodeList;
	for (int ni = 0; ni < querySSG->m_nodeNum; ni++)
	{
		SemNode& sgNode = querySSG->m_nodes[ni];
		
		if (sgNode.isMatched && m_queryToDBSsgNodeIdMap.count(ni))
		{
			int matchedDBSsgId = m_queryToDBSsgNodeIdMap[ni];
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

void SemGraphMatcher::alignObjectNodes(SemanticGraph *querySSG, SceneSemGraph *dbSSG, double &matchingScore)
{
	// first match object node and per-object attribute node
	for (int tni = 0; tni < querySSG->m_nodeNum; tni++)
	{
		SemNode& sgNode = querySSG->m_nodes[tni];

		if (sgNode.nodeType == "object")
		{
			for (int dni = 0; dni < dbSSG->m_nodeNum; dni++)
			{
				SemNode& dbSgNode = dbSSG->m_nodes[dni];
				// skip the aligned nodes
				if (!dbSgNode.isMatched && dbSgNode.nodeType == "object" && sgNode.nodeName == dbSgNode.nodeName)
				{
					if (!sgNode.inEdgeNodeList.empty())
					{

						int matchedAttNum = 0;
						int tsgAttNum = 0;

						// align attribute node 
						for (int tai = 0; tai < sgNode.inEdgeNodeList.size(); tai++)
						{
							int taNodeId = sgNode.inEdgeNodeList[tai];  // id of attribute node in tsg
							SemNode &taNode = querySSG->m_nodes[taNodeId];

							if (taNode.nodeType == "per_obj_attribute")
							{
								tsgAttNum++;
								if (!dbSgNode.inEdgeNodeList.empty())
								{
									for (int dai = 0; dai < dbSgNode.inEdgeNodeList.size(); dai++)
									{
										int daNodeId = dbSgNode.inEdgeNodeList[dai]; // id of attribute node in dbssg
										SemNode &daNode = dbSSG->m_nodes[daNodeId];

										if (taNode.nodeType == daNode.nodeType && taNode.nodeName == daNode.nodeName)
										{
											matchedAttNum++;

											taNode.isMatched = true;
											daNode.isMatched = true;
											m_queryToDBSsgNodeIdMap[taNodeId] = daNodeId; // save aligned attribute node into map		
											matchingScore += 0;  // attribute node does not contribute to matching score
										}
									}
								}
							}
						}

						//  if all attribute nodes matched, then the node is matched
						if (matchedAttNum == tsgAttNum)
						{
							sgNode.isMatched = true;
							dbSgNode.isMatched = true;
							m_queryToDBSsgNodeIdMap[tni] = dni; // save aligned object node into map									
							matchingScore += 1;
							break;
						}
						else
						{
							sgNode.isMatched = true;
							dbSgNode.isMatched = true;
							m_queryToDBSsgNodeIdMap[tni] = dni; // save partial aligned object node into map									
							matchingScore += 0.5;
							break;
						}
					}
					else
					{
						sgNode.isMatched = true;
						dbSgNode.isMatched = true;
						m_queryToDBSsgNodeIdMap[tni] = dni; // save aligned object node map
						matchingScore += 1;
						break;
					}
				}
			}
		}
	}
}

void SemGraphMatcher::alignRelationshipNodes(SemanticGraph *querySSG, SceneSemGraph *dbSSG, double &matchingScore)
{
	for (int tni = 0; tni < querySSG->m_nodeNum; tni++)
	{
		SemNode& sgNode = querySSG->m_nodes[tni];

		// align pair-wise relationship node
		if (sgNode.nodeType == "pairwise_relationship")
		{
			// To test whether in and out node exist
			if (sgNode.inEdgeNodeList.empty() || sgNode.outEdgeNodeList.empty())
			{
				break;
			}

			int tInNodeId = sgNode.inEdgeNodeList[0];
			int tOutNodeId = sgNode.outEdgeNodeList[0];

			// if any object node is not in the matched map (not matched), then break
			if (!m_queryToDBSsgNodeIdMap.count(tInNodeId) || !m_queryToDBSsgNodeIdMap.count(tOutNodeId))
			{
				break;
			}

			for (int dni = 0; dni < dbSSG->m_nodeNum; dni++)
			{
				SemNode& dbSgNode = dbSSG->m_nodes[dni];
				// skip the aligned nodes
				if (!dbSgNode.isMatched && dbSgNode.nodeType == "pairwise_relationship" && sgNode.nodeName == dbSgNode.nodeName)
				{
					if (dbSgNode.inEdgeNodeList[0] == m_queryToDBSsgNodeIdMap[tInNodeId]
						&& dbSgNode.outEdgeNodeList[0] == m_queryToDBSsgNodeIdMap[tOutNodeId])
					{
						sgNode.isMatched = true;
						dbSgNode.isMatched = true;
						m_queryToDBSsgNodeIdMap[tni] = dni;  // save aligned pairwise relationship node into map

						matchingScore += 1;
						break;
					}
				}
			}
		}

		if (sgNode.nodeType == "group_attribute")
		{
			if (sgNode.outEdgeNodeList.empty())
			{
				break;
			}

			int refNodeIdInTsg = sgNode.outEdgeNodeList[0];
			if (!m_queryToDBSsgNodeIdMap.count(refNodeIdInTsg))
			{
				break;
			}

			SemNode &tsgRefNode = querySSG->m_nodes[refNodeIdInTsg];

			for (int dni = 0; dni < dbSSG->m_nodeNum; dni++)
			{
				SemNode& dbSgNode = dbSSG->m_nodes[dni];
				// skip the aligned nodes
				if (!dbSgNode.isMatched && dbSgNode.nodeType == "group_attribute" && sgNode.nodeName == dbSgNode.nodeName)
				{
					int dbRefId = dbSgNode.outEdgeNodeList[0];
					SemNode& dbRefNode = dbSSG->m_nodes[dbRefId];
					if (dbRefNode.nodeName == tsgRefNode.nodeName)
					{
						sgNode.isMatched = true;
						dbSgNode.isMatched = true;
						m_queryToDBSsgNodeIdMap[tni] = dni;  // save aligned pairwise relationship node map

						matchingScore += 1;
						break;
					}
				}
			}
		}
	}
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

		if (sgNode.isMatched && m_queryToDBSsgNodeIdMap.count(ni))
		{
			// first find the DB-SSG node corresponding to the query node
			int dbNodeId = m_queryToDBSsgNodeIdMap[ni];

			// then find the Sub-SSG node for the query node
			queryToSubSsgNodeMap[ni] = matchedSubSSG->m_dbNodeToSubNodeMap[dbNodeId];
		}
	}

	// insert synthesized object node
	for (int ni = 0; ni < querySSG->m_nodeNum; ni++)
	{
		SemNode& sgNode = querySSG->m_nodes[ni];

		if (!sgNode.isMatched && !m_queryToDBSsgNodeIdMap.count(ni))
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

		if (!sgNode.isMatched && (sgNode.nodeType == "per_obj_attribute" || sgNode.nodeType == "group_attribute"))
		{
			if (!sgNode.outEdgeNodeList.empty())
			{
				int refNodeId = sgNode.outEdgeNodeList[0];

				// only insert the relationship node for the object that is matched
				if (sgNode.isMatched&&queryToSubSsgNodeMap.count(refNodeId))
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

		if (!sgNode.isMatched && (sgNode.nodeType == "pairwise_relationship"))
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





