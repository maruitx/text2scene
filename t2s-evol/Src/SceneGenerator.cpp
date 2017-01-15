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
	// geometry align of the matched nodes
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
			int tarRefModelId = targetSg->m_objectGraphNodeIdToModelSceneIdMap[tarRefNodeId];

			MetaModel &mRefModel = matchedSg->m_metaScene.m_metaModellList[mRefModelId];
			MetaModel &tarRefModel = targetSg->m_metaScene.m_metaModellList[tarRefModelId];

			// initial alignment; align the rotation etc.	
			mat4 alignTransMat = computeTransMat(mRefModel, tarRefModel);

			// transform active model by initial alignment
			// initial alignment will make the relative orientation between the new active model and the target active model right
			int newActiveModelId = targetSg->m_objectGraphNodeIdToModelSceneIdMap[newActiveNodeId];
			MetaModel &newActiveModel = targetSg->m_metaScene.m_metaModellList[newActiveModelId];

			vec3 initPositionInScene = newActiveModel.position; // get the pos of model in current scene
	
			// find the position after initial alignment
			vec3 alignedPosition = TransformPoint(alignTransMat, initPositionInScene); // position after initial alignment

			// find the target position on new ref obj using the U, V w.r.t the original parent
			MetaModel &mActiveNode = matchedSg->m_metaScene.m_metaModellList[mActiveNodeId];
			vec3 mUVH = mActiveNode.parentPlaneUVH;
			//qDebug() << QString("UVH %1 %2 %3").arg(mUVH.x).arg(mUVH.y).arg(mUVH.z) <<"\n";

			SuppPlane& tarRefSuppPlane = tarRefModel.suppPlane;
			vec3 targetPosition = tarRefSuppPlane.getPointByUV(mUVH.x, mUVH.y); // position in the current scene, support plane is already transformed
			//for (int ci = 0; ci < 4; ci++)
			//{
			//	qDebug() << QString("corner%1 %2 %3 %4").arg(ci).arg(tarRefSuppPlane.m_corners[ci].x).arg(tarRefSuppPlane.m_corners[ci].y).arg(tarRefSuppPlane.m_corners[ci].z) << "\n";
			//}

			vec3 translationVec = targetPosition - alignedPosition;
			//qDebug() << QString("alignedPosition %1 %2 %3").arg(alignedPosition.x).arg(alignedPosition.y).arg(alignedPosition.z) << "\n";
			//qDebug() << QString("targetPosition %1 %2 %3").arg(targetPosition.x).arg(targetPosition.y).arg(targetPosition.z) << "\n";
			//qDebug() << QString("translationVec %1 %2 %3").arg(translationVec.x).arg(translationVec.y).arg(translationVec.z) << "\n";

			mat4 adjustTransMat;
			adjustTransMat = adjustTransMat.translate(translationVec);
			//qDebug() << QString("adjustTransMat %1 %2 %3 %4 ").arg(adjustTransMat.a11).arg(adjustTransMat.a21).arg(adjustTransMat.a31).arg(adjustTransMat.a41) <<
			//	QString("%1 %2 %3 %4 ").arg(adjustTransMat.a12).arg(adjustTransMat.a22).arg(adjustTransMat.a32).arg(adjustTransMat.a42) <<
			//	QString("%1 %2 %3 %4 ").arg(adjustTransMat.a13).arg(adjustTransMat.a23).arg(adjustTransMat.a33).arg(adjustTransMat.a43) <<
			//	QString("%1 %2 %3 %4 ").arg(adjustTransMat.a14).arg(adjustTransMat.a24).arg(adjustTransMat.a34).arg(adjustTransMat.a44) <<"\n";

			mat4 finalTransMat = adjustTransMat*alignTransMat;

			newActiveModel.position = finalTransMat*newActiveModel.position;
			newActiveModel.transformation = finalTransMat*newActiveModel.transformation;
			newActiveModel.frontDir = TransformVector(finalTransMat, newActiveModel.frontDir);
			newActiveModel.upDir = TransformVector(finalTransMat, newActiveModel.upDir);
			newActiveModel.suppPlane.tranfrom(finalTransMat);

			// adjust position of transformed active model
			// sample a position on the support plane on the target reference model using it's UV parameters from the original ref model

		}
	}

	// TODO: geometry align of the inferred nodes

}


mat4 SceneGenerator::computeTransMat(const MetaModel &fromModel, const MetaModel &toModel)
{
	mat4 rotMat = GetRotationMatrix(fromModel.frontDir, toModel.frontDir);
	mat4 transMat = GetTransformationMat(rotMat, fromModel.position, toModel.position);

	return transMat;
}


