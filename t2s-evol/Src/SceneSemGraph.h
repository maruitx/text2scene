#pragma once

#include "SemanticGraph.h"
#include "MetaData.h"

class ModelDatabase;
class TSScene;
class Model;


class SceneSemGraph : public SemanticGraph
{
public:
	SceneSemGraph();
	SceneSemGraph(const QString &s);
	SceneSemGraph(SceneSemGraph *sg); // generate a copy of sg
	~SceneSemGraph();

	void loadGraph(const QString &filename);
	TSScene* covertToTSScene(unordered_map<string, Model*> &models);

	SceneSemGraph* getSubGraph(const vector<int> &nodeList, bool useContext = false);

	void mergeWithMatchedSSG(SceneSemGraph *matchedSg); // update current USserSSG with retrieved subSSG

	bool findRefNodeForRelationNode(const SemNode &sgNode, int &refNodeId, int &activeNodeId);
	int findParentNodeIdForModel(int modelId);

	int getNodeIdWithModelId(int modelId);
	MetaModel& getModelWithNodeId(int nodeId);

public:
	MetaScene m_metaScene;

	std::map<int, int> m_objectGraphNodeToModelListIdMap;	

	map<int, int> m_dbNodeToSubNodeMap;

	int m_idInMatchList;

private:
	int m_modelNum;
	QString m_fullFilename;
};

