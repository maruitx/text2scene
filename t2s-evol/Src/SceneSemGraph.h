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

	void mergeWithMatchedSSG(SceneSemGraph *matchedSg, std::map<int, int> &matchToNewUserSsgNodeMap); // update current USserSSG with retrieved subSSG


	int findParentNodeId(int modelId);
	int getNodeIdWithModelId(int modelId);

public:
	MetaScene m_metaScene;

	std::map<int, int> m_objectGraphNodeToModelListIdMap;	

	map<int, int> m_dbNodeToSubNodeMap;


private:
	int m_modelNum;
	QString m_fullFilename;
};

