#include "SceneSemGraphManager.h"
#include "SceneSemGraph.h"
#include "Utility.h"


SceneSemGraphManager::SceneSemGraphManager(const QString &folderDir)
	: m_mainSceneDBFolderPath(folderDir)
{
	loadGraphs();
	loadNodeLabelMap();
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

	inFile.close();
}

void SceneSemGraphManager::loadNodeLabelMap()
{
	QString filename = m_mainSceneDBFolderPath + "/SSGNodeLabelMap.txt";
	QFile inFile(filename);
	QTextStream ifs(&inFile);

	if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		cout << "\n SceneSemGraphManager: cannot open node label map file\n";
		return;
	}

	m_nodeStringToLabelIDMap.clear();

	while (!ifs.atEnd())
	{
		QString currLine = ifs.readLine();

		std::vector<string> parts = PartitionString(currLine.toStdString(), " ");

		QString nodeNameString(parts[0].c_str());
		int labelID = StringToInt(parts[1]);
		int typeID = StringToInt(parts[2]);

		m_nodeStringToLabelIDMap[nodeNameString] = std::make_pair(labelID, typeID);
	}

	inFile.close();

}
