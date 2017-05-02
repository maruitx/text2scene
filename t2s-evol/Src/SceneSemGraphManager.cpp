#include "SceneSemGraphManager.h"
#include "SceneSemGraph.h"
#include "Utility.h"

SceneSemGraphManager::SceneSemGraphManager()
{
	loadGraphs();
}


SceneSemGraphManager::~SceneSemGraphManager()
{

}

void SceneSemGraphManager::loadGraphs()
{
	QString filename = QString("./SceneDB/scene_list_%1.txt").arg(params::inst()->sceneDBType);
	QFile inFile(filename);
	QTextStream ifs(&inFile);

	if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		cout << "\n SceneSemGraphManager: cannot open scene list file\n";
		return;
	}

	QString currLine = ifs.readLine();

	if (currLine.contains("SceneNum"))
	{
		int sceneNum = StringToIntegerList(currLine.toStdString(), "SceneNum ")[0];

		for (int i = 0; i < sceneNum; i++)
		{
			currLine = ifs.readLine();

			QString ssgFileName = "./SceneDB/SSGs/" + currLine + ".ssg";

			SceneSemGraph *newSceneSemGraph = new SceneSemGraph(ssgFileName);
			m_sceneSemGraphs.push_back(newSceneSemGraph);
		}
	}

	m_ssgNum = m_sceneSemGraphs.size();

	inFile.close();
}
