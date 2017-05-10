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

	m_nodeAlignMap = sg->m_nodeAlignMap;
}


SemanticGraph::~SemanticGraph()
{
}

void SemanticGraph::addNode(const QString &nType, const QString &nName)
{
	QString unifiedType = nType;

	if (nType == "group_relation_anno")
	{
		unifiedType = SSGNodeType[SemNode::Group];
	}

	SemNode newNode = SemNode(unifiedType, nName, m_nodeNum);
	m_nodes.push_back(newNode);

	m_nodeNum++;
}

void SemanticGraph::addNode(const SemNode &node)
{
	SemNode newNode = SemNode(node.nodeType, node.nodeName, m_nodeNum);
	newNode.isAligned = node.isAligned;
	newNode.matchingStatus = node.matchingStatus;
	newNode.isSynthesized = node.isSynthesized;
	newNode.useNewInst = node.useNewInst;

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

		if (sgNode.nodeType.contains("relation") || sgNode.nodeType.contains("attribute"))
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

SemEdge& SemanticGraph::getEdge(int s, int t)
{
	for (int i = 0; i < m_edgeNum; i++)
	{
		if (m_edges[i].sourceNodeId == s && m_edges[i].targetNodeId == t)
		{
			return m_edges[i];
		}
	}
}

void SemanticGraph::alignObjectNodesWithGraph(SemanticGraph *targetGraph, double &alignScore, bool userDeterminer/*= false*/)
{
	double NodeScore[] = { 0, 0, 20};

	// first match object node and per-object attribute node
	for (int qNi = 0; qNi < this->m_nodeNum; qNi++)
	{
		SemNode& objNode = this->m_nodes[qNi];

		if (objNode.nodeType == "object")
		{
			// not aligned this node if marked for use new instance
			if (userDeterminer && objNode.useNewInst) continue;

			for (int tarNi = 0; tarNi < targetGraph->m_nodeNum; tarNi++)
			{
				SemNode& tarObjNode = targetGraph->m_nodes[tarNi];
				// skip the aligned nodes
				if (!tarObjNode.isAligned && tarObjNode.nodeType == "object" && objNode.nodeName == tarObjNode.nodeName)
				{
					if (!objNode.nodeLabels.empty())
					{
						int matchedAttNum = 0;
						int sgAttNum = 0;

						// align attribute node
						for (int ai = 0; ai < objNode.nodeLabels.size(); ai++)
						{
							int attNodeId = objNode.nodeLabels[ai];

							SemNode &attNode = this->m_nodes[attNodeId];
							sgAttNum++;

							if (!tarObjNode.nodeLabels.empty())
							{
								for (int tarAi = 0; tarAi < tarObjNode.nodeLabels.size(); tarAi++)
								{
									int tarAttNodeId = tarObjNode.nodeLabels[tarAi]; // id of attribute node in dbssg
									SemNode &tarAttNode = targetGraph->m_nodes[tarAttNodeId];

									if (attNode.nodeType == tarAttNode.nodeType && attNode.nodeName == tarAttNode.nodeName)
									{
										matchedAttNum++;

										attNode.isAligned = true;
										tarAttNode.isAligned = true;
										tarAttNode.matchingStatus = attNode.matchingStatus;

										m_nodeAlignMap[attNodeId] = tarAttNodeId; // save aligned attribute node into map		
										alignScore += 0;  // attribute node does not contribute to matching score
									}
								}
							}
						}

						//  if all attribute nodes matched, then the node is matched
						if (matchedAttNum == sgAttNum)
						{
							objNode.isAligned = true;
							tarObjNode.isAligned = true;
							tarObjNode.matchingStatus = objNode.matchingStatus;
							tarObjNode.useNewInst = objNode.useNewInst;

							m_nodeAlignMap[qNi] = tarNi; // save aligned object node into map									
							alignScore += NodeScore[objNode.matchingStatus];
							break;
						}
						else
						{
							objNode.isAligned = true;
							tarObjNode.isAligned = true;

							//objNode.isAligned = false;
							//tarObjNode.isAligned = false;

							tarObjNode.matchingStatus = objNode.matchingStatus;
							tarObjNode.useNewInst = objNode.useNewInst;

							m_nodeAlignMap[qNi] = tarNi; // save partial aligned object node into map									
							alignScore += 0.5*NodeScore[objNode.matchingStatus];
							break;
						}
					}
					else
					{
						objNode.isAligned = true;
						tarObjNode.isAligned = true;
						tarObjNode.matchingStatus = objNode.matchingStatus;
						tarObjNode.useNewInst = objNode.useNewInst;

						m_nodeAlignMap[qNi] = tarNi; // save aligned object node map
						alignScore += NodeScore[objNode.matchingStatus];
						break;
					}
				}
			}
		}
	}
}

void SemanticGraph::alignRelationNodesWithGraph(SemanticGraph *targetGraph, double &alignScore)
{
	double NodeScore[] = { 0, 0, 20 };

	for (int qNi = 0; qNi < this->m_nodeNum; qNi++)
	{
		SemNode& relNode = this->m_nodes[qNi];

		// align pair-wise relationship node
		if (relNode.nodeType == "pair_relation")
		{
			// To test whether in and out node exist
			if (relNode.activeNodeList.empty() || relNode.anchorNodeList.empty()) continue;

			// edge dir: (active, relation), (relation, reference)
			int actNodeId = relNode.activeNodeList[0];
			int anchorNodeId = relNode.anchorNodeList[0];

			// if any object node is not in the matched map (not matched), then break
			if (!m_nodeAlignMap.count(actNodeId) || !m_nodeAlignMap.count(anchorNodeId)) continue;

			SemNode& queryActiveNode = m_nodes[actNodeId];

			for (int tarNi = 0; tarNi < targetGraph->m_nodeNum; tarNi++)
			{
				SemNode& tarRelNode = targetGraph->m_nodes[tarNi];
				// skip the aligned nodes
				if (!tarRelNode.isAligned && tarRelNode.nodeType == "pair_relation" && relNode.nodeName == tarRelNode.nodeName)
				{
					if (tarRelNode.activeNodeList[0] == m_nodeAlignMap[actNodeId]
						&& tarRelNode.anchorNodeList[0] == m_nodeAlignMap[anchorNodeId])
					{
						relNode.isAligned = true;
						tarRelNode.isAligned = true;
						tarRelNode.matchingStatus = relNode.matchingStatus;

						m_nodeAlignMap[qNi] = tarNi;  // save aligned pairwise relationship node into map

						alignScore += NodeScore[queryActiveNode.matchingStatus];
						break;
					}
				}
			}
		}

		if (relNode.nodeType == "group_relation")
		{
			if (relNode.anchorNodeList.empty()) continue;

			int refNodeIdInQuery = relNode.anchorNodeList[0];
			if (!m_nodeAlignMap.count(refNodeIdInQuery)) continue;

			SemNode &queryRefNode = this->m_nodes[refNodeIdInQuery];

			for (int tarNi = 0; tarNi < targetGraph->m_nodeNum; tarNi++)
			{
				SemNode& tarRelNode = targetGraph->m_nodes[tarNi];
				// skip the aligned nodes
				if (relNode.matchingStatus == SemNode::ExplicitNode
					&&!tarRelNode.isAligned && tarRelNode.nodeType == "group_relation" && relNode.nodeName == tarRelNode.nodeName)
				{
					int tarRefId = tarRelNode.anchorNodeList[0];
					SemNode& tarRefNode = targetGraph->m_nodes[tarRefId];
					if (tarRefNode.nodeName == queryRefNode.nodeName)
					{
						relNode.isAligned = true;
						tarRelNode.isAligned = true;
						tarRelNode.matchingStatus = relNode.matchingStatus;

						m_nodeAlignMap[qNi] = tarNi;  // save aligned pairwise relationship node map
						m_nodeAlignMap[refNodeIdInQuery] = tarRefId;  // update the ref node id in the map, since multiple instances may exist in the scene and we use the one connected to the relation node

						alignScore += NodeScore[relNode.matchingStatus];
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
			inputGraph->m_nodeAlignMap[mi] = this->m_nodeNum - 1;  // node id is the last node's id; save inserted node map
		}
	}

	// insert edges from matched ssg if it is not existing in current ssg
	for (int mei = 0; mei < inputGraph->m_edgeNum; mei++)
	{
		SemEdge& inputSgEdge = inputGraph->m_edges[mei];

		int s = inputGraph->m_nodeAlignMap[inputSgEdge.sourceNodeId];
		int t = inputGraph->m_nodeAlignMap[inputSgEdge.targetNodeId];
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
	sg->alignObjectNodesWithGraph(this, alignScore, true);
	sg->alignRelationNodesWithGraph(this, alignScore);

	this->mergeWithGraph(sg);

	return this;
}

void SemEdge::reverseEdgeDir()
{
	int tempId = sourceNodeId;
	sourceNodeId = targetNodeId;
	targetNodeId = tempId;
}
