#pragma once

#include "Headers.h"

class SceneSemGraph;

class SceneSemGraphManager
{
public:
	SceneSemGraphManager(const QString &folderDir);
	~SceneSemGraphManager();

	void loadGraphs();
	SceneSemGraph* getGraph(int id) { return m_sceneSemGraphs[id]; };


private:
	vector<SceneSemGraph*> m_sceneSemGraphs;

	QString m_mainSceneDBFolderPath;
};

