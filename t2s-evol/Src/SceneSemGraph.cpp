#include "SceneSemGraph.h"
#include "Model.h"
#include "TSScene.h"
#include "Utility.h"
#include "Headers.h"

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

	// set DB path for the SSG
	if (m_metaScene.m_sceneFormat == "StanfordSceneDatabase" || m_metaScene.m_sceneFormat == "SceneNNConversionOutput")
	{
		m_metaScene.m_sceneDBPath = QString(params::inst()->localSceneDBDirectory.c_str());
		m_metaScene.m_sceneFilePath = m_metaScene.m_sceneDBPath + "/scenes";
		m_metaScene.m_modelRepository = m_metaScene.m_sceneDBPath + "/models";
	}

	int currModelID = -1;
	QString currLine;
	while (!ifs.atEnd() && !currLine.contains("nodeNum"))
	{
		currLine = ifs.readLine();

		//	load model info
		if (currLine.contains("modelCount "))
		{
			m_modelNum = StringToIntegerList(currLine.toStdString(), "modelCount ")[0];
			m_metaScene.m_metaModellList.resize(m_modelNum);
		}

		if (currLine.contains("newModel "))
		{
			currModelID++;

			std::vector<std::string> parts = PartitionString(currLine.toStdString(), " ");
			int modelIndex = StringToInt(parts[1]);

			m_metaScene.m_metaModellList[currModelID].id = modelIndex;
			m_metaScene.m_metaModellList[currModelID].name = parts[2];
			m_metaScene.m_metaModellList[currModelID].path = m_metaScene.m_modelRepository.toStdString() + "/" + parts[2] + ".obj";
		}

		if (currLine.contains("transform "))
		{
			std::vector<float> transformVec = StringToFloatList(currLine.toStdString(), "transform ");  // transformation vector in stanford scene file is column-wise
			mat4 transMat(transformVec.data());
			transMat = transMat.transpose();
			m_metaScene.m_metaModellList[currModelID].transformation = transMat;
		}

		if (currLine.contains("position "))
		{
			// position saves models position in current scene after transformed
			std::vector<float> elementList = StringToFloatList(currLine.toStdString(), "position ");
			vec3 initPos(elementList[0], elementList[1], elementList[2]);
			m_metaScene.m_metaModellList[currModelID].position = TransformPoint(m_metaScene.m_metaModellList[currModelID].transformation, initPos);
		}

		if (currLine.contains("frontDir "))
		{
			// frontDir saves models frontDir in current scene after transformed
			std::vector<int> dirElementList = StringToIntegerList(currLine.toStdString(), "frontDir ");
			vec3 initFrontDir(dirElementList[0], dirElementList[1], dirElementList[2]);
			m_metaScene.m_metaModellList[currModelID].frontDir = TransformVector(m_metaScene.m_metaModellList[currModelID].transformation, initFrontDir);

		}

		if (currLine.contains("upDir "))
		{
			// upDir saves models upDir in current scene after transformed
			std::vector<int> dirElementList = StringToIntegerList(currLine.toStdString(), "upDir ");
			vec3 initUpDir(dirElementList[0], dirElementList[1], dirElementList[2]);
			m_metaScene.m_metaModellList[currModelID].upDir = TransformVector(m_metaScene.m_metaModellList[currModelID].transformation, initUpDir);
		}

		if (currLine.contains("suppPlane "))
		{
			// for support plane, the data saved in ssg is already transformed
			std::vector<float> floatElementList = StringToFloatList(currLine.toStdString(), "suppPlane ");
			std::vector<vec3> corners(4); 
			for (int v = 0; v < 4; v++)
			{
				corners[v] = vec3(floatElementList[3 * v], floatElementList[3 * v + 1], floatElementList[3 * v + 2]);
			}

			// for support plane, we use save the original data and do not transform it now
			m_metaScene.m_metaModellList[currModelID].suppPlane = SuppPlane(corners);
		}

		if (currLine.contains("parentPlaneUVH "))
		{
			std::vector<float> floatElementList = StringToFloatList(currLine.toStdString(), "parentPlaneUVH ");
			vec3 uvh(floatElementList[0], floatElementList[1], floatElementList[2]);

			m_metaScene.m_metaModellList[currModelID].parentPlaneUVH = uvh;
		}
	}

	//	load nodes
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
				if (parts.size() > 2)
				{
					addNode(QString(parts[1].c_str()), QString(parts[2].c_str()));
				}
				else
				{
					addNode(QString(parts[1].c_str()), "noname");
				}

				m_objectGraphNodeIdToModelSceneIdMap[m_nodeNum - 1] = metaModelId;
				metaModelId++;
			}
			else
			{
				addNode(QString(parts[1].c_str()), QString(parts[2].c_str()));
			}
		}
	}

	currLine = ifs.readLine();

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

	inFile.close();
}

TSScene* SceneSemGraph::covertToTSScene(unordered_map<string, Model*> &models, const QString &sceneName)
{
	m_metaScene.m_sceneFileName = sceneName;
	TSScene* newScene = new TSScene(models, m_metaScene);
	newScene->m_ssg = this;

	return newScene;
}

SceneSemGraph* SceneSemGraph::getSubGraph(const vector<int> &nodeList, bool useContext /* = false*/)
{
	SceneSemGraph *subGraph = new SceneSemGraph();

	map<int, int> oldToNewNodeIdMap;

	// build graph nodes
	for (int i = 0; i < nodeList.size(); i++)
	{
		int oldNodeId = nodeList[i];
		oldToNewNodeIdMap[oldNodeId] = i;

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

	// enrich subgraph with context
	std::vector<int> enrichedNodeList = nodeList;
	if (useContext)
	{
		// add support parent
		for (int i = 0; i < nodeList.size(); i++)
		{
			int oldNodeId = nodeList[i];

			SemNode &oldNode = m_nodes[oldNodeId];

			if (oldNode.nodeType == "object")
			{
				std::vector<int> outNodeList = oldNode.outEdgeNodeList; // find its support node

				// find support node
				for (int j = 0; j < outNodeList.size(); j++)
				{
					int outNodeId = outNodeList[j];  // id of support node

					// if there is support node missing
					if (!oldToNewNodeIdMap.count(outNodeId) && m_nodes[outNodeId].nodeName.contains("support"))
					{
						std::vector<int> suppParentIdList = m_nodes[outNodeId].outEdgeNodeList; // should only have one support parent

						// find whether support parent is room
						bool parentIsRoom = false;
						for (int k = 0; k < suppParentIdList.size(); k++)
						{
							int suppParentNodeId = suppParentIdList[k];
							if (m_nodes[suppParentNodeId].nodeName.contains("room"))
							{
								parentIsRoom = true;
								break;
							}
						}

						if (parentIsRoom) break;

						// add support node
						subGraph->addNode(m_nodes[outNodeId].nodeType, m_nodes[outNodeId].nodeName);
						int currNodeId = subGraph->m_nodeNum - 1;
						oldToNewNodeIdMap[outNodeId] = currNodeId;
						//subGraph->addEdge(currNodeId, oldToNewNodeIdMap[oldNodeId]);
						subGraph->addEdge(oldToNewNodeIdMap[oldNodeId], currNodeId); // e.g., (tv, support)

						enrichedNodeList.push_back(outNodeId);

						// add support parent
						for (int k = 0; k < suppParentIdList.size(); k++)
						{
							int suppParentNodeId = suppParentIdList[k];

							if (!oldToNewNodeIdMap.count(suppParentNodeId) && m_nodes[suppParentNodeId].nodeType == "object")
							{
								subGraph->addNode(m_nodes[suppParentNodeId].nodeType, m_nodes[suppParentNodeId].nodeName);
								int currNodeId = subGraph->m_nodeNum - 1;
								oldToNewNodeIdMap[suppParentNodeId] = currNodeId;
								enrichedNodeList.push_back(suppParentNodeId);

								//subGraph->addEdge(currNodeId, oldToNewNodeIdMap[outNodeId]);
								subGraph->addEdge(oldToNewNodeIdMap[outNodeId], currNodeId); // e.g. (support, tv_stand)

								subGraph->m_nodes[currNodeId].isInferredObj = true;
								subGraph->m_nodes[currNodeId].inferRefObjId = oldToNewNodeIdMap[oldNodeId];
							}
						}
					}
				}
			}
		}

		// add high co-occur objects with probability
	}


	// Debug
	if (subGraph->m_edgeNum % 2 != 0)
	{
		delete subGraph;
		subGraph = NULL;
		return subGraph;
	}

	// set meta scene
	int sceneId = 0;
	for (int i = 0; i < enrichedNodeList.size(); i++)
	{
		int oldNodeId = enrichedNodeList[i];

		// non-object node is not saved in the map
		if (m_objectGraphNodeIdToModelSceneIdMap.count(oldNodeId))
		{
			int oldMetaModelId = m_objectGraphNodeIdToModelSceneIdMap[oldNodeId];

			if (oldMetaModelId < m_modelNum)
			{
				subGraph->m_metaScene.m_metaModellList.push_back(m_metaScene.m_metaModellList[oldMetaModelId]);
				int currNodeId = oldToNewNodeIdMap[oldNodeId];
				subGraph->m_objectGraphNodeIdToModelSceneIdMap[currNodeId] = sceneId;
				sceneId++;
			}
		}
	}

	return subGraph;
}

int SceneSemGraph::findParentNodeId(int modelId)
{
	int currModelNodeId;

	// find graph node id w.r.t to the model id
	for (auto iter = m_objectGraphNodeIdToModelSceneIdMap.begin(); iter != m_objectGraphNodeIdToModelSceneIdMap.end(); iter++)
	{
		if (iter->second == modelId)
		{
			currModelNodeId = iter->first;
			break;
		}
	}

	int relationNodeId = m_nodes[currModelNodeId].outEdgeNodeList[0]; // monitor -> on

	int refModelNodeId = m_nodes[refModelNodeId].outEdgeNodeList[0]; // on -> desk

	int refModelId = m_objectGraphNodeIdToModelSceneIdMap[refModelNodeId];

	return refModelId;
}


