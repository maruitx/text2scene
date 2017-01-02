#include "SemGraphMatcher.h"
#include "SceneSemGraphManager.h"
#include "SceneSemGraph.h"
#include "TextSemGraph.h"
#include "GMTMatcher.h"


SemGraphMatcher::SemGraphMatcher(SceneSemGraphManager *ssgManager)
	:m_sceneSemGraphManager(ssgManager)
{
	m_gmtMatcher = new GMTMatcher();

	cout << "SemGraphMatcher: start converting ssg to gmt graph...\n";

	// add scene ssg to GTM graph database
	for (int i = 0; i < m_sceneSemGraphManager->m_ssgNum; i++)
	{
		//Graph *G1 = new Graph;
		//double values[1];

		//values[0] = 2;
		//G1->set(0, 1, values, 1);
		//values[0] = 3;
		//G1->set(1, 3, values, 1);
		//values[0] = 1;
		//G1->set(2, 1, values, 1);

		///* set edges */

		///* setEdge(out-node,in-node,number,label,attributes-array,dim of array) */
		//G1->setEdge(0, 1, 1, 0, values, 1);
		//G1->setEdge(1, 2, 2, 0, values, 1);

		//G1->setName("G1");
		///* mark end of graph G1 definition (1) directed, (0) undirected */
		//G1->done(1);
		//m_gmtMatcher->m_graphDatabase->addModel(G1, 0);

		SemanticGraph *sg = m_sceneSemGraphManager->getGraph(i);
		Graph *gmtGraph = convertToGMTGraph(sg);
		m_gmtMatcher->m_graphDatabase->addModel(gmtGraph, 0);

		cout << "\t finish converting " << i +1 << "/" << m_sceneSemGraphManager->m_ssgNum<<" graphs\r";
	}

	cout << "\nSemGraphMatcher: loading semantic graphs number " << m_sceneSemGraphManager->m_ssgNum<<"\n";
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
	cout << "SemGraphMatcher: start graph matching.\n";

	Graph *textGraph = convertToGMTGraph(m_currTextSemGraph);

	double match_threshold = -1;
	/* recognition(Graph*, error threshold=-1 (no limit) =0 (exact match only)  */
	/*                                    >0 (find first inexact match)         */
	m_gmtMatcher->m_graphDatabase->recognition(textGraph, match_threshold);

	int matchedSSGNum = m_gmtMatcher->m_graphDatabase->NumberOfMatches();

	vector<pair<double, InstanceData *>> matchedInstances;

	for (int i = 0; i < matchedSSGNum; i++)
	{
		InstanceData* IS = m_gmtMatcher->m_graphDatabase->getInstance(i);

		matchedInstances.push_back(make_pair(IS->totalError, IS));
	}

	sort(matchedInstances.begin(), matchedInstances.end()); // ascending order

	vector<SceneSemGraph*> matchedSubSSGs;

	int ssgNum = min(topMacthNum, matchedSSGNum);

	for (int i = 0; i < ssgNum; i++)
	{
		SceneSemGraph *ssg = convertGMTInstanceToSSG(matchedInstances[i].second);
		matchedSubSSGs.push_back(ssg);
	}


	cout << "SemGraphMatcher: graph matching done, found instance number " << matchedSSGNum << ", shown instance number " << ssgNum << ".\n";

	return matchedSubSSGs;
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
		// do not add the virtual node which we added to make the graph as one component
		if (matchedNodeId < matchedFullSSG->m_nodeNum)
		{
			nodeList.push_back(matchedNodeId);
		}			
	}

	SceneSemGraph *matchedSubSSG = matchedFullSSG->getSubGraph(nodeList);

	return matchedSubSSG;
}
