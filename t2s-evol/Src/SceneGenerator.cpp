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

	m_layoutPlanner = new LayoutPlanner(m_relModelManager, m_sceneSemGraphManager);
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
	m_currScene = ts;
	m_currUserSSG = ts->m_ssg;

	if (m_currUserSSG != NULL)
	{
		m_currUserSSG->setNodesUnAligned();
	}
}

SemanticGraph* SceneGenerator::prepareQuerySG()
{
	SemanticGraph *querySG;

	if (!m_textSSG->m_isCommand)
	{
		// binding to current scene
		if (m_currUserSSG != NULL)
		{
			//resetNodes(querySG);
			m_currUserSSG->m_nodeAlignMap.clear();

			// enrich or modify textSSG based on current scene; clarify determiners etc.
			adjustTextSSGWithCurrSSG(m_textSSG, m_currUserSSG);

			// init query graph with current scene
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
		// init a new scene with the text
		else
		{
			querySG = new SemanticGraph(m_textSSG);

			for (int i = 0; i < querySG->m_nodeNum; i++)
			{
				SemNode &sgNode = querySG->m_nodes[i];
				sgNode.matchingStatus = SemNode::ExplicitNode;
			}
		}
	}

	return querySG;
}

void SceneGenerator::adjustTextSSGWithCurrSSG(TextSemGraph *textSSG, SceneSemGraph *currSSG)
{
	// collect entity that has determiner "each"
	std::vector<QString> anchorEntityNames;
	std::vector<QString> anchorDeterminers;
	for (int i = 0; i < textSSG->m_sentence.entityCount; i++)
	{
		SelEntity &currEntity = textSSG->m_sentence.m_entities[i];
		QString determiner = currEntity.m_determiner;

		if (determiner == "each" || determiner =="the")
		{
			anchorEntityNames.push_back(textSSG->m_sentence.m_entities[i].nameString);
			anchorDeterminers.push_back(determiner);
		}
	}

	// count instance number of anchor obj in current scene and add missing act and relation node
	for (int i=0; i < anchorEntityNames.size(); i++)
	{
		QString objName = anchorEntityNames[i];
		int instanceCountInCurrSSG = 0;
		int instanceCountInTextSSG = 0;
		
		// find object node, add duplicated relation node and act obj node
		for (int j=0; j < currSSG->m_nodeNum; j++)
		{
			SemNode &objNode = currSSG->m_nodes[j];
			QString currNodeName = objNode.nodeName;

			if (textSSG->m_entityNameToNodeNameMap.count(objName))
			{
				objName = textSSG->m_entityNameToNodeNameMap[currNodeName];
			}

			if (currNodeName == objName)
			{
				instanceCountInCurrSSG++;
			}
		}

		std::vector<int> anchorObjNodeIds;
		for (int j = 0; j < textSSG->m_nodeNum; j++)
		{
			SemNode &objNode = textSSG->m_nodes[j];
			QString currNodeName = objNode.nodeName;

			if (textSSG->m_entityNameToNodeNameMap.count(objName))
			{
				objName = textSSG->m_entityNameToNodeNameMap[objName];
			}

			if (currNodeName == objName)
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
			SemNode currTSGAnchorNode = textSSG->m_nodes[currAnchorNodeId];

			textSSG->addNode(currTSGAnchorNode);
			int newAnchorNodeId = textSSG->m_nodeNum - 1;

			if (!currTSGAnchorNode.inEdgeNodeList.empty())
			{
				for (int r=0; r < currTSGAnchorNode.inEdgeNodeList.size(); r++)
				{
					int relNodeId = currTSGAnchorNode.inEdgeNodeList[r];
					SemNode relNode = textSSG->m_nodes[relNodeId];

					// find determiner of the act obj; add instance if the determiner is not "a" or specific number
					int actNodeId = relNode.activeNodeList[0];
					SemNode actNode = textSSG->m_nodes[actNodeId];

					QString actDeterminer = textSSG->getDeterminerOfEntity(actNode.nodeName);
					if (actDeterminer!="a" || anchorDeterminers[i] == "each")
					{
						textSSG->addNode(relNode);
						int newRelNodeId = textSSG->m_nodeNum - 1;
						textSSG->addEdge(newRelNodeId, newAnchorNodeId);

						for (int k = 0; k < relNode.activeNodeList.size(); k++)
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

			//for (int a=0; a < currSSG->m_addedContextNodeIds.size(); a++)
			//{
			//	int addedActNodeId = currSSG->m_addedContextNodeIds[a];
			//	SemNode &currSSGActNode = currSSG->m_nodes[addedActNodeId];

			//	if (!currSSGActNode.outEdgeNodeList.empty())
			//	{
			//		int addedRelNodeId = currSSGActNode.outEdgeNodeList[0];
			//		SemNode &currSSGRelNode = currSSG->m_nodes[addedRelNodeId];
			//		if (!currSSGRelNode.outEdgeNodeList.empty())
			//		{
			//			int currSSGAnchorNodeId = currSSGRelNode.outEdgeNodeList[0];
			//			SemNode &currSSGAnchorNode = currSSG->m_nodes[currSSGAnchorNodeId];

			//			if (currSSGAnchorNode.nodeName == currTSGAnchorNode.nodeName)
			//			{
			//				textSSG->addNode(currSSG->m_nodes[addedActNodeId]);
			//				int newActNodeId = textSSG->m_nodeNum - 1;

			//				textSSG->addNode(currSSGRelNode);
			//				int newRelNodeId = textSSG->m_nodeNum - 1;

			//				textSSG->addEdge(newActNodeId, newRelNodeId);
			//				textSSG->addEdge(newRelNodeId, newAnchorNodeId);
			//			}
			//		}
			//	}		
			//}
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

	if (querySG == NULL)
	{
		qDebug() << "No command could be executed";
	}

	std::vector<TSScene*> tsscenes;

	if (!querySG->m_isCommand)
	{
		m_semanticGraphMatcher->updateQuerySG(querySG);

		std::vector<SceneSemGraph*> matchedSSGs = m_semanticGraphMatcher->retrieveDatabaseSSGs(num);

		for (int i = 0; i < matchedSSGs.size(); i++)
		{
			SceneSemGraph *newUserSsg = accommodateToCurrTSScene(matchedSSGs[i]);
			TSScene *s = newUserSsg->covertToTSScene(m_models);

			s->m_layoutPlanner = m_layoutPlanner;
			s->m_relModelManager = m_relModelManager;
			tsscenes.push_back(s);
		}
	}

	if (querySG!=NULL)
	{
		delete querySG;
	}

	return tsscenes;
}

void SceneGenerator::executeCommandsToCurrentScene()
{	
	if (m_currUserSSG == NULL) return;

	m_currScene->m_toPlaceModelIds.clear();

	for (int i=0; i < m_currUserSSG->m_nodes.size(); i++)
	{
		SemNode &objNode = m_currUserSSG->m_nodes[i];

		if (objNode.nodeType == "object")
		{
			int metaModelId = m_currUserSSG->m_graphNodeToModelListIdMap[i];
			MetaModel &md = m_currUserSSG->m_graphMetaScene.m_metaModellList[metaModelId];
			if (md.isJustReplaced)
			{
				md.isJustReplaced = false;
			}
		}
	}

	for (int i=0; i < m_textSSG->m_nodes.size(); i++)
	{
		SemNode &commandNode = m_textSSG->m_nodes[i];

		if (commandNode.nodeType == "command")
		{
			if (!commandNode.inEdgeNodeList.empty())
			{
				for (int j=0; j < commandNode.inEdgeNodeList.size(); j++)
				{
					int directObjNodeId = commandNode.inEdgeNodeList[j];
					SemNode &dirObjNode = m_textSSG->m_nodes[directObjNodeId];

					// collect all direct objs
					for (int j = 0; j < m_currUserSSG->m_nodes.size(); j++)
					{
						SemNode &objNode = m_currUserSSG->m_nodes[j];
						if (objNode.nodeType == "object" && objNode.nodeName == dirObjNode.nodeName)
						{
							m_layoutPlanner->executeCommand(m_currScene, commandNode.nodeName, j);
						}
					}
				}
			}
		}
	}
}

SceneSemGraph* SceneGenerator::accommodateToCurrTSScene(SceneSemGraph *matchedSg)
{
	SceneSemGraph *newUserSsg;

	// unalign nodes of matched ssg
	matchedSg->setNodesUnAligned();

	if (m_currUserSSG == NULL)
	{
		newUserSsg = new SceneSemGraph(matchedSg);
		newUserSsg->initMetaModelSuppPlanes(m_models);

		// set scene file name that current match comes from
		newUserSsg->m_graphMetaScene.m_sceneFileName = matchedSg->m_graphMetaScene.m_sceneFileName;
		return newUserSsg;
	}

	// set status for object already in current ssg
	for (int i = 0; i < m_currUserSSG->m_graphMetaScene.m_metaModellList.size(); i++)
	{
		m_currUserSSG->m_graphMetaScene.m_metaModellList[i].isAlreadyPlaced = true;
		m_currUserSSG->m_graphMetaScene.m_metaModellList[i].isBvhReady = false;  // update BVH before each evolution
	}

	// copy from current sg
	newUserSsg = new SceneSemGraph(m_currUserSSG);

	// align object node and relationship node
	double alignScore = 0;
	matchedSg->m_nodeAlignMap.clear();
	matchedSg->alignObjectNodesToGraph(newUserSsg, alignScore);
	matchedSg->alignRelationNodesToGraph(newUserSsg, alignScore);

	// merge matched ssg to current scene ssg
	newUserSsg->mergeWithMatchedSSG(matchedSg);

	// geometry alignment
	newUserSsg->initMetaModelSuppPlanes(m_models);
	m_layoutPlanner->initPlaceByAlignRelation(matchedSg, newUserSsg);

	// set scene file name that current match comes from
	newUserSsg->m_graphMetaScene.m_sceneFileName = matchedSg->m_graphMetaScene.m_sceneFileName;
	return newUserSsg;
}

