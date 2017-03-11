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
	m_objectGraphNodeToModelListIdMap(sg->m_objectGraphNodeToModelListIdMap),
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
		m_metaScene.m_sceneDBPath = toQString(params::inst()->localSceneDBDirectory);
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
			m_metaScene.m_metaModellList[currModelID].suppPlane.m_sceneMetric = 0.0254;
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
			if (toQString(parts[1]) == "object")
			{
				if (parts.size() > 2)
				{
					addNode(toQString(parts[1]), toQString(parts[2]));

					// set model catgory in meta model
					m_metaScene.m_metaModellList[metaModelId].catName = (parts[2].c_str());
				}
				else
				{
					addNode(toQString(parts[1]), "noname");
				}

				m_objectGraphNodeToModelListIdMap[m_nodeNum - 1] = metaModelId;
				metaModelId++;
			}
			else
			{
				addNode(toQString(parts[1]), toQString(parts[2]));
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

TSScene* SceneSemGraph::covertToTSScene(unordered_map<string, Model*> &models)
{
	TSScene* newScene = new TSScene(models, m_metaScene);
	newScene->m_ssg = this;

	return newScene;
}

SceneSemGraph* SceneSemGraph::getSubGraph(const vector<int> &nodeList, bool useContext /* = false*/)
{
	SceneSemGraph *subGraph = new SceneSemGraph();

	std::vector<int> enrichedNodeList = nodeList;

	m_dbNodeToSubNodeMap.clear();

	// build graph nodes
	int currSubSSGNodeNum = 0;
	for (int i = 0; i < nodeList.size(); i++)
	{
		int oldNodeId = nodeList[i];
		m_dbNodeToSubNodeMap[oldNodeId] = currSubSSGNodeNum;

		SemNode &oldNode = m_nodes[oldNodeId];
		subGraph->addNode(oldNode);
		currSubSSGNodeNum++;

		if (oldNode.nodeType == "group_attribute")
		{
			if (!oldNode.outEdgeNodeList.empty())
			{
				int refNodeId = oldNode.outEdgeNodeList[0];
				SemNode &refNode = m_nodes[refNodeId]; // desk

				//std::vector<int> inNodeList = refNode.inEdgeNodeList;

				//for (int r = 0; r < inNodeList.size(); r++)
				//{
				//	int relationId = inNodeList[r];

				//	SemNode &relationNode = m_nodes[relationId];

				//	if (relationNode.nodeName == "vert_support")
				//	{
				//		// add all support children
				//		int childId = relationNode.inEdgeNodeList[0];
				//		SemNode &childNode = m_nodes[childId];
				//		double insertProb = GenRandomDouble(0, 1);

				//		if (insertProb > 0.3)
				//		{
				//			subGraph->addNode(relationNode.nodeType, relationNode.nodeName);
				//			oldToNewNodeIdMap[relationId] = currSubSSGNodeNum;							
				//			enrichedNodeList.push_back(relationId);
				//			currSubSSGNodeNum++;

				//			subGraph->addNode(childNode.nodeType, childNode.nodeName);
				//			oldToNewNodeIdMap[childId] = currSubSSGNodeNum;
				//			enrichedNodeList.push_back(childId);
				//			currSubSSGNodeNum++;
				//		}
				//	}
				//}
				std::vector<int> inNodeList  = oldNode.inEdgeNodeList;
				for (int a = 0; a < inNodeList.size(); a++)
				{
					int actNodeId = inNodeList[a];
					SemNode &actNode = m_nodes[actNodeId];
					double insertProb = GenRandomDouble(0, 1);

					//if (insertProb > 0.3)
					{
						subGraph->addNode(actNode);
						m_dbNodeToSubNodeMap[actNodeId] = currSubSSGNodeNum;
						enrichedNodeList.push_back(actNodeId);
						currSubSSGNodeNum++;

						actNode.isAnnotated = true;
					}
				}
			}
		}
	}


	// build graph edges
	for (int i = 0; i < m_edgeNum; i++)
	{
		SemEdge oldEdge = m_edges[i];

		if (m_dbNodeToSubNodeMap.find(oldEdge.sourceNodeId) != m_dbNodeToSubNodeMap.end()
			&& m_dbNodeToSubNodeMap.find(oldEdge.targetNodeId) != m_dbNodeToSubNodeMap.end())
		{
			int newSourceId = m_dbNodeToSubNodeMap[oldEdge.sourceNodeId];
			int newTargetId = m_dbNodeToSubNodeMap[oldEdge.targetNodeId];
			subGraph->addEdge(newSourceId, newTargetId);
		}
	}

	// enrich subgraph with context
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
					if (!m_dbNodeToSubNodeMap.count(outNodeId) && m_nodes[outNodeId].nodeName.contains("support"))
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

						// add support node; TEMP, only add the support node for tv now
						if (oldNode.nodeName == "tv" || oldNode.nodeName == "printer" || oldNode.nodeName == "books")
						{
							subGraph->addNode(m_nodes[outNodeId]);
							int currNodeId = subGraph->m_nodeNum - 1;
							m_dbNodeToSubNodeMap[outNodeId] = currNodeId;
							//subGraph->addEdge(currNodeId, oldToNewNodeIdMap[oldNodeId]);
							subGraph->addEdge(m_dbNodeToSubNodeMap[oldNodeId], currNodeId); // e.g., (tv, support)

							enrichedNodeList.push_back(outNodeId);

							// add support parent
							for (int k = 0; k < suppParentIdList.size(); k++)
							{
								int suppParentNodeId = suppParentIdList[k];

								if (!m_dbNodeToSubNodeMap.count(suppParentNodeId) && m_nodes[suppParentNodeId].nodeType == "object")
								{
									subGraph->addNode(m_nodes[suppParentNodeId]);
									int currNodeId = subGraph->m_nodeNum - 1;
									subGraph->m_nodes[currNodeId].inferedType = SemNode::InferBySupport;
									subGraph->m_nodes[currNodeId].isInferred = true;
									subGraph->m_nodes[currNodeId].inferRefNodeId = m_dbNodeToSubNodeMap[oldNodeId];

									m_dbNodeToSubNodeMap[suppParentNodeId] = currNodeId;
									enrichedNodeList.push_back(suppParentNodeId);

									//subGraph->addEdge(currNodeId, oldToNewNodeIdMap[outNodeId]);
									subGraph->addEdge(m_dbNodeToSubNodeMap[outNodeId], currNodeId); // e.g. (support, tv_stand)
								}
							}
						}
					}
				}
			}
		}

		// add high co-occur objects with probability
	}

	// set meta scene
	int modelInSceneId = 0;
	for (int i = 0; i < enrichedNodeList.size(); i++)
	{
		int oldNodeId = enrichedNodeList[i];

		// non-object node is not saved in the map
		if (m_objectGraphNodeToModelListIdMap.count(oldNodeId))
		{
			int oldMetaModelId = m_objectGraphNodeToModelListIdMap[oldNodeId];

			if (oldMetaModelId < m_modelNum)
			{
				m_metaScene.m_metaModellList[oldMetaModelId].isSelected = m_nodes[oldNodeId].isAnnotated;
				subGraph->m_metaScene.m_metaModellList.push_back(m_metaScene.m_metaModellList[oldMetaModelId]);
				int currNodeId = m_dbNodeToSubNodeMap[oldNodeId];
				subGraph->m_objectGraphNodeToModelListIdMap[currNodeId] = modelInSceneId;
				modelInSceneId++;
			}
		}
	}

	subGraph->m_metaScene.m_sceneFileName = m_metaScene.m_sceneFileName;

	return subGraph;
}

int SceneSemGraph::findParentNodeId(int modelId)
{
	int currNodeId = getNodeIdWithModelId(modelId);

	if (currNodeId == -1 || m_nodes[currNodeId].nodeName == "chair" || m_nodes[currNodeId].nodeName == "desk" 
		|| m_nodes[currNodeId].nodeName.contains("cabinet") || m_nodes[currNodeId].nodeName.contains("couch"))
	{
		return -1;
	}

	if (m_nodes[currNodeId].outEdgeNodeList.empty())
	{
		// try to find the potention parent node in current graph
		for (int i = 0; i < m_nodes.size(); i++)
		{
			if (m_nodes[i].nodeName == "desk" || m_nodes[i].nodeName == "couch"
				|| m_nodes[i].nodeName == "bookcase" || m_nodes[i].nodeName.contains("cabinet")
				|| m_nodes[i].nodeName == "bed")
			{
				return i;
			}
		}


		return -1;
	}
	else
	{
		int relationNodeId = m_nodes[currNodeId].outEdgeNodeList[0]; // monitor -> on

		if (m_nodes[relationNodeId].outEdgeNodeList.empty())
		{
			return -1;
		}
		else
		{
			int refNodeId = m_nodes[relationNodeId].outEdgeNodeList[0]; // on -> desk

			return refNodeId;
		}
	}
}

int SceneSemGraph::getNodeIdWithModelId(int modelId)
{
	int currNodeId = -1;

	// find graph node id w.r.t to the model id
	for (auto iter = m_objectGraphNodeToModelListIdMap.begin(); iter != m_objectGraphNodeToModelListIdMap.end(); iter++)
	{
		if (iter->second == modelId)
		{
			currNodeId = iter->first;
			break;
		}
	}

	return currNodeId;
}

void SceneSemGraph::mergeWithMatchedSSG(SceneSemGraph *matchedSg, std::map<int, int> &matchToNewUserSsgNodeMap)
{
	// insert nodes and edges
	this->mergeWithGraph(matchedSg, matchToNewUserSsgNodeMap);


	// insert unaligned objects to meta model list
	for (int mi = 0; mi < matchedSg->m_nodeNum; mi++)
	{
		SemNode& matchedSgNode = matchedSg->m_nodes[mi];
		if (!matchedSgNode.isAligned && matchedSgNode.nodeType == "object")
		{
			int mModelId = matchedSg->m_objectGraphNodeToModelListIdMap[mi];
			MetaModel modelToInsert = matchedSg->m_metaScene.m_metaModellList[mModelId];
			this->m_metaScene.m_metaModellList.push_back(modelToInsert);

			int currMetaModelNum = this->m_metaScene.m_metaModellList.size();
			int ci = matchToNewUserSsgNodeMap[mi];
			this->m_objectGraphNodeToModelListIdMap[ci] = currMetaModelNum - 1;
		}
	}
}


