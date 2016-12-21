#include "SceneSemGraph.h"
#include "Model.h"
#include "TSScene.h"
#include "Utility.h"

SceneSemGraph::SceneSemGraph()
{

}

SceneSemGraph::SceneSemGraph(const QString &s)
	: m_fullFilename(s)
{
	loadGraph(m_fullFilename);
}

SceneSemGraph::~SceneSemGraph()
{

}

void SceneSemGraph::loadGraph(const QString &filename)
{
	m_fullFilename = filename;

	QFile inFile(filename);
	QTextStream ifs(&inFile);

	if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		cout << "\nSceneSemGraph: cannot open scene semantic graph file\n";
		return;
	}

	QFileInfo sceneFileInfo(inFile.fileName());
	m_metaScene.m_sceneFileName = sceneFileInfo.baseName();   // scene_01.txt

	ifs >> m_metaScene.m_sceneFormat;

	if (m_metaScene.m_sceneFormat == "StanfordSceneDatabase")
	{
		int cutPos = sceneFileInfo.absolutePath().lastIndexOf("/");
		m_metaScene.m_sceneDBPath = sceneFileInfo.absolutePath().left(cutPos) + "/StanfordSceneDB";

		m_metaScene.m_sceneFilePath = m_metaScene.m_sceneDBPath + "/scenes";
		m_metaScene.m_modelRepository = m_metaScene.m_sceneDBPath + "/models";
	}
	else if (m_metaScene.m_sceneFormat == "SceneNNConversionOutput")
	{

	}

	
	while (!ifs.atEnd())
	{
		QString currLine = ifs.readLine();

		// load model info


		if (currLine.contains("modelCount "))
		{
			m_modelNum = StringToIntegerList(currLine.toStdString(), "modelCount ")[0];
			m_metaScene.m_metaModellList.resize(m_modelNum);

			for (int i = 0; i < m_modelNum; i++)
			{
				currLine = ifs.readLine();
				if (currLine.contains("newModel "))
				{
					std::vector<std::string> parts = PartitionString(currLine.toStdString(), " ");
					int modelIndex = StringToInt(parts[1]);

					m_metaScene.m_metaModellList[i].id = modelIndex;
					m_metaScene.m_metaModellList[i].name = parts[2];
					m_metaScene.m_metaModellList[i].path = m_metaScene.m_modelRepository.toStdString() + "/" + parts[2] + ".obj";


					currLine = ifs.readLine();

					if (currLine.contains("transform "))
					{
						std::vector<float> transformVec = StringToFloatList(currLine.toStdString(), "transform ");  // transformation vector in stanford scene file is column-wise
						mat4 transMat(transformVec.data());
						transMat = transMat.transpose();
						m_metaScene.m_metaModellList[i].transformation = transMat;
					}
				}
			}
		}

		// load nodes
		if (currLine.contains("nodeNum "))
		{
			int nodeNum = StringToIntegerList(currLine.toStdString(), "nodeNum ")[0];
			for (int i = 0; i < nodeNum; i++)
			{
				currLine = ifs.readLine();
				std::vector<std::string> parts = PartitionString(currLine.toStdString(), ",");
				addNode(QString(parts[1].c_str()), QString(parts[2].c_str()));
			}
		}

		// load edges
		if (currLine.contains("edgeNum "))
		{
			int edgeNum = StringToIntegerList(currLine.toStdString(), "edgeNum ")[0];
			for (int i = 0; i < edgeNum; i++)
			{

			}
		}
	}



	
	inFile.close();
}

TSScene* SceneSemGraph::covertToTSScene(unordered_map<string, Model*> &models)
{
	TSScene* newScene = new TSScene(models, m_metaScene);

	return newScene;
}


