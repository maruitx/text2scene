#include "SemGraphMatcher.h"
#include "SceneSemGraphManager.h"
#include "SceneSemGraph.h"
#include "TextSemGraph.h"
#include "GMTMatcher.h"


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

vector<SceneSemGraph*> SemGraphMatcher::matchTSGWithSSGs(int topMatchNum)
{
	cout << "SemGraphMatcher: start graph matching.\n";

	Graph *textGraph = convertToGMTGraph(m_currTextSemGraph);

	double match_threshold = -1;
	/* recognition(Graph*, error threshold=-1 (no limit) =0 (exact match only)  */
	/*                                    >0 (find first inexact match)         */
	m_gmtMatcher->m_graphDatabase->recognition(textGraph, match_threshold);

	int matchedSSGNum = m_gmtMatcher->m_graphDatabase->NumberOfMatches();

	vector<pair<double, InstanceData *>> matchedInstances;

	// collected matched instance ids
	std::set<int> matchedSSGIds;
	for (int i = 0; i < matchedSSGNum; i++)
	{
		InstanceData* IS = m_gmtMatcher->m_graphDatabase->getInstance(i);
		int currId = IS->modelNr;

		// only return non-repeated instance
		if (std::find(matchedSSGIds.begin(), matchedSSGIds.end(), currId) == matchedSSGIds.end())
		{
			matchedInstances.push_back(make_pair(IS->totalError, IS));
		}

		matchedSSGIds.insert(currId);
	}

	sort(matchedInstances.begin(), matchedInstances.end()); // ascending order

	matchedSSGNum = matchedInstances.size();
	vector<SceneSemGraph*> matchedSubSSGs;

	int ssgNum = min(topMatchNum, matchedSSGNum);

	for (int i = 0; i < ssgNum; i++)
	{
		SceneSemGraph *ssg = convertGMTInstanceToSSG(matchedInstances[i].second);

		if (ssg!=NULL)
		{
			matchedSubSSGs.push_back(ssg);
		}
	}


	cout << "SemGraphMatcher: graph matching done, found instance number " << matchedSSGNum << ", shown instance number " << matchedSubSSGs.size() << ".\n";

	return matchedSubSSGs;
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

vector<SceneSemGraph*> SemGraphMatcher::testMatchTSGWithSSGs(int topMacthNum)
{
	//m_gmtMatcher->test();

	vector<pair<double, SceneSemGraph *>> evaluatedSSGs;

	for (int i = 0; i < m_sceneSemGraphManager->m_ssgNum; i++)
	{
		SceneSemGraph *currSSG = m_sceneSemGraphManager->getGraph(i);

		//double simVal = rand();
		double simVal = computeSimilarity(m_currTextSemGraph, currSSG);
		evaluatedSSGs.push_back(make_pair(simVal, currSSG));
	}

	sort(evaluatedSSGs.begin(), evaluatedSSGs.end());  // ascending order
	reverse(evaluatedSSGs.begin(), evaluatedSSGs.end()); // descending order

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

Graph* SemGraphMatcher::convertToGMTGraph(SemanticGraph *sg)
{
	Graph *gmtGraph = new Graph;

	double attrVal[1];  // use node attribute to store node type; later we can weight on diff node types
	
	for (int i = 0; i < sg->m_nodeNum; i++)
	{
		int labelID = m_sceneSemGraphManager->getNodeLabelID(sg->m_nodes[i].nodeName);
		int attriID = m_sceneSemGraphManager->getNodeTypeID(sg->m_nodes[i].nodeName);

		attrVal[0] = attriID;
		/* set(id,Label,attribute array, dimension of array) */
		gmtGraph->set(i, labelID, attrVal, 1);
	}

	for (int i = 0; i < sg->m_edgeNum; i++)
	{
		attrVal[0] = 0;
		/* setEdge(out-node(source),in-node(target),number,label,attributes-array,dim of array) */
		gmtGraph->setEdge(sg->m_edges[i].sourceNodeId, sg->m_edges[i].targetNodeId, i, 0, attrVal, 1); // // set edge label as 0, and the attribute label as 0
	}

	// Debug: add a virtual node and edges to make sure the graph is one component
	// To-fix: should not be needed when horizon and vert support hierarchy is built
	int vNum = gmtGraph->xVertices;
	int eNum = gmtGraph->xEdges;
	gmtGraph->set(vNum, 277, attrVal, 1);  // set label as room
	for (int i = 0; i < vNum; i++)
	{
		gmtGraph->setEdge(vNum, i, i+eNum, 0, attrVal, 1); // // set edge label as 0, and the attribute label as 0
	}

	gmtGraph->setName("G0");
	/* mark end of graph definition (1) directed, (0) undirected */
	gmtGraph->done(1);

	return gmtGraph;
}

SceneSemGraph* SemGraphMatcher::convertGMTInstanceToSSG(InstanceData *gmtInstance)
{
	int matchedSSGId = gmtInstance->modelNr;
	SceneSemGraph *matchedFullSSG = m_sceneSemGraphManager->getGraph(matchedSSGId);

	int nodeNum = gmtInstance->dim;
	vector<int> nodeList;

	// collect node list
	for (int i = 0; i < nodeNum; i++)
	{		
		int matchedNodeId = gmtInstance->model[i];

		// we add a virtual node in GMT graph, do not add the virtual node which we added to make the graph as one component		
		if (matchedNodeId < matchedFullSSG->m_nodeNum)
		{
			nodeList.push_back(matchedNodeId);
		}			
	}

	std::sort(nodeList.begin(), nodeList.end());
	SceneSemGraph *matchedSubSSG = matchedFullSSG->getSubGraph(nodeList);

	return matchedSubSSG;
}

