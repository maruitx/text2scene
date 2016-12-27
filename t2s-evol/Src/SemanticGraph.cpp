#include "SemanticGraph.h"
#include "Utility.h"

SemanticGraph::SemanticGraph()
	:m_nodeNum(0), m_edgeNum(0)
{
}


SemanticGraph::~SemanticGraph()
{
}

void SemanticGraph::addNode(const QString &nType, const QString &nName)
{
	SemNode newNode = SemNode(nType, nName, m_nodeNum);
	m_nodes.push_back(newNode);

	

	m_nodeNum++;
}

void SemanticGraph::addEdge(int s, int t)
{
	SemEdge newEdge = SemEdge(s, t, m_edgeNum);

	m_edges.push_back(newEdge);

	// update node edge list
	m_nodes[s].outEdgeNodeList.push_back(t);
	m_nodes[t].inEdgeNodeList.push_back(s);

	m_edgeNum++;
}
