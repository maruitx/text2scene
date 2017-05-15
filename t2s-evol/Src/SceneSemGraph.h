#pragma once

#include "SemanticGraph.h"
#include "MetaData.h"

class RelationModelManager;
class ModelDatabase;
class TSScene;
class Model;

const QString floorObjs[] = { "desk", "table", "bed", "chair", "couch", "cabinet", "dresser", "cabinet" };


class SceneSemGraph : public SemanticGraph
{
public:
	SceneSemGraph();
	SceneSemGraph(const QString &s);
	SceneSemGraph(SceneSemGraph *sg); // generate a copy of sg
	~SceneSemGraph();

	void loadGraph(const QString &filename);
	TSScene* covertToTSScene(unordered_map<string, Model*> &models);
	void saveGraph(const QString &filename);

	void initMetaModelSuppPlanes(unordered_map<string, Model*> &models);

	SceneSemGraph* getSubGraph(const vector<int> &nodeList, RelationModelManager *relManager);
	void restoreMissingSupportNodes();
	bool hasSupportNode(int actNodeId);

	void mergeWithMatchedSSG(SceneSemGraph *matchedSg); // update current USserSSG with retrieved subSSG
	void insertUnAlignedObjsFromGraph(SceneSemGraph *matchedSg);

	bool findRefNodeForRelationNode(const SemNode &sgNode, int &refNodeId, int &activeNodeId);
	int findParentNodeIdForModel(int modelId);
	int findParentNodeIdForNode(int nodeId);
	int findPotentialParentNodeIdForNode(int nodeId);

	int getNodeIdWithModelId(int modelId);
	MetaModel& getModelWithNodeId(int nodeId);

	void buildSupportHierarchy();
	bool isBaseLevelObj(int nodeId);

	std::vector<int> findExistingInstanceIds(const QString &catName);


public:
	MetaScene m_metaScene;
	std::map<int, int> m_graphNodeToModelListIdMap;	

	// for matching
	map<int, int> m_dbNodeToSubNodeMap;

	// support hierarchy
	map<int, int> m_parentOfModel;  //  map to modelId of current model's parent
	map<int, vector<int>> m_childListOfModel;  // map to modelIds of current model's children
	std::vector<std::vector<int>> m_levelOfObjs;  // map to modelIds of objs in different support levels

	bool m_allSynthNodesInited;
	int m_modelNum;

	int m_dbSSGId;  // id of matched dbSSG
	SemanticGraph *m_alignedQuerySSG;

private:
	QString m_fullFilename;
};

