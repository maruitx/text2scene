#include "SceneSemGraphManager.h"
#include "SceneSemGraph.h"
#include "Utility.h"


SceneSemGraphManager::SceneSemGraphManager(const QString &folderDir)
	: m_mainSceneDBFolderPath(folderDir)
{
	loadGraphs();
}


SceneSemGraphManager::~SceneSemGraphManager()
{

}

void SceneSemGraphManager::loadGraphs()
{
	QString filename = m_mainSceneDBFolderPath + "/scene_list.txt";
	QFile inFile(filename);
	QTextStream ifs(&inFile);

	if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		cout << "\n SceneSemGraphManager: cannot open scene list file\n";
		return;
	}

	QString currLine = ifs.readLine();

	if (currLine.contains("StanfordSceneDatabase"))
	{
		int sceneNum = StringToIntegerList(currLine.toStdString(), "StanfordSceneDatabase ")[0];

		for (int i = 0; i < sceneNum; i++)
		{
			currLine = ifs.readLine();

			//
			QString ssgFileName = m_mainSceneDBFolderPath + "/SSGs/" + currLine + ".ssg";

			SceneSemGraph *newSceneSemGraph = new SceneSemGraph(ssgFileName);
			m_sceneSemGraphs.push_back(newSceneSemGraph);
		}
	}

	m_ssgNum = m_sceneSemGraphs.size();
}
