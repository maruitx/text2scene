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
	m_textSSG = tsg;
}

void SceneGenerator::updateCurrentTSScene(TSScene *ts)
{
	m_currTSScene = ts;
	m_currUserSSG = m_currTSScene->m_ssg;

	if (m_currUserSSG != NULL)
	{
		m_currUserSSG->setNodesUnAligned();
	}
}

SemanticGraph* SceneGenerator::prepareQuerySG()
{
	SemanticGraph *querySG;

	if (m_currUserSSG != NULL)
	{
		querySG = new SemanticGraph(m_currUserSSG);

		// update current UserSSG with textSSG for retrieval
		querySG->alignAndMergeWithGraph(m_textSSG);

		for (int i = 0; i < querySG->m_nodeNum; i++)
		{
			SemNode &sgNode = querySG->m_nodes[i];
			if (sgNode.isAligned)
			{
				sgNode.matchingStatus = SemNode::ExplicitNode;
			}
			else
			{
				sgNode.matchingStatus = SemNode::ContextNode;
			}
		}
	}

	else
	{
		querySG = new SemanticGraph(m_textSSG);
		
		for (int i = 0; i < querySG->m_nodeNum; i++)
		{
			SemNode &sgNode = querySG->m_nodes[i];
			sgNode.matchingStatus = SemNode::ExplicitNode;
		}
	}

	return querySG;
}

std::vector<TSScene*> SceneGenerator::generateTSScenes(int num)
{
	SemanticGraph *querySG = prepareQuerySG();
	m_semanticGraphMatcher->updateQuerySG(querySG);

	std::vector<SceneSemGraph*> matchedSSGs = m_semanticGraphMatcher->alignWithDatabaseSSGs(num);

	std::vector<TSScene*> tsscenes;
	for (int i = 0; i < matchedSSGs.size(); i++)
	{
		SceneSemGraph *newUserSsg = bindToCurrTSScene(matchedSSGs[i]);
		TSScene *s = newUserSsg->covertToTSScene(m_models);
		tsscenes.push_back(s);
	}

	delete querySG;

	return tsscenes;
}

SceneSemGraph* SceneGenerator::bindToCurrTSScene(SceneSemGraph *matchedSg)
{
	SceneSemGraph *newUserSsg;

	if (m_currUserSSG == NULL)
	{
		newUserSsg = new SceneSemGraph(matchedSg);

		// set scene file name that current match comes from
		newUserSsg->m_metaScene.m_sceneFileName = matchedSg->m_metaScene.m_sceneFileName;
		bindBySynthesizedRelationships(newUserSsg);
		return newUserSsg;
	}

	// unalign nodes of matched ssg
	matchedSg->setNodesUnAligned();

	// set status for object already in current ssg
	for (int i = 0; i < m_currUserSSG->m_metaScene.m_metaModellList.size(); i++)
	{
		m_currUserSSG->m_metaScene.m_metaModellList[i].isAlreadyPlaced = true;
		m_currUserSSG->m_metaScene.m_metaModellList[i].isBvhReady = false;  // update BVH before each evolution
	}

	// copy from current sg
	newUserSsg = new SceneSemGraph(m_currUserSSG);

	m_matchToNewUserSsgNodeMap.clear();
	double alignScore = 0;

	// align object node and relationship node
	matchedSg->alignObjectNodesWithGraph(newUserSsg, m_matchToNewUserSsgNodeMap, alignScore);
	matchedSg->alignRelationNodesWithGraph(newUserSsg, m_matchToNewUserSsgNodeMap, alignScore);

	// merge matched ssg to current scene ssg
	newUserSsg->mergeWithMatchedSSG(matchedSg, m_matchToNewUserSsgNodeMap);

	// geometry alignment
	geometryAlignmentWithCurrScene(matchedSg, newUserSsg);

	bindBySynthesizedRelationships(newUserSsg);

	// set scene file name that current match comes from
	newUserSsg->m_metaScene.m_sceneFileName = matchedSg->m_metaScene.m_sceneFileName;
	return newUserSsg;
}

void SceneGenerator::geometryAlignmentWithCurrScene(SceneSemGraph *matchedSg, SceneSemGraph *targetSg)
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
			// if no reference node is aligned, just use the ref node in matchedSg, and align to it
			else
			{
				break;
			}

			int tarRefNodeId = m_matchToNewUserSsgNodeMap[mRefNodeId];

			// compute transformation matrix based on the ref nodes
			int mRefModelId = matchedSg->m_objectGraphNodeToModelListIdMap[mRefNodeId];
			int tarRefModelId = targetSg->m_objectGraphNodeToModelListIdMap[tarRefNodeId];

			MetaModel &mRefModel = matchedSg->m_metaScene.m_metaModellList[mRefModelId];
			MetaModel &tarRefModel = targetSg->m_metaScene.m_metaModellList[tarRefModelId];

			// initial alignment; align the rotation etc.	
			mat4 alignTransMat = computeTransMat(mRefModel, tarRefModel);

			// transform active model by initial alignment
			// initial alignment will make the relative orientation between the new active model and the target active model right
			int newActiveNodeId = m_matchToNewUserSsgNodeMap[mActiveNodeId];
			int newActiveModelId = targetSg->m_objectGraphNodeToModelListIdMap[newActiveNodeId];
			MetaModel &newActiveModel = targetSg->m_metaScene.m_metaModellList[newActiveModelId];

			// find the target position on new ref obj using the U, V w.r.t the original parent
			int mActiveModelId = matchedSg->m_objectGraphNodeToModelListIdMap[mActiveNodeId];
			MetaModel &mActiveModel = matchedSg->m_metaScene.m_metaModellList[mActiveModelId];
			vec3 mUVH = mActiveModel.parentPlaneUVH;

			vec3 initPositionInScene = newActiveModel.position; // get the pos of model in current scene
	
			// find the position after initial alignment
			vec3 alignedPosition = TransformPoint(alignTransMat, initPositionInScene); // position after initial alignment

			SuppPlane& tarRefSuppPlane = tarRefModel.suppPlane;
			vec3 targetPosition = tarRefSuppPlane.getPointByUV(mUVH.x, mUVH.y); // position in the current scene, support plane is already transformed
			//for (int ci = 0; ci < 4; ci++)
			//{
			//	qDebug() << QString("corner%1 %2 %3 %4").arg(ci).arg(tarRefSuppPlane.m_corners[ci].x).arg(tarRefSuppPlane.m_corners[ci].y).arg(tarRefSuppPlane.m_corners[ci].z) << "\n";
			//}

			vec3 translationVec = targetPosition - alignedPosition;

			mat4 adjustTransMat;
			adjustTransMat = adjustTransMat.translate(translationVec);

			mat4 finalTransMat = adjustTransMat*alignTransMat;

			newActiveModel.position = finalTransMat*newActiveModel.position;
			newActiveModel.transformation = finalTransMat*newActiveModel.transformation;
			newActiveModel.frontDir = TransformVector(finalTransMat, newActiveModel.frontDir);
			newActiveModel.upDir = TransformVector(finalTransMat, newActiveModel.upDir);
			newActiveModel.suppPlane.tranfrom(finalTransMat);                    

			//qDebug() << QString("Preview:%1 Query anchor:%2").arg(matchedSg->m_matchListId).arg(toQString(mRefModel.catName));
			//mRefModel.transformation.print();
			//qDebug() << QString("Preview:%1 Current anchor:%2").arg(matchedSg->m_matchListId).arg(toQString(tarRefModel.catName));
			//tarRefModel.transformation.print();

			//qDebug() << QString("Preview:%1 Current active:%2").arg(matchedSg->m_matchListId).arg(toQString(newActiveModel.catName));
			//qDebug() << QString("alignedPos");
			//alignedPosition.print();
			//qDebug() << QString("targetPos");
			//targetPosition.print();
			//qDebug() << QString("Parent UV:%1 %2").arg(mUVH.x).arg(mUVH.y);
			//
			//qDebug() << QString("TSG-Anchor:%1 USG-Anchor:%2 Current:%3").arg(toQString(mRefModel.catName)).arg(toQString(tarRefModel.catName)).arg(toQString(newActiveModel.catName));
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

void SceneGenerator::bindBySynthesizedRelationships(SceneSemGraph *targetSg)
{
	return;

	for (int mi = 0; mi < targetSg->m_nodeNum; mi++)
	{
		SemNode& targSgNode = targetSg->m_nodes[mi];
		if (!targSgNode.isAligned && targSgNode.nodeType == "pairwise_relationship") //  synthesized relationship node
		{
			// edge dir: (active, relation), (relation, reference)

			if (!targSgNode.outEdgeNodeList.empty())
			{
				int refNodeId = targSgNode.outEdgeNodeList[0];
				int activeNodeId = targSgNode.inEdgeNodeList[0];

				int refModelId = targetSg->m_objectGraphNodeToModelListIdMap[refNodeId];
				int activeModelId = targetSg->m_objectGraphNodeToModelListIdMap[activeNodeId];

				// compute transformation matrix based on the ref nodes
				MetaModel &tarRefModel = targetSg->m_metaScene.m_metaModellList[refModelId];
				MetaModel &newActiveModel = targetSg->m_metaScene.m_metaModellList[activeModelId];


				SuppPlane &suppPlane = tarRefModel.suppPlane;
				vec3 uvh = newActiveModel.parentPlaneUVH;

				vec3 newPos = suppPlane.getPointByUV(uvh.x, uvh.y);

				mat4 transMat;
				vec3 translateVec;
				translateVec = newPos - newActiveModel.position;
				transMat = transMat.translate(translateVec);

				newActiveModel.position = transMat*newActiveModel.position;
				newActiveModel.transformation = transMat*newActiveModel.transformation;
				newActiveModel.frontDir = TransformVector(newActiveModel.transformation, newActiveModel.frontDir);
				newActiveModel.upDir = TransformVector(newActiveModel.transformation, newActiveModel.upDir);
				newActiveModel.suppPlane.tranfrom(transMat);
			}
		}
	}
}


