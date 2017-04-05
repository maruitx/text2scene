#include "SceneGenerator.h"
#include "SceneSemGraphManager.h"
#include "SceneSemGraph.h"
#include "TextSemGraph.h"
#include "SemGraphMatcher.h"
#include "RelationModelManager.h"
#include "LayoutPlanner.h"
#include "TSScene.h"
#include "CollisionManager.h"
#include "Utility.h"
#include "Headers.h"

SceneGenerator::SceneGenerator(unordered_map<string, Model*> &models)
	:m_models(models)
{
	m_sceneSemGraphManager = new SceneSemGraphManager();
	m_semanticGraphMatcher = new SemGraphMatcher(m_sceneSemGraphManager);

	m_relModelManager = new RelationModelManager();
	m_layoutPlanner = new LayoutPlanner(m_relModelManager);
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
	m_currUserSSG = ts->m_ssg;

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

		// set node matching status
		for (int i = 0; i < querySG->m_nodeNum; i++)
		{
			SemNode& sgNode = querySG->m_nodes[i];

			if (isMapContainsValue(m_textSSG->m_toNewSgNodeIdMap, i))
			{
				sgNode.matchingStatus = SemNode::ExplicitNode;
			}
			else
				sgNode.matchingStatus = SemNode::ContextNode;
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
		s->m_layoutPlanner = m_layoutPlanner;
		s->m_relModelManager = m_relModelManager;

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

	// align object node and relationship node
	double alignScore = 0;
	matchedSg->m_toNewSgNodeIdMap.clear();
	matchedSg->alignObjectNodesWithGraph(newUserSsg, alignScore);
	matchedSg->alignRelationNodesWithGraph(newUserSsg, alignScore);

	// merge matched ssg to current scene ssg
	newUserSsg->mergeWithMatchedSSG(matchedSg);

	// geometry alignment
	geometryAlignmentWithCurrScene(matchedSg, newUserSsg);

	bindBySynthesizedRelationships(newUserSsg);

	// set scene file name that current match comes from
	newUserSsg->m_metaScene.m_sceneFileName = matchedSg->m_metaScene.m_sceneFileName;
	return newUserSsg;
}

void SceneGenerator::geometryAlignmentWithCurrScene(SceneSemGraph *matchedSg, SceneSemGraph *currSg)
{
	m_layoutPlanner->initPlaceByAlignRelation(matchedSg, currSg);


	// TODO: geometry align of the inferred nodes

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

				int refModelId = targetSg->m_graphNodeToModelListIdMap[refNodeId];
				int activeModelId = targetSg->m_graphNodeToModelListIdMap[activeNodeId];

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

				newActiveModel.updateWithTransform(transMat);
			}
		}
	}
}

