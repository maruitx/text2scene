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
	m_dbSSGId = -1;
	m_alignedQuerySSG = NULL;
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
	m_dbSSGId = -1;
	m_alignedQuerySSG = NULL;
}

SceneSemGraph::~SceneSemGraph()
{
	if (m_alignedQuerySSG)
	{
		delete m_alignedQuerySSG;
	}
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
			// position saves models init position before transformed
			std::vector<float> elementList = StringToFloatList(currLine.toStdString(), "position ");
			m_metaScene.m_metaModellList[currModelID].position = vec3(elementList[0], elementList[1], elementList[2]);
		}

		if (currLine.contains("frontDir "))
		{
			// position saves model OBB bottom center which is already transformed
			std::vector<int> dirElementList = StringToIntegerList(currLine.toStdString(), "frontDir ");
			vec3 initFrontDir(dirElementList[0], dirElementList[1], dirElementList[2]);
			m_metaScene.m_metaModellList[currModelID].frontDir = TransformVector(m_metaScene.m_metaModellList[currModelID].transformation, initFrontDir);
			m_metaScene.m_metaModellList[currModelID].frontDir.normalize();
		}

		if (currLine.contains("upDir "))
		{
			// upDir saves models init upDir before transformed
			std::vector<int> dirElementList = StringToIntegerList(currLine.toStdString(), "upDir ");
			vec3 initUpDir(dirElementList[0], dirElementList[1], dirElementList[2]);
			m_metaScene.m_metaModellList[currModelID].upDir = TransformVector(m_metaScene.m_metaModellList[currModelID].transformation, initUpDir);
			m_metaScene.m_metaModellList[currModelID].upDir.normalize();
		}

		if (currLine.contains("bbTopPlane "))
		{
			// for bbTopPlane plane, the data saved in ssg is already transformed
			std::vector<float> floatElementList = StringToFloatList(currLine.toStdString(), "bbTopPlane ");
			std::vector<vec3> corners(4); 
			for (int v = 0; v < 4; v++)
			{
				corners[v] = vec3(floatElementList[3 * v], floatElementList[3 * v + 1], floatElementList[3 * v + 2]);
			}

			m_metaScene.m_metaModellList[currModelID].bbTopPlane = SuppPlane(corners);
			m_metaScene.m_metaModellList[currModelID].bbTopPlane.m_sceneMetric = 0.0254;
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

void SceneSemGraph::saveGraph(const QString &filename)
{
	QFile outFile(filename);
	QTextStream ofs(&outFile);

	if (outFile.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
	{
		ofs << "StanfordSceneDatabase\n";

		// save scene meta data
		int modelNum = m_metaScene.m_metaModellList.size();
		ofs << "modelCount " << modelNum << "\n";

		for (int i = 0; i < modelNum; i++)
		{
			MetaModel &md = m_metaScene.m_metaModellList[i];
			ofs << "newModel " << i << " " << toQString(md.name) << "\n";
			ofs << "transform " << QString("%1").arg(md.transformation.a11, 0, 'f') << " " << QString("%1").arg(md.transformation.a21, 0, 'f') << " " << QString("%1").arg(md.transformation.a31, 0, 'f') << " " << QString("%1").arg(md.transformation.a41, 0, 'f') << " "
				<< QString("%1").arg(md.transformation.a12, 0, 'f') << " " << QString("%1").arg(md.transformation.a22, 0, 'f') << " " << QString("%1").arg(md.transformation.a32, 0, 'f') << " " << QString("%1").arg(md.transformation.a42, 0, 'f') << " "
				<< QString("%1").arg(md.transformation.a13, 0, 'f') << " " << QString("%1").arg(md.transformation.a23, 0, 'f') << " " << QString("%1").arg(md.transformation.a33, 0, 'f') << " " << QString("%1").arg(md.transformation.a43, 0, 'f') << " "
				<< QString("%1").arg(md.transformation.a14, 0, 'f') << " " << QString("%1").arg(md.transformation.a24, 0, 'f') << " " << QString("%1").arg(md.transformation.a34, 0, 'f') << " " << QString("%1").arg(md.transformation.a44, 0, 'f') << "\n";

			// position saves model OBB bottom center which is already transformed
			ofs << "position " << md.position.x << " " << md.position.y << " " << md.position.z << "\n";

			// frontDir saves models init upDir frontDir transformed
			mat4 invTrans = md.transformation.inverse();
			vec3 initFrontDir = TransformVector(invTrans, md.frontDir);
			initFrontDir.normalize();
			ofs << "frontDir " << initFrontDir.x << " " << initFrontDir.y << " " << initFrontDir.z << "\n";

			// upDir saves models init upDir before transformed
			vec3 initUpDir = TransformVector(invTrans, md.upDir);
			initUpDir.normalize();
			ofs << "upDir " << initUpDir.x << " " << initUpDir.y << " " << initUpDir.z << "\n";

			// for support plane, the data saved in ssg is already transformed
			ofs << "bbTopPlane " << md.bbTopPlane.m_corners[0].x << " " << md.bbTopPlane.m_corners[0].y << " " << md.bbTopPlane.m_corners[0].z << " "
				<< md.bbTopPlane.m_corners[1].x << " " << md.bbTopPlane.m_corners[1].y << " " << md.bbTopPlane.m_corners[1].z << " "
				<< md.bbTopPlane.m_corners[2].x << " " << md.bbTopPlane.m_corners[2].y << " " << md.bbTopPlane.m_corners[2].z << " "
				<< md.bbTopPlane.m_corners[3].x << " " << md.bbTopPlane.m_corners[3].y << " " << md.bbTopPlane.m_corners[3].z << "\n";
			ofs << "parentPlaneUVH " << md.parentPlaneUVH.x << " " << md.parentPlaneUVH.y << " " << md.parentPlaneUVH.z << "\n";
		}

		// save nodes in format: nodeId,nodeType,nodeName,inEdgeNodeList,outEdgeNodeList
		ofs << "nodeNum " << m_nodeNum << "\n";
		for (int i = 0; i < m_nodeNum; i++)
		{
			ofs << i << "," << m_nodes[i].nodeType << "," << m_nodes[i].nodeName << ","
				<< GetIntString(m_nodes[i].inEdgeNodeList, " ") << ","
				<< GetIntString(m_nodes[i].outEdgeNodeList, " ") << "\n";
		}


		// save edges in format: edgeId,startId endId
		ofs << "edgeNum " << m_edgeNum << "\n";
		for (int i = 0; i < m_edgeNum; i++)
		{
			ofs << i << "," << m_edges[i].sourceNodeId << "," << m_edges[i].targetNodeId << "\n";
		}

		outFile.close();

		std::cout << "SceneSemGraph: graph saved.\n";
	}

	else
	{
		std::cout << "SceneSemGraph: fail to save graph .\n";
	}
}

void SceneSemGraph::initMetaModelSuppPlanes(unordered_map<string, Model*> &models)
{
	for (int i = 0; i < m_metaScene.m_metaModellList.size(); i++)
	{
		MetaModel &md = m_metaScene.m_metaModellList[i];
		if (models.count(md.name))
		{
			Model *currModel = models[md.name];
			md.suppPlaneManager = SuppPlaneManager(currModel->m_suppPlaneManager->m_suppPlanes);
			md.suppPlaneManager.transformSuppPlanes(md.transformation);
		}
		else
		{
			md.suppPlaneManager = SuppPlaneManager(toQString(md.name));
			md.suppPlaneManager.transformSuppPlanes(md.transformation);
		}
	}
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

SceneSemGraph* SceneSemGraph::getSubGraph(const vector<int> &nodeList, RelationModelManager *relManager)
{
	SceneSemGraph *subGraph = new SceneSemGraph();

	std::vector<int> objNodeList;

	m_dbNodeToSubNodeMap.clear();

	// build graph nodes
	int currSubSSGNodeNum = 0;
	for (int i = 0; i < nodeList.size(); i++)
	{
		int dbNodeId = nodeList[i];
		m_dbNodeToSubNodeMap[dbNodeId] = currSubSSGNodeNum;

		SemNode &dbNode = m_nodes[dbNodeId];
		subGraph->addNode(dbNode);
		currSubSSGNodeNum++;

		if (dbNode.nodeType == "object")
		{
			objNodeList.push_back(dbNodeId);
		}
	}

	// build graph edges
	for (int i = 0; i < m_edgeNum; i++)
	{
		SemEdge dbEdge = m_edges[i];

		if (m_dbNodeToSubNodeMap.find(dbEdge.sourceNodeId) != m_dbNodeToSubNodeMap.end()
			&& m_dbNodeToSubNodeMap.find(dbEdge.targetNodeId) != m_dbNodeToSubNodeMap.end())
		{
			int newSourceId = m_dbNodeToSubNodeMap[dbEdge.sourceNodeId];
			int newTargetId = m_dbNodeToSubNodeMap[dbEdge.targetNodeId];
			subGraph->addEdge(newSourceId, newTargetId);
		}
	}

	// set meta scene
	int modelInSceneId = 0;
	for (int i = 0; i < objNodeList.size(); i++)
	{
		int dbNodeId = objNodeList[i];

		// non-object node is not saved in the map
		if (m_graphNodeToModelListIdMap.count(dbNodeId))
		{
			int dbMetaModelId = m_graphNodeToModelListIdMap[dbNodeId];

			if (dbMetaModelId < m_modelNum)
			{
				MetaModel &dbMd = m_metaScene.m_metaModellList[dbMetaModelId];
				dbMd.isSelected = m_nodes[dbNodeId].isAnnotated;

				subGraph->m_metaScene.m_metaModellList.push_back(dbMd);
				int currNodeId = m_dbNodeToSubNodeMap[dbNodeId];
				subGraph->m_graphNodeToModelListIdMap[currNodeId] = modelInSceneId;
				modelInSceneId++;
			}
		}
	}

	subGraph->m_metaScene.m_sceneFileName = m_metaScene.m_sceneFileName;

	subGraph->parseNodeNeighbors();

	return subGraph;
}

void SceneSemGraph::restoreMissingSupportNodes()
{
	for (int i=0; i<m_nodes.size(); i++)
	{
		SemNode &relNode = m_nodes[i];

		// onleft, onright, oncenter
		if (relNode.nodeType == "pair_relation" &&
			relNode.nodeName.contains("on")&&!relNode.nodeName.contains("front"))
		{
			int actNodeId = relNode.inEdgeNodeList[0];
			if (!hasSupportNode(actNodeId))
			{
				int anchorNodeId = relNode.outEdgeNodeList[0];
				addNode(SSGNodeType[SemNode::Pair], "vertsupport");
				int currNodeId = m_nodeNum - 1;

				addEdge(actNodeId, currNodeId);
				addEdge(currNodeId, anchorNodeId);
			}
		}

		//// restore for objects in a group relation
		//if (relNode.nodeType == "group_relation")
		//{
		//	for (int t=0; t < relNode.activeNodeList.size(); t++)
		//	{
		//		int actNodeId = relNode.activeNodeList[t];
		//		if (!hasSupportNode(actNodeId))
		//		{
		//			int anchorNodeId = relNode.anchorNodeList[0];
		//			addNode(SSGNodeType[2], "vertsupport");
		//			int currNodeId = m_nodeNum - 1;

		//			addEdge(actNodeId, currNodeId);
		//			addEdge(currNodeId, anchorNodeId);
		//		}
		//	}
		//}
	}

	for (int i=0;  i<m_nodes.size(); i++)
	{
		SemNode &currNode = m_nodes[i];

		if (currNode.nodeType =="object")
		{
			int actParentNodeId = findParentNodeIdForNode(i);
			// try to find potential node in current scene
			if (actParentNodeId == -1)
			{
				actParentNodeId = findPotentialParentNodeIdForNode(i);
				if (actParentNodeId !=-1)
				{
					addNode(SSGNodeType[SemNode::Pair], "vertsupport");
					int currNodeId = m_nodeNum - 1;

					addEdge(i, currNodeId);
					addEdge(currNodeId, actParentNodeId);
				}
			}
		}	
	}

}

bool SceneSemGraph::hasSupportNode(int actNodeId)
{
	SemNode &actNode = m_nodes[actNodeId];

	for (int i=0; i < actNode.outEdgeNodeList.size(); i++)
	{
		int relNodeId = actNode.outEdgeNodeList[i];
		SemNode &relNode = m_nodes[relNodeId];

		if (relNode.nodeName.contains("support"))
		{
			return true;
		}
	}

	return false;
}

int SceneSemGraph::findParentNodeIdForModel(int modelId)
{
	int currNodeId = getNodeIdWithModelId(modelId);
	return findParentNodeIdForNode(currNodeId);
}

int SceneSemGraph::findParentNodeIdForNode(int currNodeId)
{
	if (currNodeId==-1)
	{
		return -1;
	}

	QString currNodeName = m_nodes[currNodeId].nodeName;
	if (currNodeName == "chair" || currNodeName == "desk" || currNodeName == "table"
		|| currNodeName.contains("cabinet") || currNodeName.contains("couch")
		|| currNodeName.contains("shelf"))
	{
		return -1;
	}

	if (m_nodes[currNodeId].outEdgeNodeList.empty())
	{
		return -1;
	}
	else
	{
		for (int i=0; i < m_nodes[currNodeId].outEdgeNodeList.size(); i++)
		{
			int relationNodeId = m_nodes[currNodeId].outEdgeNodeList[i]; // monitor -> on
			SemNode &relNode = m_nodes[relationNodeId];
			if (!m_nodes[relationNodeId].outEdgeNodeList.empty() && relNode.nodeName.contains("support"))
			{
				int refNodeId = m_nodes[relationNodeId].outEdgeNodeList[0]; // on -> desk
				return refNodeId;
			}
		}
		return -1;	
	}
}

int SceneSemGraph::findPotentialParentNodeIdForNode(int nodeId)
{
	QString currNodeName = m_nodes[nodeId].nodeName;

	for (int i=0; i < m_nodes.size(); i++)
	{
		SemNode &sgNode = m_nodes[i];

		if ((currNodeName == "knife" || currNodeName=="fork" || currNodeName == "plate")
			&&sgNode.nodeName == "table")
		{
			return i;
		}
	}

	return -1;
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
	insertUnAlignedObjsFromGraph(matchedSg);
}

void SceneSemGraph::insertUnAlignedObjsFromGraph(SceneSemGraph *matchedSg)
{
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


