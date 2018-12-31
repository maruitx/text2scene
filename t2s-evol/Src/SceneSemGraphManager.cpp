#include "SceneSemGraphManager.h"
#include "SceneSemGraph.h"
#include "Utility.h"

SceneSemGraphManager::SceneSemGraphManager()
{
	loadGraphs();
	loadModelBlackList();
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
		m_sceneSemGraphs.resize(sceneNum);

		std::chrono::high_resolution_clock::time_point start = GetCurrentClockTime();

		std::vector<QString> allSSGFileNames(sceneNum);
		
		// load all file names
		for (int i = 0; i < sceneNum; i++)
		{
			currLine = ifs.readLine();

			QString ssgFileName = "./SceneDB/SSGs/" + currLine + ".ssg";
			allSSGFileNames[i] = ssgFileName;
		}

		//#pragma omp parallel for
		for (int i = 0; i < sceneNum; i++)
		{
			SceneSemGraph *newSceneSemGraph = new SceneSemGraph(allSSGFileNames[i]);
			m_sceneSemGraphs[i] = newSceneSemGraph;
		}

		std::cout << sceneNum << " graphs loaded in " << GetElapsedTime(start) << " s\n";
	}

	m_ssgNum = m_sceneSemGraphs.size();

	inFile.close();
}

void SceneSemGraphManager::loadModelBlackList()
{
	QString filename = "./SceneDB/model_blacklist.txt";
	QFile inFile(filename);
	QTextStream ifs(&inFile);

	if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;

	while (!ifs.atEnd())
	{
		QString currLine = ifs.readLine();
		QStringList parts = currLine.split(",");
		m_modelBlackList.insert(parts[0]);
	}
	inFile.close();
}

bool SceneSemGraphManager::isModelInBlackList(const QString &s)
{
	if (m_modelBlackList.count(s))
	{
		return true;
	}
	else
	{
		return false;
	}
}

MetaModel& SceneSemGraphManager::retrieveForModelInstance(const QString catName, const std::vector<QString> attriNames /*= std::vector<QString>()*/)
{
	std::vector<std::pair<int, int>> exactMatchMdIds;
	std::vector<std::pair<int, int>> otherCandiMdIds;

	for (int i = 0; i < this->m_ssgNum; i++)
	{
		SceneSemGraph *currDBSSG = this->getGraph(i);

		for (int qNi = 0; qNi < currDBSSG->m_nodeNum; qNi++)
		{
			SemNode& sgNode = currDBSSG->m_nodes[qNi];
			if (sgNode.nodeType == "object" && sgNode.nodeName == catName)
			{
				int modelId = currDBSSG->m_graphNodeToModelListIdMap[qNi];
				MetaModel &md = currDBSSG->m_graphMetaScene.m_metaModellList[modelId];

				// test front dir for book to filter stand book
				if (catName == "book" && md.frontDir.dot(vec3(0, 0, 1)) < 0.5)
				{
					continue;
				}

				if (md.catName == "desk")
				{
					// 5ce562e0632b7e81d8e3889d90601dd1					
					// 6d67f92d6e11f29e5792b99b8245d225
					//if (md.name == "6d67f92d6e11f29e5792b99b8245d225")
					//{
					//	return md;
					//}
				}

				if (md.catName == "bed")
				{
					// 5ce562e0632b7e81d8e3889d90601dd1					
					// 6d67f92d6e11f29e5792b99b8245d225
					if (md.name == "b32ad01b13c6739daaddabe471cbd38e")
					{
						return md;
					}
				}

				if (md.catName == "monitor")
				{
					if (md.name == "241b26438e8523ee2846fa729d90e125")
					{
						return md;
					}
				}

				
				if (md.catName == "computermouse")
				{
					if (md.name == "90490d506c40f6b344ebbc705ad8c07")
					{
						return md;
					}
				}

				

				if (md.catName == "chair")
				{
					// b6f10c040560d6e41f6a5b0146bf3030
					// 1308f3ff2e55eae4f1783a44a88d6274
					//if (md.name == "1308f3ff2e55eae4f1783a44a88d6274")
					//{
					//	return md;
					//}
				}

				if (md.catName == "table")
				{
					//if (md.name == "97ac69442e657fcd7a968eef090b6698")
					//{
					//	return md;
					//}
				}

				if (!isModelInBlackList(toQString(md.name)))
				{
					// collect attri names for current node
					std::vector<QString> currAttriNames = currDBSSG->getAttriNamesForNode(qNi);

					// match attributes
					int mAttriNum = 0;
					for (int t = 0; t < attriNames.size(); t++)
					{
						if (std::find(currAttriNames.begin(), currAttriNames.end(), attriNames[t]) != currAttriNames.end())
						{
							mAttriNum++;
						}
					}

					if (mAttriNum == attriNames.size())
					{
						exactMatchMdIds.push_back(std::make_pair(i, modelId));  // (graphId, modelId)
					}
					else
					{
						otherCandiMdIds.push_back(std::make_pair(i, modelId));
					}
				}
			}
		}
	}

	if (!exactMatchMdIds.empty())
	{
		int randId = GenRandomInt(0, exactMatchMdIds.size());
		std::pair<int, int> selectPair = exactMatchMdIds[randId];

		SceneSemGraph *currDBSSG = this->getGraph(selectPair.first);
		return currDBSSG->m_graphMetaScene.m_metaModellList[selectPair.second];
	}
	else if (!otherCandiMdIds.empty())
	{
		int randId = GenRandomInt(0, otherCandiMdIds.size());
		std::pair<int, int> selectPair = otherCandiMdIds[randId];

		SceneSemGraph *currDBSSG = this->getGraph(selectPair.first);
		return currDBSSG->m_graphMetaScene.m_metaModellList[selectPair.second];
	}
	else
	{
		// Failure case: return a room for debug
		return MetaModel();
	}
}
