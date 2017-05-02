#pragma once

#include "Headers.h"

class SceneSemGraph;

class SceneSemGraphManager
{
public:
	SceneSemGraphManager();
	~SceneSemGraphManager();

	void loadGraphs();
	SceneSemGraph* getGraph(int id) { return m_sceneSemGraphs[id]; };

	int m_ssgNum;

private:
	vector<SceneSemGraph*> m_sceneSemGraphs;
	QString m_mainSceneDBFolderPath;
};

