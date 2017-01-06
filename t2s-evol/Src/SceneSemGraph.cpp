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

SceneSemGraph::SceneSemGraph(SceneSemGraph *sg)
	: m_metaScene(sg->m_metaScene),
	m_objectGraphNodeIdToModelSceneIdMap(sg->m_objectGraphNodeIdToModelSceneIdMap),
	m_modelNum(sg->m_modelNum)
{
	m_nodeNum = sg->m_nodeNum;
	m_edgeNum = sg->m_edgeNum;

	m_nodes = sg->m_nodes;
	m_edges = sg->m_edges;
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
				}

				currLine = ifs.readLine();
				if (currLine.contains("transform "))
				{
					std::vector<float> transformVec = StringToFloatList(currLine.toStdString(), "transform ");  // transformation vector in stanford scene file is column-wise
					mat4 transMat(transformVec.data());
					transMat = transMat.transpose();
					m_metaScene.m_metaModellList[i].transformation = transMat;					
				}

				currLine = ifs.readLine();
				if (currLine.contains("position "))
				{
					std::vector<float> elementList = StringToFloatList(currLine.toStdString(), "position ");
					vec3 initPos(elementList[0], elementList[1], elementList[2]);
					m_metaScene.m_metaModellList[i].position = TransformPoint(m_metaScene.m_metaModellList[i].transformation, initPos);
				}

				currLine = ifs.readLine();
				if (currLine.contains("frontDir "))
				{
					std::vector<int> dirElementList = StringToIntegerList(currLine.toStdString(), "frontDir ");
					vec3 initFrontDir(dirElementList[0], dirElementList[1], dirElementList[2]);
					m_metaScene.m_metaModellList[i].frontDir = TransformVector(m_metaScene.m_metaModellList[i].transformation, initFrontDir);
					
				}

				currLine = ifs.readLine();
				if (currLine.contains("upDir "))
				{
					std::vector<int> dirElementList = StringToIntegerList(currLine.toStdString(), "upDir ");
					vec3 initUpDir(dirElementList[0], dirElementList[1], dirElementList[2]);
					m_metaScene.m_metaModellList[i].upDir = TransformVector(m_metaScene.m_metaModellList[i].transformation, initUpDir);			
				}
			}
		}

		// load nodes
		int metaModelId = 0;
		if (currLine.contains("nodeNum "))
		{
			int nodeNum = StringToIntegerList(currLine.toStdString(), "nodeNum ")[0];
			for (int i = 0; i < nodeNum; i++)
			{
				currLine = ifs.readLine();
				std::vector<std::string> parts = PartitionString(currLine.toStdString(), ",");
				
				// object node
				if (QString(parts[1].c_str()) == "object")
				{
					addNode(QString(parts[1].c_str()), QString(parts[2].c_str()));
					m_objectGraphNodeIdToModelSceneIdMap[m_nodeNum - 1] = metaModelId;
					metaModelId++;
				}
				else
				{
					addNode(QString(parts[1].c_str()), QString(parts[2].c_str()));
				}
				
			}
		} 

		// load edges
		if (currLine.contains("edgeNum "))
		{
			int edgeNum = StringToIntegerList(currLine.toStdString(), "edgeNum ")[0];
			for (int i = 0; i < edgeNum; i++)
			{
				currLine = ifs.readLine();
				std::vector<int> parts = StringToIntegerList(currLine.toStdString(), "", ",");
				addEdge(parts[1], parts[2]);
			}
		}
	}
	
	inFile.close();
}

TSScene* SceneSemGraph::covertToTSScene(unordered_map<string, Model*> &models, const QString &sceneName)
{
	m_metaScene.m_sceneFileName = sceneName;
	TSScene* newScene = new TSScene(models, m_metaScene);
	newScene->m_ssg = this;

	return newScene;
}

SceneSemGraph* SceneSemGraph::getSubGraph(const vector<int> &nodeList)
{
	SceneSemGraph *subGraph = new SceneSemGraph();

	map<int, int> oldToNewNodeIdMap;
	for (int i = 0; i < nodeList.size(); i++)
	{
		int oldNodeId = nodeList[i];
		oldToNewNodeIdMap[oldNodeId] = i;
	}

	// build graph nodes
	for (int i = 0; i < nodeList.size(); i++)
	{
		SemNode oldNode = m_nodes[nodeList[i]];
		subGraph->addNode(oldNode.nodeType, oldNode.nodeName);
	}	

	// build graph edges
	for (int i = 0; i < m_edgeNum; i++)
	{
		SemEdge oldEdge = m_edges[i];

		if (oldToNewNodeIdMap.find(oldEdge.sourceNodeId) != oldToNewNodeIdMap.end() 
			&& oldToNewNodeIdMap.find(oldEdge.targetNodeId) != oldToNewNodeIdMap.end())
		{
			int newSourceId = oldToNewNodeIdMap[oldEdge.sourceNodeId];
			int newTargetId = oldToNewNodeIdMap[oldEdge.targetNodeId];
			subGraph->addEdge(newSourceId, newTargetId);
		}
	}

	// Debug
	if (subGraph->m_edgeNum %2 != 0 )
	{
		delete subGraph;
		subGraph = NULL;
		return subGraph;
	}

	// set meta scene
	for (int i = 0; i < nodeList.size(); i++)
	{
		int oldNodeId = nodeList[i];

		// non-object node is not saved in the map
		if (m_objectGraphNodeIdToModelSceneIdMap.count(oldNodeId))
		{
			int oldMetaModelId = m_objectGraphNodeIdToModelSceneIdMap[oldNodeId];

			if (oldMetaModelId < m_modelNum)
			{
				subGraph->m_metaScene.m_metaModellList.push_back(m_metaScene.m_metaModellList[oldMetaModelId]);
				subGraph->m_objectGraphNodeIdToModelSceneIdMap[i] = i;
			}
		}
	}

	return subGraph;
}


