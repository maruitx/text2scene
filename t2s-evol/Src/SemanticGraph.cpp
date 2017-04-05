#include "SemanticGraph.h"
#include "Utility.h"

SemanticGraph::SemanticGraph()
	:m_nodeNum(0), m_edgeNum(0)
{
}

SemanticGraph::SemanticGraph(SemanticGraph *sg)
{
	m_nodeNum = sg->m_nodeNum;
	m_edgeNum = sg->m_edgeNum;

	m_nodes = sg->m_nodes;
	m_edges = sg->m_edges;

	m_toNewSgNodeIdMap = sg->m_toNewSgNodeIdMap;
}


SemanticGraph::~SemanticGraph()
{
}

void SemanticGraph::addNode(const QString &nType, const QString &nName)
{
	QString uniType = nType;

	if (nType == "pairwise_relationship")
	{
		uniType = QString("relation");
	}

	if (nType == "per_obj_attribute")
	{
		uniType = QString("attribute");
	}

	SemNode newNode = SemNode(uniType, nName, m_nodeNum);
	m_nodes.push_back(newNode);

	m_nodeNum++;
}

void SemanticGraph::addNode(const SemNode &node)
{
	SemNode newNode = SemNode(node.nodeType, node.nodeName, m_nodeNum);
	newNode.isAligned = node.isAligned;
	newNode.matchingStatus = node.matchingStatus;

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

void SemanticGraph::parseNodeNeighbors()
{
	for (int i = 0; i < m_nodeNum; i++)
	{
		SemNode& sgNode = m_nodes[i];

		if (sgNode.nodeType == "object")
		{
			sgNode.nodeLabels.clear();

			for (int ai = 0; ai < sgNode.inEdgeNodeList.size(); ai++)
			{
				int attNodeId = sgNode.inEdgeNodeList[ai];  // id of attribute node in tsg
				SemNode &attNode = this->m_nodes[attNodeId];

				if (attNode.nodeType == "attribute")
				{
					sgNode.nodeLabels.push_back(attNodeId);
				}
			}
		}

		if (sgNode.nodeType == "relation")
		{
			sgNode.activeNodeList.clear();
			sgNode.anchorNodeList.clear();

			// edge dir: (active, relation), (relation, reference)

			for (int ai = 0; ai < sgNode.inEdgeNodeList.size(); ai++)
			{
				int inNodeId = sgNode.inEdgeNodeList[ai];
				sgNode.activeNodeList.push_back(inNodeId);
			}

			for (int ai = 0; ai < sgNode.outEdgeNodeList.size(); ai++)
			{
				int outNodeId = sgNode.outEdgeNodeList[ai];
				sgNode.anchorNodeList.push_back(outNodeId);
			}
		}
	}
}

std::vector<int> SemanticGraph::findNodeWithName(const QString &nName)
{
	std::vector<int> nodeIds;

	for (int i = 0; i < m_nodeNum; i++)
	{
		if (m_nodes[i].nodeName == nName)
		{
			nodeIds.push_back(i);
		}
	}

	return nodeIds;
}

bool SemanticGraph::isEdgeExist(int s, int t)
{
	for (int i = 0; i < m_edgeNum; i++)
	{
		if (m_edges[i].sourceNodeId == s && m_edges[i].targetNodeId == t)
		{
			return true;
		}
	}

	return false;
}

void SemanticGraph::alignObjectNodesWithGraph(SemanticGraph *targetGraph, double &alignScore)
{
	double NodeScore[] = { 0, 1, 10};

	// first match object node and per-object attribute node
	for (int qNi = 0; qNi < this->m_nodeNum; qNi++)
	{
		SemNode& sgNode = this->m_nodes[qNi];

		if (sgNode.nodeType == "object")
		{
			for (int tarNi = 0; tarNi < targetGraph->m_nodeNum; tarNi++)
			{
				SemNode& tarSgNode = targetGraph->m_nodes[tarNi];
				// skip the aligned nodes
				if (!tarSgNode.isAligned && tarSgNode.nodeType == "object" && sgNode.nodeName == tarSgNode.nodeName)
				{
					if (!sgNode.nodeLabels.empty())
					{
						int matchedAttNum = 0;
						int sgAttNum = 0;

						// align attribute node
						for (int ai = 0; ai < sgNode.nodeLabels.size(); ai++)
						{
							int attNodeId = sgNode.nodeLabels[ai];

							SemNode &attNode = this->m_nodes[attNodeId];
							sgAttNum++;

							if (!tarSgNode.nodeLabels.empty())
							{
								for (int tarAi = 0; tarAi < tarSgNode.nodeLabels.size(); tarAi++)
								{
									int tarAttNodeId = tarSgNode.nodeLabels[tarAi]; // id of attribute node in dbssg
									SemNode &tarAttNode = targetGraph->m_nodes[tarAttNodeId];

									if (attNode.nodeType == tarAttNode.nodeType && attNode.nodeName == tarAttNode.nodeName)
									{
										matchedAttNum++;

										attNode.isAligned = true;
										tarAttNode.isAligned = true;
										tarAttNode.matchingStatus = attNode.matchingStatus;

										m_toNewSgNodeIdMap[attNodeId] = tarAttNodeId; // save aligned attribute node into map		
										alignScore += 0;  // attribute node does not contribute to matching score
									}
								}
							}
						}

						//  if all attribute nodes matched, then the node is matched
						if (matchedAttNum == sgAttNum)
						{
							sgNode.isAligned = true;
							tarSgNode.isAligned = true;
							tarSgNode.matchingStatus = sgNode.matchingStatus;

							m_toNewSgNodeIdMap[qNi] = tarNi; // save aligned object node into map									
							alignScore += NodeScore[sgNode.matchingStatus];
							break;
						}
						else
						{
							sgNode.isAligned = true;
							tarSgNode.isAligned = true;
							tarSgNode.matchingStatus = sgNode.matchingStatus;

							m_toNewSgNodeIdMap[qNi] = tarNi; // save partial aligned object node into map									
							alignScore += 0.5*NodeScore[sgNode.matchingStatus];
							break;
						}
					}
					else
					{
						sgNode.isAligned = true;
						tarSgNode.isAligned = true;
						tarSgNode.matchingStatus = sgNode.matchingStatus;

						m_toNewSgNodeIdMap[qNi] = tarNi; // save aligned object node map
						alignScore += NodeScore[sgNode.matchingStatus];
						break;
					}
				}
			}
		}
	}
}

void SemanticGraph::alignRelationNodesWithGraph(SemanticGraph *targetGraph, double &alignScore)
{
	double NodeScore[] = { 0, 1, 10 };

	for (int qNi = 0; qNi < this->m_nodeNum; qNi++)
	{
		SemNode& sgNode = this->m_nodes[qNi];

		// align pair-wise relationship node
		if (sgNode.nodeType == "relation")
		{
			// To test whether in and out node exist
			if (sgNode.activeNodeList.empty() || sgNode.anchorNodeList.empty()) continue;

			// edge dir: (active, relation), (relation, reference)
			int actNodeId = sgNode.activeNodeList[0];
			int anchorNodeId = sgNode.anchorNodeList[0];

			// if any object node is not in the matched map (not matched), then break
			if (!m_toNewSgNodeIdMap.count(actNodeId) || !m_toNewSgNodeIdMap.count(anchorNodeId)) continue;

			SemNode& queryActiveNode = m_nodes[actNodeId];

			for (int tarNi = 0; tarNi < targetGraph->m_nodeNum; tarNi++)
			{
				SemNode& tarSgNode = targetGraph->m_nodes[tarNi];
				// skip the aligned nodes
				if (!tarSgNode.isAligned && tarSgNode.nodeType == "relation" && sgNode.nodeName == tarSgNode.nodeName)
				{
					if (tarSgNode.activeNodeList[0] == m_toNewSgNodeIdMap[actNodeId]
						&& tarSgNode.anchorNodeList[0] == m_toNewSgNodeIdMap[anchorNodeId])
					{
						sgNode.isAligned = true;
						tarSgNode.isAligned = true;
						tarSgNode.matchingStatus = sgNode.matchingStatus;

						m_toNewSgNodeIdMap[qNi] = tarNi;  // save aligned pairwise relationship node into map

						alignScore += NodeScore[queryActiveNode.matchingStatus];
						break;
					}
				}
			}
		}

		if (sgNode.nodeType == "group_attribute")
		{
			if (sgNode.anchorNodeList.empty()) continue;

			int refNodeIdInQuery = sgNode.anchorNodeList[0];
			if (!m_toNewSgNodeIdMap.count(refNodeIdInQuery)) continue;

			SemNode &queryRefNode = this->m_nodes[refNodeIdInQuery];

			for (int tarNi = 0; tarNi < targetGraph->m_nodeNum; tarNi++)
			{
				SemNode& tarSgNode = targetGraph->m_nodes[tarNi];
				// skip the aligned nodes
				if (!tarSgNode.isAligned && tarSgNode.nodeType == "group_attribute" && sgNode.nodeName == tarSgNode.nodeName)
				{
					int tarRefId = tarSgNode.anchorNodeList[0];
					SemNode& tarRefNode = targetGraph->m_nodes[tarRefId];
					if (tarRefNode.nodeName == queryRefNode.nodeName)
					{
						sgNode.isAligned = true;
						tarSgNode.isAligned = true;
						tarSgNode.matchingStatus = sgNode.matchingStatus;

						m_toNewSgNodeIdMap[qNi] = tarNi;  // save aligned pairwise relationship node map

						alignScore += NodeScore[sgNode.matchingStatus];
						break;
					}
				}
			}
		}
	}
}

void SemanticGraph::setNodesUnAligned()
{
	for (int i = 0; i < m_nodeNum; i++)
	{
		m_nodes[i].isAligned = false;
	}
}

void SemanticGraph::mergeWithGraph(SemanticGraph *inputGraph)
{
	// insert all unaligned nodes
	for (int mi = 0; mi < inputGraph->m_nodeNum; mi++)
	{
		SemNode& inputSgNode = inputGraph->m_nodes[mi];
		if (!inputSgNode.isAligned)
		{
			// update graph
			this->addNode(inputSgNode);
			inputGraph->m_toNewSgNodeIdMap[mi] = this->m_nodeNum - 1;  // node id is the last node's id; save inserted node map
		}
	}

	// insert edges from matched ssg if it is not existing in current ssg
	for (int mei = 0; mei < inputGraph->m_edgeNum; mei++)
	{
		SemEdge& inputSgEdge = inputGraph->m_edges[mei];

		int s = inputGraph->m_toNewSgNodeIdMap[inputSgEdge.sourceNodeId];
		int t = inputGraph->m_toNewSgNodeIdMap[inputSgEdge.targetNodeId];
		if (!this->isEdgeExist(s, t))
		{
			this->addEdge(s, t);
		}
	}

	// update node labels, active and anchor node list
	this->parseNodeNeighbors();
}

SemanticGraph* SemanticGraph::alignAndMergeWithGraph(SemanticGraph *sg)
{
	double alignScore;

	sg->alignObjectNodesWithGraph(this, alignScore);
	sg->alignRelationNodesWithGraph(this, alignScore);

	this->mergeWithGraph(sg);

	return this;
}
