#pragma once

#include "Headers.h"

const QString SSGNodeType[] = { "object", "attribute", "pair_relation", "group_relation"};

class SemNode{
public:
	enum NodeType {
		Obj = 0,
		Attri,
		Pair,
		Group
	};


	SemNode(const QString &t, const QString &n, int id) { nodeType = t; nodeName = n; nodeId = id; 
	isAligned = false; isSynthesized = false; matchingStatus = 0;
	isInferred = false;  inferedType = -1;  inferRefNodeId = -1; isAnnotated = false; useNewInst = false;
	};
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

	// parsed from in and out edge node list
	std::vector<int> nodeLabels;  // idx of per-object attribute node, valid for "object" node
	std::vector<int> activeNodeList; // idx of active object node, valid for "relation" node
	std::vector<int> anchorNodeList; // idx of anchor object node, valid for "relation" node

	bool isAligned;
	bool isSynthesized;
	bool useNewInst; // whether use a new instance for current object node

	enum InferNodeType{
		InferBySupport = 0,
		InferByContext
	};

	bool isInferred;
	int inferedType;
	int inferRefNodeId;  // reference object id of the inferred object

	enum NodeMatchingStatus
	{
		ContextNode=1,
		ExplicitNode
	};

	int matchingStatus;

	bool isAnnotated;

};

class SemEdge{
public:
	SemEdge(int s, int t, int id) { sourceNodeId = s; targetNodeId = t; edgeId = id;};
	~SemEdge() {};

	void reverseEdgeDir();

	int sourceNodeId, targetNodeId;
	int edgeId;
};

class SemanticGraph
{
public:
	SemanticGraph();
	SemanticGraph(SemanticGraph *sg);  // generate a copy of sg
	~SemanticGraph();

	void addNode(const QString &nType, const QString &nName);
	void addNode(const SemNode &node);

	void addEdge(int s, int t); // after adding edge, parseNodeNeighbors must be called to update the neighbor list
	void parseNodeNeighbors();  // parse in and out node list to node labels, active and anchor node list

	std::vector<int> findNodeWithName(const QString &nName);
	bool isEdgeExist(int s, int t);
	SemEdge& getEdge(int s, int t);
	

	void setNodesUnAligned();

	void alignObjectNodesWithGraph(SemanticGraph *targetGraph, double &alignScore, bool userDeterminer =false);
	void alignRelationNodesWithGraph(SemanticGraph *targetGraph, double &alignScore);

	void mergeWithGraph(SemanticGraph *inputGraph);
	SemanticGraph* alignAndMergeWithGraph(SemanticGraph *sg);

	bool isAnchor(int nodeId);

	void initAttributeSet();
	bool isStrictAttribute(const QString &attriName);
	bool isGroupAttribute(const QString &attriName);
	std::vector<QString> getAttriNamesForNode(int nodeId);

public:
	int m_nodeNum, m_edgeNum;
	std::vector<SemNode> m_nodes;
	std::vector<SemEdge> m_edges;

	std::map<int, int> m_nodeAlignMap;  // current to target node id alignment map
	std::vector<QString> m_strictAttriSet; // strict attribute that must be satisfied
	std::vector<QString> m_groupAttriSet;
};

