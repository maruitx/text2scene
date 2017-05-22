#pragma once

#include "Headers.h"
#include "MetaData.h"

class SceneSemGraph;

class SceneSemGraphManager
{
public:
	SceneSemGraphManager();
	~SceneSemGraphManager();

	void loadGraphs();
	SceneSemGraph* getGraph(int id) { return m_sceneSemGraphs[id]; };

	int m_ssgNum;

	void loadModelBlackList();
	bool isModelInBlackList(const QString &s);
	MetaModel& retrieveForModelInstance(const QString catName, const std::vector<QString> attriNames = std::vector<QString>());

private:
	vector<SceneSemGraph*> m_sceneSemGraphs;
	QString m_mainSceneDBFolderPath;

	std::set<QString> m_modelBlackList;  // list of models that is irregular and need to be blacked

};

