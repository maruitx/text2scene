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

	// set status for object already in current ssg
	for (int i = 0; i < currSg->m_metaScene.m_metaModellList.size(); i++)
	{
		currSg->m_metaScene.m_metaModellList[i].isAlreadyPlaced = true;
	}

	// copy from current sg
	newSg = new SceneSemGraph(currSg);

	m_mapFromMatchToNewNodeId.clear();

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
						m_mapFromMatchToNewNodeId[mi] = ci; // save aligned object node map

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

			// edge dir: (active, relation), (relation, reference)
			int mInNodeId = matchedSgNode.inEdgeNodeList[0]; // active Node
			int mOutNodeId = matchedSgNode.outEdgeNodeList[0]; // reference Node

			// if any object node is not in the aligned map, then break
			if (!m_mapFromMatchToNewNodeId.count(mInNodeId) || !m_mapFromMatchToNewNodeId.count(mOutNodeId))
			{
				break;
			}

			for (int ci = 0; ci < newSg->m_nodeNum; ci++)
			{
				SemNode& newSgNode = newSg->m_nodes[ci];

				// skip the aligned nodes
				if (!newSgNode.isAligned && newSgNode.nodeType == "pairwise_relationship")
				{
					if (newSgNode.inEdgeNodeList[0] == m_mapFromMatchToNewNodeId[mInNodeId]
						&& newSgNode.outEdgeNodeList[0] == m_mapFromMatchToNewNodeId[mOutNodeId])
					{
						matchedSgNode.isAligned = true;
						newSgNode.isAligned = true;
						m_mapFromMatchToNewNodeId[mi] = ci;  // save aligned pairwise relationship node map

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
			m_mapFromMatchToNewNodeId[mi] = newSg->m_nodeNum-1;  // node id is the last node's id; save inserted node map
		}
	}

	// insert edges from matched ssg if it is not existing in current ssg
	for (int mei = 0; mei < matchedSg->m_edgeNum; mei++)
	{
		SemEdge& matchedSgEdge = matchedSg->m_edges[mei];

		int s = m_mapFromMatchToNewNodeId[matchedSgEdge.sourceNodeId];
		int t = m_mapFromMatchToNewNodeId[matchedSgEdge.targetNodeId];
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
			int ci = m_mapFromMatchToNewNodeId[mi];
			newSg->m_objectGraphNodeIdToModelSceneIdMap[ci] = currMetaModelNum-1;
		}
	}

	// geometry alignment
	geometryAlignment(matchedSg, newSg);

	return newSg;
}


void SceneGenerator::geometryAlignment(SceneSemGraph *matchedSg, SceneSemGraph *targetSg)
{
	for (int mi = 0; mi < matchedSg->m_nodeNum; mi++)
	{
		SemNode& matchedSgNode = matchedSg->m_nodes[mi];
		if (!matchedSgNode.isAligned && matchedSgNode.nodeType == "pairwise_relationship")
		{
			// edge dir: (active, relation), (relation, reference)
			int mInNodeId = matchedSgNode.inEdgeNodeList[0];  // active
			int mOutNodeId = matchedSgNode.outEdgeNodeList[0]; // reference

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

			int tarRefNodeId = m_mapFromMatchToNewNodeId[mRefNodeId];
			int newActiveNodeId = m_mapFromMatchToNewNodeId[mActiveNodeId];

			// compute transformation matrix based on the ref nodes
			int mRefModelId = matchedSg->m_objectGraphNodeIdToModelSceneIdMap[mRefNodeId];
			int tarRefModeId = targetSg->m_objectGraphNodeIdToModelSceneIdMap[tarRefNodeId];

			MetaModel &mRefModel = matchedSg->m_metaScene.m_metaModellList[mRefModelId];
			MetaModel &tarRefModel = targetSg->m_metaScene.m_metaModellList[tarRefModeId];

			mat4 transMat = computeTransMat(mRefModel, tarRefModel);

			// transform active model by initial alignment
			// initial alignment will make the relative orientation between the new active model and the target active model right
			int newActiveModelId = targetSg->m_objectGraphNodeIdToModelSceneIdMap[newActiveNodeId];
			MetaModel &newActiveModel = targetSg->m_metaScene.m_metaModellList[newActiveModelId];
			newActiveModel.transformation = transMat*newActiveModel.transformation;
			newActiveModel.position = TransformPoint(transMat, newActiveModel.position);
			newActiveModel.frontDir = TransformVector(transMat, newActiveModel.frontDir);
			newActiveModel.upDir = TransformVector(transMat, newActiveModel.upDir);

			// adjust position of transformed active model
			// sample a position on the support plane on the target reference model using it's UV parameters from the original ref model


		}
	}
}


mat4 SceneGenerator::computeTransMat(const MetaModel &fromModel, const MetaModel &toModel)
{
	mat4 rotMat = GetRotationMatrix(fromModel.frontDir, toModel.frontDir);
	mat4 transMat = GetTransformationMat(rotMat, fromModel.position, toModel.position);

	return transMat;
}


