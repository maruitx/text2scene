#pragma once

#include "Headers.h"

class SemNode{
public:
	SemNode(const QString &t, const QString &n, int id) { nodeType = t; nodeName = n; nodeId = id; isAligned = false; isInferred = false;  inferedType = -1;  inferRefNodeId = -1; isAnnotated = false; };
	~SemNode() {};

	// node types: object, p_attribute, p_relation, g_relation, g_attribute, 
	QString nodeType;

	// model category name, relationship name or attribute name, e.g. chair, support, messy
	QString nodeName;

	int nodeId;

	// node id of the other end of in edge
	// for object node: saves the non-object node (relations, attributes) id to the object node, e.g. table <-- dining,  chair <-- support
	// for relation/attribute node: saves passive object id to relation/attribute node, e.g. support <-- table
	std::vector<int> inEdgeNodeList;

	// node id of the other end of out edge
	// for relation/attribute node: from relation/attribute node to object, e.g. messy --> book, messy --> utensils
	// for object: from passive object to relation/attribute node, e.g. table --> support
	std::vector<int> outEdgeNodeList;

	bool isAligned;
	bool isSynthesized;

	enum InferNodeType{
		InferBySupport = 0,
		InferByContext
	};

	bool isInferred;
	int inferedType;
	int inferRefNodeId;  // reference object id of the inferred object

	bool isAnnotated;

};

class SemEdge{
public:
	SemEdge(int s, int t, int id) { sourceNodeId = s; targetNodeId = t; edgeId = id; isAligned = false; isMatched = false; };
	~SemEdge() {};

	int sourceNodeId, targetNodeId;
	int edgeId;

	bool isMatched;
	bool isAligned;
};

class SemanticGraph
{
public:
	SemanticGraph();
	~SemanticGraph();

	void addNode(const QString &nType, const QString &nName);
	void addNode(const SemNode &node);

	void addEdge(int s, int t);

	std::vector<int> findNodeWithName(const QString &nName);
	bool isEdgeExist(int s, int t);

	void setNodesUnAligned();

	void alignObjectNodesWithGraph(SemanticGraph *targetGraph, std::map<int, int> &queryToTargetNodeIdMap, double &alignScore);
	void alignRelationNodesWithGraph(SemanticGraph *targetGraph, std::map<int, int> &queryToTargetNodeIdMap, double &alignScore);

	void mergeWithGraph(SemanticGraph *inputGraph, std::map<int, int> &inputToNewSGNodeIdMap);

public:
	int m_nodeNum, m_edgeNum;
	std::vector<SemNode> m_nodes;
	std::vector<SemEdge> m_edges;
};

