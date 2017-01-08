#include "SceneGenerator.h"
#include "SceneSemGraphManager.h"
#include "SceneSemGraph.h"
#include "TextSemGraph.h"
#include "SemGraphMatcher.h"
#include "TSScene.h"
#include "Utility.h"
#include "Headers.h"

SceneGenerator::SceneGenerator(unordered_map<string, Model*> &models)
	:m_models(models)
{
	m_sceneSemGraphManager = new SceneSemGraphManager();
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
	std::vector<SceneSemGraph*> matchedSSGs = m_semanticGraphMatcher->alignmentTSGWithDatabaseSSGs(num);

	std::vector<TSScene*> tsscenes;

	for (int i = 0; i < matchedSSGs.size(); i++)
	{
		QString sceneName = QString("Preview %1").arg(i);

		SceneSemGraph *newSSG = alignToCurrTSScene(matchedSSGs[i]);
		TSScene *s = newSSG->covertToTSScene(m_models, sceneName);
		tsscenes.push_back(s);
	}

	return tsscenes;
}

SceneSemGraph* SceneGenerator::alignToCurrTSScene(SceneSemGraph *matchedSg)
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

	std::map<int, int> mapFromMatchToNewNodeId;

	// first match object node
	for (int mi = 0; mi < matchedSg->m_nodeNum; mi++)
	{
		SemNode& matchedSgNode = matchedSg->m_nodes[mi];

		if (matchedSgNode.nodeType == "object")
		{
			for (int ci = 0; ci < newSg->m_nodeNum; ci++)
			{
				SemNode& newSgNode = newSg->m_nodes[ci];
				// skip the aligned nodes
				if (!newSgNode.isAligned && newSgNode.nodeType == "object")
				{
					if (matchedSgNode.nodeName == newSgNode.nodeName)
					{
						matchedSgNode.isAligned = true;
						newSgNode.isAligned = true;
						mapFromMatchToNewNodeId[mi] = ci; // save aligned object node map

						break;
					}
				}
			}
		}
	}

	// match pair-wise relationship node
	for (int mi = 0; mi < matchedSg->m_nodeNum; mi++)
	{
		SemNode& matchedSgNode = matchedSg->m_nodes[mi];

		if (matchedSgNode.nodeType == "pairwise_relationship")
		{
			// To test whether in and out node exist
			if (matchedSgNode.inEdgeNodeList.empty() || matchedSgNode.outEdgeNodeList.empty())
			{ 
				break;			
			}

			int mInNodeId = matchedSgNode.inEdgeNodeList[0];
			int mOutNodeId = matchedSgNode.outEdgeNodeList[0];

			// if any object node is not in the aligned map, then break
			if (!mapFromMatchToNewNodeId.count(mInNodeId) || !mapFromMatchToNewNodeId.count(mOutNodeId))
			{
				break;
			}

			for (int ci = 0; ci < newSg->m_nodeNum; ci++)
			{
				SemNode& newSgNode = newSg->m_nodes[ci];

				// skip the aligned nodes
				if (!newSgNode.isAligned && newSgNode.nodeType == "pairwise_relationship")
				{
					if (newSgNode.inEdgeNodeList[0] == mapFromMatchToNewNodeId[mInNodeId]
						&& newSgNode.outEdgeNodeList[0] == mapFromMatchToNewNodeId[mOutNodeId])
					{
						matchedSgNode.isAligned = true;
						newSgNode.isAligned = true;
						mapFromMatchToNewNodeId[mi] = ci;  // save aligned pairwise relationship node map

						break;
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
			mapFromMatchToNewNodeId[mi] = newSg->m_nodeNum-1;  // node id is the last node's id; save inserted node map
		}
	}

	// insert edges from matched ssg if it is not existing in current ssg
	for (int mei = 0; mei < matchedSg->m_edgeNum; mei++)
	{
		SemEdge& matchedSgEdge = matchedSg->m_edges[mei];

		int s = mapFromMatchToNewNodeId[matchedSgEdge.sourceNodeId];
		int t = mapFromMatchToNewNodeId[matchedSgEdge.targetNodeId];
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
			int mModelId = matchedSg->m_objectGraphNodeIdToModelSceneIdMap[mi];
			MetaModel modelToInsert = matchedSg->m_metaScene.m_metaModellList[mModelId];
			newSg->m_metaScene.m_metaModellList.push_back(modelToInsert);

			int currMetaModelNum = newSg->m_metaScene.m_metaModellList.size();
			int ci = mapFromMatchToNewNodeId[mi];
			newSg->m_objectGraphNodeIdToModelSceneIdMap[ci] = currMetaModelNum-1;
		}
	}

	// geometry alignment
	for (int mi = 0; mi < matchedSg->m_nodeNum; mi++)
	{
		SemNode& matchedSgNode = matchedSg->m_nodes[mi];
		if (!matchedSgNode.isAligned && matchedSgNode.nodeType == "pairwise_relationship")
		{
			int mInNodeId = matchedSgNode.inEdgeNodeList[0];
			int mOutNodeId = matchedSgNode.outEdgeNodeList[0];

			int mRefNodeId;
			int mActiveNodeId;

			// find the reference node
			if (matchedSg->m_nodes[mInNodeId].isAligned && !matchedSg->m_nodes[mOutNodeId].isAligned)
			{
				mRefNodeId = mInNodeId;
				mActiveNodeId = mOutNodeId;
			}
			else if (matchedSg->m_nodes[mOutNodeId].isAligned && !matchedSg->m_nodes[mInNodeId].isAligned)
			{
				mRefNodeId = mOutNodeId;
				mActiveNodeId = mInNodeId;
			}
			else
			{
				break;
			}

			int newRefNodeId = mapFromMatchToNewNodeId[mRefNodeId];
			int newActiveNodeId = mapFromMatchToNewNodeId[mActiveNodeId];

			// compute transformation matrix based on the ref nodes
			int mRefModelId = matchedSg->m_objectGraphNodeIdToModelSceneIdMap[mRefNodeId];
			int newRefModeId = newSg->m_objectGraphNodeIdToModelSceneIdMap[newRefNodeId];

			mat4 transMat = computeTransMat(matchedSg->m_metaScene.m_metaModellList[mRefModelId], newSg->m_metaScene.m_metaModellList[newRefModeId]);

			// transform active model
			int newActiveModelId = newSg->m_objectGraphNodeIdToModelSceneIdMap[newActiveNodeId];
			MetaModel &newActiveModel = newSg->m_metaScene.m_metaModellList[newActiveModelId];
			newActiveModel.transformation = transMat*newActiveModel.transformation;
			newActiveModel.position = TransformPoint(transMat, newActiveModel.position);
			newActiveModel.frontDir = TransformVector(transMat, newActiveModel.frontDir);
			newActiveModel.upDir = TransformVector(transMat,newActiveModel.upDir);

			// adjust position of transformed active model
		}
	}

	return newSg;
}

mat4 SceneGenerator::computeTransMat(const MetaModel &fromModel, const MetaModel &toModel)
{
	mat4 rotMat = GetRotationMatrix(fromModel.frontDir, toModel.frontDir);
	mat4 transMat = GetTransformationMat(rotMat, fromModel.position, toModel.position);

	return transMat;
}


