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
	~SceneSemGraph();

	void loadGraph(const QString &filename);
	TSScene* covertToTSScene(unordered_map<string, Model*> &models);

	SceneSemGraph* getSubGraph(const vector<int> &nodeList);

public:
	MetaScene m_metaScene;


private:
	int m_modelNum;

	QString m_fullFilename;
};

