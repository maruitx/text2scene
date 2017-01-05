#include "SceneGenerator.h"
#include "SceneSemGraphManager.h"
#include "SceneSemGraph.h"
#include "TextSemGraph.h"
#include "SemGraphMatcher.h"
#include "TSScene.h"

SceneGenerator::SceneGenerator(const QString &sceneDBPath, unordered_map<string, Model*> &models)
	:m_models(models)
{
	m_sceneSemGraphManager = new SceneSemGraphManager(sceneDBPath);
	m_semanticGraphMatcher = new SemGraphMatcher(m_sceneSemGraphManager);
}

SceneGenerator::~SceneGenerator()
{

}

void SceneGenerator::updateCurrentTextGraph(TextSemGraph *tsg)
{
	m_semanticGraphMatcher->updateCurrentTextSemGraph(tsg);
}

void SceneGenerator::updateCurrentTSScene(TSScene *ts)
{
	m_currTSScene = ts;
}

std::vector<TSScene*> SceneGenerator::generateTSScenes(int num)
{
	//vector<SceneSemGraph*> matchedSSGs = m_semanticGraphMatcher->testMatchTSGWithSSGs(num);
	std::vector<SceneSemGraph*> matchedSSGs = m_semanticGraphMatcher->matchTSGWithSSGs(num);

	std::vector<TSScene*> tsscenes;

	for (int i = 0; i < matchedSSGs.size(); i++)
	{
		QString sceneName = QString("Preview %1").arg(i);

		SceneSemGraph *newSSG = semanticAlignToCurrTSScene(matchedSSGs[i]);

		//TSScene *s = matchedSSGs[i]->covertToTSScene(m_models, sceneName);
		TSScene *s = newSSG->covertToTSScene(m_models, sceneName);
		tsscenes.push_back(s);
	}

	return tsscenes;
}

SceneSemGraph* SceneGenerator::semanticAlignToCurrTSScene(SceneSemGraph *matchedSg)
{
	SceneSemGraph *newSg;
	SceneSemGraph *currSg = m_currTSScene->m_ssg;

	// unalign nodes of matched ssg
	for (int i = 0; i < matchedSg->m_nodeNum; i++)
	{
		matchedSg->m_nodes[i].isAligned = false;
	}

	if (currSg == NULL)
	{
		newSg = new SceneSemGraph(matchedSg);
		return newSg;
	}

	// unalign nodes of current ssg
	for (int i = 0; i < currSg->m_nodeNum; i++)
	{
		currSg->m_nodes[i].isAligned = false;
	}

	// copy from current sg
	newSg = new SceneSemGraph(currSg);

	std::map<int, int> mapFromMatchToCurrentNodeId;

	// first match object node
	for (int mi = 0; mi < matchedSg->m_nodeNum; mi++)
	{
		SemNode& matchedSgNode = matchedSg->m_nodes[mi];

		if (matchedSgNode.nodeType == "object")
		{
			for (int ci = 0; ci < newSg->m_nodeNum; ci++)
			{
				SemNode& currSgNode = newSg->m_nodes[ci];
				// skip the aligned edges
				if (!currSgNode.isAligned && currSgNode.nodeType == "object")
				{
					if (matchedSgNode.nodeName == currSgNode.nodeName)
					{
						matchedSgNode.isAligned = true;
						currSgNode.isAligned = true;
						mapFromMatchToCurrentNodeId[mi] = ci;
					}
				}
			}
		}
	}

	// match pair-wise relationship node
	for (int mi = 0; mi < matchedSg->m_nodeNum; mi++)
	{
		SemNode& matchedSgNode = matchedSg->m_nodes[mi];

		if (matchedSgNode.nodeType == "pairwise_relation")
		{
			int mInNodeId = matchedSgNode.inEdgeNodeList[0];
			int mOutNodeId = matchedSgNode.outEdgeNodeList[0];

			for (int ci = 0; ci < newSg->m_nodeNum; ci++)
			{
				SemNode& currSgNode = newSg->m_nodes[ci];
				if (!currSgNode.isAligned && currSgNode.nodeType == "pairwise_relation")
				{
					if (currSgNode.inEdgeNodeList[0] == mapFromMatchToCurrentNodeId[mInNodeId]
						&& currSgNode.outEdgeNodeList[0] == mapFromMatchToCurrentNodeId[mOutNodeId])
					{
						matchedSgNode.isAligned = true;
						currSgNode.isAligned = true;
						mapFromMatchToCurrentNodeId[mi] = ci;
					}
				}
			}
		}
	}

	// merge matched ssg to current scene ssg
	// insert all unaligned nodes
	for (int mi = 0; mi < matchedSg->m_nodeNum; mi++)
	{
		SemNode& matchedSgNode = matchedSg->m_nodes[mi];
		if (!matchedSgNode.isAligned)
		{
			// update graph
			newSg->addNode(matchedSgNode.nodeType, matchedSgNode.nodeName);
			mapFromMatchToCurrentNodeId[mi] = newSg->m_nodeNum-1;  // node id is the last node's id
		}
	}

	// insert edges from matched ssg if it is not existing in current ssg
	for (int mei = 0; mei < matchedSg->m_edgeNum; mei++)
	{
		SemEdge& matchedSgEdge = matchedSg->m_edges[mei];

		int s = mapFromMatchToCurrentNodeId[matchedSgEdge.sourceNodeId];
		int t = mapFromMatchToCurrentNodeId[matchedSgEdge.targetNodeId];
		if (!newSg->isEdgeExist(s, t))
		{
			newSg->addEdge(s, t);
		}
	}

	// insert unaligned objects to meta model list
	for (int mi = 0; mi < matchedSg->m_nodeNum; mi++)
	{
		SemNode& matchedSgNode = matchedSg->m_nodes[mi];
		if (!matchedSgNode.isAligned && matchedSgNode.nodeType == "object")
		{
			//matchedSg->m_metaScene.m_metaModellList[matchedSgNode.nodeId] = currSg->m_metaScene.m_metaModellList[currSgNode.nodeId];
			int mModelId = matchedSg->m_objectGraphNodeIdToModelSceneIdMap[mi];
			MetaModel modelToInsert = matchedSg->m_metaScene.m_metaModellList[mModelId];
			newSg->m_metaScene.m_metaModellList.push_back(modelToInsert);

			int currMetaModelNum = newSg->m_metaScene.m_metaModellList.size();
			int ci = mapFromMatchToCurrentNodeId[mi];
			newSg->m_objectGraphNodeIdToModelSceneIdMap[ci] = currMetaModelNum-1;
		}
	}

	return newSg;
}

void SceneGenerator::geometryAlignToCurrTSScene()
{

}


