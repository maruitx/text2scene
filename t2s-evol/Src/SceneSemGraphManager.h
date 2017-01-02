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

	void loadNodeLabelMap();
	int getNodeLabelID(const QString &s) { return m_nodeStringToLabelIDMap[s].first; };
	int getNodeTypeID(const QString &s) { return m_nodeStringToLabelIDMap[s].second; };

	int m_ssgNum;

private:
	vector<SceneSemGraph*> m_sceneSemGraphs;
	QString m_mainSceneDBFolderPath;

	std::map<QString, std::pair<int, int>> m_nodeStringToLabelIDMap;
};

