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

void SemanticGraph::addNode(const SemNode &node)
{
	SemNode newNode = SemNode(node.nodeType, node.nodeName, m_nodeNum);
	newNode.isAligned = node.isAligned;
	newNode.isAligned = node.isAligned;

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

void SemanticGraph::alignObjectNodesWithGraph(SemanticGraph *targetGraph, std::map<int, int> &queryToTargetNodeIdMap, double &alignScore)
{
	double NodeScore[] = { 0.5, 1 };

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
					if (!sgNode.inEdgeNodeList.empty())
					{

						int matchedAttNum = 0;
						int sgAttNum = 0;

						// align attribute node 
						for (int ai = 0; ai < sgNode.inEdgeNodeList.size(); ai++)
						{
							int attNodeId = sgNode.inEdgeNodeList[ai];  // id of attribute node in tsg
							SemNode &attNode = this->m_nodes[attNodeId];

							if (attNode.nodeType == "per_obj_attribute")
							{
								sgAttNum++;
								if (!tarSgNode.inEdgeNodeList.empty())
								{
									for (int tarAi = 0; tarAi < tarSgNode.inEdgeNodeList.size(); tarAi++)
									{
										int tarAttNodeId = tarSgNode.inEdgeNodeList[tarAi]; // id of attribute node in dbssg
										SemNode &tarAttNode = targetGraph->m_nodes[tarAttNodeId];

										if (attNode.nodeType == tarAttNode.nodeType && attNode.nodeName == tarAttNode.nodeName)
										{
											matchedAttNum++;

											attNode.isAligned = true;
											tarAttNode.isAligned = true;
											queryToTargetNodeIdMap[attNodeId] = tarAttNodeId; // save aligned attribute node into map		
											alignScore += 0;  // attribute node does not contribute to matching score
										}
									}
								}
							}
						}

						//  if all attribute nodes matched, then the node is matched
						if (matchedAttNum == sgAttNum)
						{
							sgNode.isAligned = true;
							tarSgNode.isAligned = true;
							queryToTargetNodeIdMap[qNi] = tarNi; // save aligned object node into map									
							alignScore += NodeScore[sgNode.matchingStatus];
							break;
						}
						else
						{
							sgNode.isAligned = true;
							tarSgNode.isAligned = true;
							queryToTargetNodeIdMap[qNi] = tarNi; // save partial aligned object node into map									
							alignScore += 0.5*NodeScore[sgNode.matchingStatus];;
							break;
						}
					}
					else
					{
						sgNode.isAligned = true;
						tarSgNode.isAligned = true;
						queryToTargetNodeIdMap[qNi] = tarNi; // save aligned object node map
						alignScore += NodeScore[sgNode.matchingStatus];
						break;
					}
				}
			}
		}
	}
}

void SemanticGraph::alignRelationNodesWithGraph(SemanticGraph *targetGraph, std::map<int, int> &queryToTargetNodeIdMap, double &alignScore)
{
	double NodeScore[] = { 0.5, 1 };

	for (int qNi = 0; qNi < this->m_nodeNum; qNi++)
	{
		SemNode& sgNode = this->m_nodes[qNi];

		// align pair-wise relationship node
		if (sgNode.nodeType == "pairwise_relationship")
		{
			// To test whether in and out node exist
			if (sgNode.inEdgeNodeList.empty() || sgNode.outEdgeNodeList.empty())
			{
				break;
			}

			// edge dir: (active, relation), (relation, reference)
			int inNodeId = sgNode.inEdgeNodeList[0];
			int outNodeId = sgNode.outEdgeNodeList[0];

			// if any object node is not in the matched map (not matched), then break
			if (!queryToTargetNodeIdMap.count(inNodeId) || !queryToTargetNodeIdMap.count(outNodeId))
			{
				break;
			}

			for (int tarNi = 0; tarNi < targetGraph->m_nodeNum; tarNi++)
			{
				SemNode& tarSgNode = targetGraph->m_nodes[tarNi];
				// skip the aligned nodes
				if (!tarSgNode.isAligned && tarSgNode.nodeType == "pairwise_relationship" && sgNode.nodeName == tarSgNode.nodeName)
				{
					if (tarSgNode.inEdgeNodeList[0] == queryToTargetNodeIdMap[inNodeId]
						&& tarSgNode.outEdgeNodeList[0] == queryToTargetNodeIdMap[outNodeId])
					{
						sgNode.isAligned = true;
						tarSgNode.isAligned = true;
						queryToTargetNodeIdMap[qNi] = tarNi;  // save aligned pairwise relationship node into map

						alignScore += NodeScore[sgNode.matchingStatus];
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

			int refNodeIdInQuery = sgNode.outEdgeNodeList[0];
			if (!queryToTargetNodeIdMap.count(refNodeIdInQuery))
			{
				break;
			}

			SemNode &queryRefNode = this->m_nodes[refNodeIdInQuery];

			for (int tarNi = 0; tarNi < targetGraph->m_nodeNum; tarNi++)
			{
				SemNode& tarSgNode = targetGraph->m_nodes[tarNi];
				// skip the aligned nodes
				if (!tarSgNode.isAligned && tarSgNode.nodeType == "group_attribute" && sgNode.nodeName == tarSgNode.nodeName)
				{
					int tarRefId = tarSgNode.outEdgeNodeList[0];
					SemNode& tarRefNode = targetGraph->m_nodes[tarRefId];
					if (tarRefNode.nodeName == queryRefNode.nodeName)
					{
						sgNode.isAligned = true;
						tarSgNode.isAligned = true;
						queryToTargetNodeIdMap[qNi] = tarNi;  // save aligned pairwise relationship node map

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

void SemanticGraph::mergeWithGraph(SemanticGraph *inputGraph, std::map<int, int> &inputToNewSGNodeIdMap)
{
	// insert all unaligned nodes
	for (int mi = 0; mi < inputGraph->m_nodeNum; mi++)
	{
		SemNode& inputSgNode = inputGraph->m_nodes[mi];
		if (!inputSgNode.isAligned)
		{
			// update graph
			this->addNode(inputSgNode.nodeType, inputSgNode.nodeName);
			inputToNewSGNodeIdMap[mi] = this->m_nodeNum - 1;  // node id is the last node's id; save inserted node map
		}
	}

	// insert edges from matched ssg if it is not existing in current ssg
	for (int mei = 0; mei < inputGraph->m_edgeNum; mei++)
	{
		SemEdge& inputSgEdge = inputGraph->m_edges[mei];

		int s = inputToNewSGNodeIdMap[inputSgEdge.sourceNodeId];
		int t = inputToNewSGNodeIdMap[inputSgEdge.targetNodeId];
		if (!this->isEdgeExist(s, t))
		{
			this->addEdge(s, t);
		}
	}
}

SemanticGraph* SemanticGraph::alignAndMergeWithGraph(SemanticGraph *sg)
{
	std::map<int, int> queryToTargetNodeIdMap;
	double alignScore;

	sg->alignObjectNodesWithGraph(this, queryToTargetNodeIdMap, alignScore);
	sg->alignRelationNodesWithGraph(this, queryToTargetNodeIdMap, alignScore);

	std::map<int, int> matchToNewSgNodeMap;
	this->mergeWithGraph(sg, matchToNewSgNodeMap);

	return this;
}
