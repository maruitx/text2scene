#include "SceneSemGraph.h"
#include "RelationModelManager.h"
#include "RelationModel.h"
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
	m_allSynthNodesInited = false;
}

SceneSemGraph::SceneSemGraph(SceneSemGraph *sg)
	: m_metaScene(sg->m_metaScene),
	m_graphNodeToModelListIdMap(sg->m_graphNodeToModelListIdMap),
	m_modelNum(sg->m_modelNum)
{
	m_nodeNum = sg->m_nodeNum;
	m_edgeNum = sg->m_edgeNum;

	m_nodes = sg->m_nodes;
	m_edges = sg->m_edges;

	m_allSynthNodesInited = false;
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

				m_graphNodeToModelListIdMap[m_nodeNum - 1] = metaModelId;
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

	parseNodeNeighbors();

	inFile.close();
}

TSScene* SceneSemGraph::covertToTSScene(unordered_map<string, Model*> &models)
{
	m_modelNum = m_metaScene.m_metaModellList.size();
	buildSupportHierarchy();

	TSScene* newScene = new TSScene(models, this);

	return newScene;
}

void SceneSemGraph::buildSupportHierarchy()
{
	m_levelOfObjs.clear();
	m_parentOfModel.clear();
	m_childListOfModel.clear();


	std::vector<int> isNodeProcessed(m_nodes.size(), 0);
	std::vector<int> newLevelOfObjIds;

	for (int i = 0; i < m_nodes.size(); i++)
	{
		SemNode &sgNode = m_nodes[i];

		if (sgNode.nodeType == "object" && isBaseLevelObj(i))
		{
			int modelId = m_graphNodeToModelListIdMap[i];
			newLevelOfObjIds.push_back(modelId);

			m_parentOfModel[modelId] = -1;
			isNodeProcessed[i] = 1;
		}
	}

	m_levelOfObjs.push_back(newLevelOfObjIds);

	if (m_modelNum == 1) return;

	int passNum = 3;
	for (int p=0; p < passNum; p++)
	{
		std::vector<int> lastLevelOfObjIds = m_levelOfObjs.back();
		std::vector<int> newLevelOfObjIds;

		for (int ni = 0; ni < m_nodes.size(); ni++)
		{
			SemNode &sgNode = m_nodes[ni];

			if (sgNode.nodeType == "object" && !isNodeProcessed[ni])
			{
				int modelId = m_graphNodeToModelListIdMap[ni];

				// find relation node
				for (int ri = 0; ri < sgNode.outEdgeNodeList.size(); ri++)
				{
					int relationNodeId = sgNode.outEdgeNodeList[ri];
					SemNode &relNode = m_nodes[relationNodeId];

					// find anchor obj
					if (relNode.nodeName.contains("support"))
					{
						int anchorNodeId = relNode.anchorNodeList[0];
						int anchorModelId = m_graphNodeToModelListIdMap[anchorNodeId];
						auto iter = std::find(lastLevelOfObjIds.begin(), lastLevelOfObjIds.end(), anchorModelId);

						if (iter != lastLevelOfObjIds.end())
						{
							newLevelOfObjIds.push_back(modelId);
							m_parentOfModel[modelId] = anchorModelId;
							m_childListOfModel[anchorModelId].push_back(modelId);
							isNodeProcessed[ni] = 1;
						}
					}
				}
			}
		}

		if (!newLevelOfObjIds.empty())
		{
			m_levelOfObjs.push_back(newLevelOfObjIds);
		}
	}

	// collect the remaining objs if there is any
	newLevelOfObjIds.clear();
	for (int ni = 0; ni < m_nodes.size(); ni++)
	{
		SemNode &sgNode = m_nodes[ni];

		if (sgNode.nodeType == "object" && isNodeProcessed[ni] == 0)
		{
			int modelId = m_graphNodeToModelListIdMap[ni];
			newLevelOfObjIds.push_back(modelId);
		}
	}

	if (!newLevelOfObjIds.empty())
	{
		m_levelOfObjs.push_back(newLevelOfObjIds);
	}
}

bool SceneSemGraph::isBaseLevelObj(int nodeId)
{
	QString objName = m_nodes[nodeId].nodeName;

	SemNode &sgNode = m_nodes[nodeId];

	// if no support rel node exist
	if (sgNode.outEdgeNodeList.empty())
	{
		return true;
	}
	else 
	{
		for (int i=0; i < sgNode.outEdgeNodeList.size(); i++)
		{
			int relationNodeId = sgNode.outEdgeNodeList[i];
			SemNode &relNode = m_nodes[relationNodeId];

			if (relNode.nodeName.contains("support"))
			{
				return false;
			}
		}
	}

	return true;
}

std::vector<int> SceneSemGraph::findExistingInstanceIds(const QString &catName)
{
	std::vector<int> ids;
	for (int i=0; i< m_metaScene.m_metaModellList.size(); i++)
	{
		MetaModel &md = m_metaScene.m_metaModellList[i];

		if (toQString(md.catName) == catName)
		{
			ids.push_back(i);
		}
	}

	return ids;
}

SceneSemGraph* SceneSemGraph::getSubGraph(const vector<int> &nodeList, RelationModelManager *relManager, bool useContext /* = false*/)
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

		// add nodes in a group
		if (oldNode.nodeType == "group_relation" && !oldNode.anchorNodeList.empty() && oldNode.matchingStatus == SemNode::ExplicitNode)
		{
			int anchorObjId = oldNode.anchorNodeList[0];
			SemNode &anchorNode = m_nodes[anchorObjId];

			QString groupKey = oldNode.nodeName + "_" + anchorNode.nodeName;

			GroupRelationModel *groupModel;
			if (relManager->m_groupRelModels.count(groupKey))
			{
				groupModel = relManager->m_groupRelModels[groupKey];
			}

			std::vector<int> actNodeList = oldNode.activeNodeList;

			for (int a = 0; a < actNodeList.size(); a++)
			{
				int actNodeId = actNodeList[a];
				SemNode &actNode = m_nodes[actNodeId];
				QString occKey =QString("%1_%2").arg(actNode.nodeName).arg(1);  // Temp, extend to multiple instances later

				double randProb = GenRandomDouble(0, 1);

				if (groupModel->m_occurModels.count(occKey))
				{
					double probTh = groupModel->m_occurModels[occKey]->m_occurProb;

					if (probTh > randProb)
					{
						subGraph->addNode(actNode);
						m_dbNodeToSubNodeMap[actNodeId] = currSubSSGNodeNum;
						enrichedNodeList.push_back(actNodeId);
						currSubSSGNodeNum++;

						actNode.isAnnotated = true;
						actNode.isAligned = false;
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
		if (m_graphNodeToModelListIdMap.count(oldNodeId))
		{
			int oldMetaModelId = m_graphNodeToModelListIdMap[oldNodeId];

			if (oldMetaModelId < m_modelNum)
			{
				m_metaScene.m_metaModellList[oldMetaModelId].isSelected = m_nodes[oldNodeId].isAnnotated;
				subGraph->m_metaScene.m_metaModellList.push_back(m_metaScene.m_metaModellList[oldMetaModelId]);
				int currNodeId = m_dbNodeToSubNodeMap[oldNodeId];
				subGraph->m_graphNodeToModelListIdMap[currNodeId] = modelInSceneId;
				modelInSceneId++;
			}
		}
	}

	subGraph->m_metaScene.m_sceneFileName = m_metaScene.m_sceneFileName;

	subGraph->parseNodeNeighbors();

	return subGraph;
}

int SceneSemGraph::findParentNodeIdForModel(int modelId)
{
	int currNodeId = getNodeIdWithModelId(modelId);
	return findParentNodeIdForNode(currNodeId);
}

int SceneSemGraph::findParentNodeIdForNode(int currNodeId)
{
	if (currNodeId == -1 || m_nodes[currNodeId].nodeName == "chair" || m_nodes[currNodeId].nodeName == "desk"
		|| m_nodes[currNodeId].nodeName.contains("cabinet") || m_nodes[currNodeId].nodeName.contains("couch")
		|| m_nodes[currNodeId].nodeName.contains("shelf"))
	{
		return -1;
	}

	if (m_nodes[currNodeId].outEdgeNodeList.empty())
	{
		return -1;
	}
	else
	{
		int relationNodeId = m_nodes[currNodeId].outEdgeNodeList[0]; // monitor -> on
		SemNode &relNode = m_nodes[relationNodeId];
		if (!m_nodes[relationNodeId].outEdgeNodeList.empty() && relNode.nodeName.contains("support"))
		{
			int refNodeId = m_nodes[relationNodeId].outEdgeNodeList[0]; // on -> desk
			return refNodeId;			
		}
		else
		{
			return -1;
		}
	}
}

int SceneSemGraph::getNodeIdWithModelId(int modelId)
{
	int currNodeId = -1;

	// find graph node id w.r.t to the model id
	for (auto iter = m_graphNodeToModelListIdMap.begin(); iter != m_graphNodeToModelListIdMap.end(); iter++)
	{
		if (iter->second == modelId && iter->first != -1)
		{
			currNodeId = iter->first;
			break;
		}
	}

	return currNodeId;
}

MetaModel& SceneSemGraph::getModelWithNodeId(int nodeId)
{
	int modelId = m_graphNodeToModelListIdMap[nodeId];
	return m_metaScene.m_metaModellList[modelId];
}

void SceneSemGraph::mergeWithMatchedSSG(SceneSemGraph *matchedSg)
{
	// insert nodes and edges
	this->mergeWithGraph(matchedSg);


	// insert unaligned objects to meta model list
	for (int mi = 0; mi < matchedSg->m_nodeNum; mi++)
	{
		SemNode& matchedSgNode = matchedSg->m_nodes[mi];
		if (!matchedSgNode.isAligned && matchedSgNode.nodeType == "object")
		{
			int mModelId = matchedSg->m_graphNodeToModelListIdMap[mi];
			MetaModel modelToInsert = matchedSg->m_metaScene.m_metaModellList[mModelId];
			this->m_metaScene.m_metaModellList.push_back(modelToInsert);

			int currMetaModelNum = this->m_metaScene.m_metaModellList.size();
			int ci = matchedSg->m_nodeAlignMap[mi];
			this->m_graphNodeToModelListIdMap[ci] = currMetaModelNum - 1;
		}
	}

	m_modelNum = m_metaScene.m_metaModellList.size();
}

bool SceneSemGraph::findRefNodeForRelationNode(const SemNode &sgNode, int &refNodeId, int &activeNodeId)
{
	// edge dir: (active, relation), (relation, reference)
	int actNodeId = sgNode.activeNodeList[0];  // active
	int anchorNodeId = sgNode.anchorNodeList[0]; // reference


	// find the reference node
	if (this->m_nodes[actNodeId].isAligned && !this->m_nodes[anchorNodeId].isAligned)
	{
		refNodeId = actNodeId;
		activeNodeId = anchorNodeId;

		return true;
	}
	else if (this->m_nodes[anchorNodeId].isAligned && !this->m_nodes[actNodeId].isAligned)
	{
		refNodeId = anchorNodeId;
		activeNodeId = actNodeId;

		return true;
	}
	// if no reference node is aligned, just use the ref node in matchedSg, and align to it
	else
	{
		return false;
	}
}


