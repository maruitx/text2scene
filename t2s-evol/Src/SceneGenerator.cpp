#include "SceneGenerator.h"
#include "SceneSemGraphManager.h"
#include "SceneSemGraph.h"
#include "TextSemGraph.h"
#include "SemGraphMatcher.h"
#include "RelationModelManager.h"
#include "LayoutPlanner.h"
#include "TSScene.h"
#include "Utility.h"
#include "Headers.h"

SceneGenerator::SceneGenerator(unordered_map<string, Model*> &models)
	:m_models(models)
{
	m_relModelManager = new RelationModelManager();

	m_sceneSemGraphManager = new SceneSemGraphManager();
	m_semanticGraphMatcher = new SemGraphMatcher(m_sceneSemGraphManager, m_relModelManager);

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
		//resetNodes(querySG);
		m_currUserSSG->m_nodeAlignMap.clear();
		adjustTextSSGWithCurrSSG(m_textSSG, m_currUserSSG);

		querySG = new SemanticGraph(m_currUserSSG);
		// update current UserSSG with textSSG for retrieval
		querySG->alignAndMergeWithGraph(m_textSSG);

		// set node matching status
		for (int i = 0; i < querySG->m_nodeNum; i++)
		{
			SemNode& sgNode = querySG->m_nodes[i];

			if (isMapContainsValue(m_textSSG->m_nodeAlignMap, i))
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

void SceneGenerator::adjustTextSSGWithCurrSSG(TextSemGraph *textSSG, SceneSemGraph *currSSG)
{
	std::vector<QString> entityNames;
	for (int i = 0; i < textSSG->m_sentence.entityCount; i++)
	{
		SelEntity &currEntity = textSSG->m_sentence.m_entities[i];
		QString determiner = currEntity.m_determiner;

		if (determiner == "each")
		{
			entityNames.push_back(textSSG->m_sentence.m_entities[i].nameString);
		}
	}

	for (int i=0; i < entityNames.size(); i++)
	{
		QString objName = entityNames[i];
		int instanceCountInCurrSSG = 0;
		int instanceCountInTextSSG = 0;
		
		// find object node, add duplicated relation node and act obj node
		for (int j=0; j < currSSG->m_nodeNum; j++)
		{
			SemNode &objNode = currSSG->m_nodes[j];
			if (objNode.nodeName == objName)
			{
				instanceCountInCurrSSG++;
			}
		}

		std::vector<int> anchorObjNodeIds;
		for (int j = 0; j < textSSG->m_nodeNum; j++)
		{
			SemNode &objNode = textSSG->m_nodes[j];
			if (objNode.nodeName == objName)
			{
				anchorObjNodeIds.push_back(j);
				instanceCountInTextSSG++;
			}
		}

		// add instance to TextSSG
		int addInstanceNum = instanceCountInCurrSSG - instanceCountInTextSSG;
		for (int j=0; j < addInstanceNum; j++)
		{
			int currAnchorNodeId = anchorObjNodeIds[0];  // use the first anchor node
			SemNode currAnchorNode = textSSG->m_nodes[currAnchorNodeId];

			textSSG->addNode(currAnchorNode);
			int newAnchoNodeId = textSSG->m_nodeNum - 1;

			if (!currAnchorNode.inEdgeNodeList.empty())
			{
				int relNodeId = currAnchorNode.inEdgeNodeList[0];
				SemNode relNode = textSSG->m_nodes[relNodeId];

				textSSG->addNode(relNode);

				int newRelNodeId = textSSG->m_nodeNum - 1;
				textSSG->addEdge(newRelNodeId, newAnchoNodeId);

				for (int k=0; k < relNode.activeNodeList.size(); k++)
				{
					int actNodeId = relNode.activeNodeList[k];
					SemNode actNode = textSSG->m_nodes[actNodeId];

					textSSG->addNode(actNode);
					
					int newActNodeId = textSSG->m_nodeNum - 1;
					textSSG->addEdge(newActNodeId, newRelNodeId);
				}
			}
		}
	}

	textSSG->parseNodeNeighbors();
}

void SceneGenerator::resetNodes(SemanticGraph *sg)
{
	sg->m_nodeAlignMap.clear();

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

	// unalign nodes of matched ssg
	matchedSg->setNodesUnAligned();

	if (m_currUserSSG == NULL)
	{
		newUserSsg = new SceneSemGraph(matchedSg);
		newUserSsg->initMetaModelSuppPlanes(m_models);

		// set scene file name that current match comes from
		newUserSsg->m_metaScene.m_sceneFileName = matchedSg->m_metaScene.m_sceneFileName;
		return newUserSsg;
	}

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
	matchedSg->m_nodeAlignMap.clear();
	matchedSg->alignObjectNodesWithGraph(newUserSsg, alignScore);
	matchedSg->alignRelationNodesWithGraph(newUserSsg, alignScore);

	// merge matched ssg to current scene ssg
	newUserSsg->mergeWithMatchedSSG(matchedSg);

	// geometry alignment
	newUserSsg->initMetaModelSuppPlanes(m_models);
	m_layoutPlanner->initPlaceByAlignRelation(matchedSg, newUserSsg);

	// set scene file name that current match comes from
	newUserSsg->m_metaScene.m_sceneFileName = matchedSg->m_metaScene.m_sceneFileName;
	return newUserSsg;
}

