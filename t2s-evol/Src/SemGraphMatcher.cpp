#include "SemGraphMatcher.h"
#include "SceneSemGraphManager.h"
#include "RelationModelManager.h"
#include "SceneSemGraph.h"
#include "TextSemGraph.h"


SemGraphMatcher::SemGraphMatcher(SceneSemGraphManager *ssgManager, RelationModelManager *relManager)
	:m_sceneSemGraphManager(ssgManager), m_relModelManager(relManager)
{
	loadModelBlackList();
	cout << "\nSemGraphMatcher: loading semantic graphs number " << m_sceneSemGraphManager->m_ssgNum<<"\n";
}

SemGraphMatcher::~SemGraphMatcher()
{

}

void SemGraphMatcher::updateQuerySG(SemanticGraph *sg)
{
	m_querySSG = sg;
}

vector<SceneSemGraph*> SemGraphMatcher::alignWithDatabaseSSGs(int targetMatchNum)
{
	cout << "SemGraphMatcher: start graph matching.\n";

	vector<pair<double, SceneSemGraph *>> scoredDBSubSSGs;
	double exactMatchScore = m_querySSG->m_nodes.size();
	int exactMatchNum = 0;

	// align to each dbSSG and compute align score
	for (int i = 0; i < m_sceneSemGraphManager->m_ssgNum; i++)
	{
		SceneSemGraph *currDBSSG = m_sceneSemGraphManager->getGraph(i);

		double matchingScore = 0;
		SceneSemGraph *subSSG = alignSSGWithDBSSG(m_querySSG, currDBSSG, matchingScore);
		subSSG->m_dbSSGId = i;

		if (!ssgContainsBlackListModel(subSSG))
		{
			scoredDBSubSSGs.push_back(make_pair(matchingScore, subSSG));
			if (matchingScore == exactMatchScore)
			{
				exactMatchNum++;
			}
		}
	}

	// find top score subSSGs
	sort(scoredDBSubSSGs.begin(), scoredDBSubSSGs.end()); // ascending order
	reverse(scoredDBSubSSGs.begin(), scoredDBSubSSGs.end()); // descending order

	int matchedSSGNum = scoredDBSubSSGs.size();
	vector<SceneSemGraph*> matchedSubSSGs;

	std::vector<int> nonRepeatSSGIds = findNonRepeatSSGs(scoredDBSubSSGs, targetMatchNum);

	for (int i = 0; i < nonRepeatSSGIds.size(); i++)
	{
		int sgId = nonRepeatSSGIds[i];
		SceneSemGraph *ssg = scoredDBSubSSGs[sgId].second;

		if (ssg != NULL)
		{
			matchedSubSSGs.push_back(ssg);
		}
	}

	// enrich subSSG
	for (int i=0; i < matchedSubSSGs.size(); i++)
	{
		SceneSemGraph *matchedSubSSG = matchedSubSSGs[i];
		SceneSemGraph *dbSSG = m_sceneSemGraphManager->getGraph(matchedSubSSG->m_dbSSGId);

		addGroupActNodesToSubSSG(matchedSubSSG, dbSSG);

		if (params::inst()->addSynthNode)
		{
			addSynthNodeToSubSSG(matchedSubSSG, dbSSG);
		}

		matchedSubSSG->restoreMissingSupportNodes();
		addSuppParentNodesToSubSSG(matchedSubSSG, dbSSG);

		if (params::inst()->isUseContext)
		{
			addContextNodesToSubSSG(matchedSubSSG, dbSSG);
		}
	}

	cout << "SemGraphMatcher: graph matching done, found instance num " << matchedSSGNum << 
		", exact match num " << exactMatchNum
		<< ", shown instance num " << matchedSubSSGs.size() << ".\n";
	return matchedSubSSGs;
}

vector<int> SemGraphMatcher::findNonRepeatSSGs(const vector<pair<double, SceneSemGraph *>> &scoredDBSubSSGs, int targetNum)
{
	// some SSGs are same since the subscenes may be re-used in multiple scenes

	int matchedSSGNum = scoredDBSubSSGs.size();
	targetNum = min(targetNum, matchedSSGNum);

	std::vector<int> nonRepeatSSGids;
	nonRepeatSSGids.push_back(0);

	for (int i = 1; i < matchedSSGNum; i++)
	{
		bool isRepeat = false;

		if (nonRepeatSSGids.size() == targetNum)
		{
			break;
		}

		for (int ni = 0; ni < nonRepeatSSGids.size(); ni++)
		{
			int refSSGId = nonRepeatSSGids[ni];
			double simVal = computeGeometrySimilarity(scoredDBSubSSGs[i].second, scoredDBSubSSGs[refSSGId].second);

			if (simVal > 0.8)
			{
				isRepeat = true;
				break;
			}
		}

		if (!isRepeat)
		{
			nonRepeatSSGids.push_back(i);
		}
	}

	return nonRepeatSSGids;
}

bool SemGraphMatcher::ssgContainsBlackListModel(SceneSemGraph *ssg)
{
	for (int i=0; i < ssg->m_metaScene.m_metaModellList.size(); i++)
	{
		MetaModel &md = ssg->m_metaScene.m_metaModellList[i];
		if (isModelInBlackList(toQString(md.name)))
		{
			return true;
		}
	}

	return false;
}

SceneSemGraph* SemGraphMatcher::alignSSGWithDBSSG(SemanticGraph *querySSG, SceneSemGraph *dbSSG, double &matchingScore)
{
	SceneSemGraph *matchedSubSSG;

	// clear aligned info
	querySSG->m_nodeAlignMap.clear();
	querySSG->setNodesUnAligned();
	dbSSG->setNodesUnAligned();

	// clear saved query 
	if (dbSSG->m_alignedQuerySSG != NULL)
	{
		delete dbSSG->m_alignedQuerySSG;
		dbSSG->m_alignedQuerySSG = NULL;
	}

	querySSG->alignObjectNodesWithGraph(dbSSG, matchingScore);
	querySSG->alignRelationNodesWithGraph(dbSSG, matchingScore);

	// collect matched nodes and generate subgraph
	std::vector<int> matchedDBSsgNodeList;
	for (int ni = 0; ni < querySSG->m_nodeNum; ni++)
	{
		SemNode& sgNode = querySSG->m_nodes[ni];
		
		if (sgNode.isAligned && querySSG->m_nodeAlignMap.count(ni))
		{
			int matchedDBSsgNodeId = querySSG->m_nodeAlignMap[ni];
			MetaModel  &dbMd = dbSSG->getModelWithNodeId(matchedDBSsgNodeId);			
			if (dbMd.catName == "book" && dbMd.frontDir.dot(vec3(0, 0, 1)) < 0.5)
			{
				matchingScore -= 20;
			}

			matchedDBSsgNodeList.push_back(matchedDBSsgNodeId);
		}
	}

	// add nodes by exact match and scene context
	matchedSubSSG = dbSSG->getSubGraph(matchedDBSsgNodeList, m_relModelManager);
	dbSSG->m_alignedQuerySSG = new SemanticGraph(querySSG); // save a copy of aligned query SSG in correspond dbSSG

	return matchedSubSSG;
}

void SemGraphMatcher::addEdgesToSubSSG(SceneSemGraph *matchedSubSSG, SceneSemGraph *dbSSG)
{
	// build graph edges
	for (int i = 0; i < dbSSG->m_edgeNum; i++)
	{
		SemEdge dbEdge = dbSSG->m_edges[i];

		if (dbSSG->m_dbNodeToSubNodeMap.find(dbEdge.sourceNodeId) != dbSSG->m_dbNodeToSubNodeMap.end()
			&& dbSSG->m_dbNodeToSubNodeMap.find(dbEdge.targetNodeId) != dbSSG->m_dbNodeToSubNodeMap.end())
		{
			int newSourceId = dbSSG->m_dbNodeToSubNodeMap[dbEdge.sourceNodeId];
			int newTargetId = dbSSG->m_dbNodeToSubNodeMap[dbEdge.targetNodeId];

			if (!matchedSubSSG->isEdgeExist(newSourceId, newTargetId))
			{
				matchedSubSSG->addEdge(newSourceId, newTargetId);
			}
		}
	}
}

void SemGraphMatcher::addGroupActNodesToSubSSG(SceneSemGraph *matchedSubSSG, SceneSemGraph *dbSSG)
{
	int initNodeNum = matchedSubSSG->m_nodeNum;
	int currSubSSGNodeNum = initNodeNum;

	std::vector<int> insertDbObjList;

	// add active obj node for group node in subSSG
	for (int i=0; i < initNodeNum; i++)
	{
		SemNode &sgNode = matchedSubSSG->m_nodes[i];
		if (sgNode.nodeType=="group_relation")
		{
			int dbNodeId = getKeyForValueInMap(dbSSG->m_dbNodeToSubNodeMap, i);
			if (dbNodeId!=-1)
			{
				SemNode &dbNode = dbSSG->m_nodes[dbNodeId];
				if (!dbNode.anchorNodeList.empty() && dbNode.matchingStatus == SemNode::ExplicitNode)
				{
					int anchorObjId = dbNode.anchorNodeList[0];
					SemNode &anchorNode = dbSSG->m_nodes[anchorObjId];

					QString groupKey = dbNode.nodeName + "_" + anchorNode.nodeName;

					GroupRelationModel *groupModel;
					if (m_relModelManager->m_groupRelModels.count(groupKey))
					{
						groupModel = m_relModelManager->m_groupRelModels[groupKey];
					}

					std::vector<int> actNodeList = dbNode.activeNodeList;

					// insert act nodes based on occ prob and add support nodes
					std::vector<int> insertedObjNodeIds;
					for (int a = 0; a < actNodeList.size(); a++)
					{
						int dbActNodeId = actNodeList[a];
						// skip if node is already inserted
						if (dbSSG->m_dbNodeToSubNodeMap.count(dbActNodeId)) continue;			

						SemNode &dbActNode = dbSSG->m_nodes[dbActNodeId];

						if (dbActNode.nodeName == "chair") continue;
						if (dbActNode.nodeName == "desk") continue;
						if (dbActNode.nodeName == "table") continue;

						QString occKey = QString("%1_%2").arg(dbActNode.nodeName).arg(1);  // Temp, extend to multiple instances later

						if (groupModel->m_occurModels.count(occKey))
						{
							double randProb = GenRandomDouble(0, 0.9*groupModel->m_maxOccProb);
							double probTh = groupModel->m_occurModels[occKey]->m_occurProb;

							if (probTh > randProb)
							{
								dbActNode.isAnnotated = true;
								dbActNode.isAligned = false;
								matchedSubSSG->addNode(dbActNode);
								dbSSG->m_dbNodeToSubNodeMap[dbActNodeId] = currSubSSGNodeNum;
								insertDbObjList.push_back(dbActNodeId);
								currSubSSGNodeNum++;

								insertedObjNodeIds.push_back(dbActNodeId);

								// add support node for current active object
								//addSupportNodeForActNode(dbActNodeId, dbSSG, matchedSubSSG, currSubSSGNodeNum);

								bool reachBaseObj = false;
								SemNode &currActNode = dbActNode;
								while (!reachBaseObj)
								{
									bool hasSuppNode = false;
									for (int r = 0; r < currActNode.outEdgeNodeList.size(); r++)
									{
										int suppNodeId = currActNode.outEdgeNodeList[r];
										SemNode &suppNode = dbSSG->m_nodes[suppNodeId];
										if (suppNode.nodeName.contains("support") && !suppNode.anchorNodeList.empty())
										{
											// to insert a support node, it's anchor object must be already in the scene
											int dbAnchorId = suppNode.anchorNodeList[0];
											if (!dbSSG->m_dbNodeToSubNodeMap.count(dbAnchorId))
											{
												SemNode &dbAnchorNode = dbSSG->m_nodes[dbAnchorId];
												if (dbAnchorNode.nodeName == "room")
												{
													reachBaseObj = true;
													break;
												}

												dbAnchorNode.isAnnotated = true;
												dbAnchorNode.isAligned = false;

												matchedSubSSG->addNode(dbAnchorNode);
												dbSSG->m_dbNodeToSubNodeMap[dbAnchorId] = currSubSSGNodeNum;
												insertDbObjList.push_back(dbAnchorId);
												currSubSSGNodeNum++;
												currActNode = dbAnchorNode;
											}
											else
											{
												reachBaseObj = true;												
											}

											suppNode.isAnnotated = true;
											suppNode.isAligned = false;

											matchedSubSSG->addNode(suppNode);
											dbSSG->m_dbNodeToSubNodeMap[suppNodeId] = currSubSSGNodeNum;
											currSubSSGNodeNum++;

											hasSuppNode = true;
										}
									}

									if (!hasSuppNode) reachBaseObj = true;										
								}
							}
						}
					}

					// TODO: add high co-occur objects to current inserted object
					if (false)
					{
						for (int a = 0; a < actNodeList.size(); a++)
						{
							int dbActNodeId = actNodeList[a];
							// skip if node is already inserted
							if (dbSSG->m_dbNodeToSubNodeMap.count(dbActNodeId)) continue;

							if (std::find(insertedObjNodeIds.begin(), insertedObjNodeIds.end(), dbActNodeId) != insertedObjNodeIds.end())
								continue;

							bool isCoOcc = false;
							SemNode& dbActNode = dbSSG->m_nodes[dbActNodeId];
							for (int k = 0; k < insertedObjNodeIds.size(); k++)
							{
								int insertedDbActNodeId = insertedObjNodeIds[k];
								SemNode& insertedDbNode = dbSSG->m_nodes[insertedDbActNodeId];
							}

							if (isCoOcc)
							{
								dbActNode.isAnnotated = true;
								dbActNode.isAligned = false;
								matchedSubSSG->addNode(dbActNode);
								dbSSG->m_dbNodeToSubNodeMap[dbActNodeId] = currSubSSGNodeNum;
								insertDbObjList.push_back(dbActNodeId);
								currSubSSGNodeNum++;

								insertedObjNodeIds.push_back(dbActNodeId);
								addSupportNodeForActNode(dbActNodeId, dbSSG, matchedSubSSG, currSubSSGNodeNum);
							}
						}
					}
				}
			}			
		}
	}

	// add edges
	addEdgesToSubSSG(matchedSubSSG, dbSSG);

	// verify all support nodes is added as some nodes are missing because the anchor obj is not inserted yet
	for (int i = 0; i < initNodeNum; i++)
	{
		SemNode &sgNode = matchedSubSSG->m_nodes[i];
		if (sgNode.nodeType == "group_relation")
		{
			int dbNodeId = getKeyForValueInMap(dbSSG->m_dbNodeToSubNodeMap, i);
			if (dbNodeId != -1)
			{
				SemNode &dbNode = dbSSG->m_nodes[dbNodeId];
				if (!dbNode.anchorNodeList.empty() && dbNode.matchingStatus == SemNode::ExplicitNode)
				{
					std::vector<int> actNodeList = dbNode.activeNodeList;
					for (int a = 0; a < actNodeList.size(); a++)
					{
						int dbActNodeId = actNodeList[a];
						if (dbSSG->m_dbNodeToSubNodeMap.count(dbActNodeId))
						{
							int mActNodeId = dbSSG->m_dbNodeToSubNodeMap[dbActNodeId];
							if (!matchedSubSSG->hasSupportNode(mActNodeId))
							{
								SemNode &dbActNode = dbSSG->m_nodes[dbActNodeId];
								// add support node for current active object
								for (int r = 0; r < dbActNode.outEdgeNodeList.size(); r++)
								{
									int suppNodeId = dbActNode.outEdgeNodeList[r];
									SemNode &suppNode = dbSSG->m_nodes[suppNodeId];
									if (suppNode.nodeName == "vertsupport" && !suppNode.anchorNodeList.empty())
									{
										// to insert a support node, it's anchor object must be already in the scene
										int dbAnchorId = suppNode.anchorNodeList[0];
										if (!dbSSG->m_dbNodeToSubNodeMap.count(dbAnchorId)) continue;

										suppNode.isAnnotated = true;
										suppNode.isAligned = false;

										matchedSubSSG->addNode(suppNode);
										dbSSG->m_dbNodeToSubNodeMap[suppNodeId] = currSubSSGNodeNum;
										currSubSSGNodeNum++;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	addEdgesToSubSSG(matchedSubSSG, dbSSG);

	// add active objs to meta scene
	int modelInSceneId = matchedSubSSG->m_graphNodeToModelListIdMap.size();
	for (int i = 0; i < insertDbObjList.size(); i++)
	{
		int dbActNodeId = insertDbObjList[i];

		// non-object node is not saved in the map
		if (dbSSG->m_graphNodeToModelListIdMap.count(dbActNodeId))
		{
			int dbMetaModelId = dbSSG->m_graphNodeToModelListIdMap[dbActNodeId];

			if (dbMetaModelId < dbSSG->m_modelNum)
			{
				dbSSG->m_metaScene.m_metaModellList[dbMetaModelId].isSelected = dbSSG->m_nodes[dbActNodeId].isAnnotated;
				matchedSubSSG->m_metaScene.m_metaModellList.push_back(dbSSG->m_metaScene.m_metaModellList[dbMetaModelId]);
				int currNodeId = dbSSG->m_dbNodeToSubNodeMap[dbActNodeId];
				matchedSubSSG->m_graphNodeToModelListIdMap[currNodeId] = modelInSceneId;
				modelInSceneId++;
			}
		}
	}

	matchedSubSSG->parseNodeNeighbors();
}

void SemGraphMatcher::addSupportNodeForActNode(int dbActNodeId, SceneSemGraph *dbSSG, SceneSemGraph *matchedSubSSG, int &currSubSSGNodeNum)
{
	bool reachBaseObj = false;
	SemNode &currActNode = dbSSG->m_nodes[dbActNodeId];
	while (!reachBaseObj)
	{
		bool hasSuppNode = false;
		for (int r = 0; r < currActNode.outEdgeNodeList.size(); r++)
		{
			int suppNodeId = currActNode.outEdgeNodeList[r];
			SemNode &suppNode = dbSSG->m_nodes[suppNodeId];
			if (suppNode.nodeName.contains("support") && !suppNode.anchorNodeList.empty())
			{
				// to insert a support node, it's anchor object must be already in the scene
				int dbAnchorId = suppNode.anchorNodeList[0];
				if (!dbSSG->m_dbNodeToSubNodeMap.count(dbAnchorId))
				{
					SemNode &dbAnchorNode = dbSSG->m_nodes[dbAnchorId];
					if (dbAnchorNode.nodeName == "room")
					{
						reachBaseObj = true;
						break;
					}

					dbAnchorNode.isAnnotated = true;
					dbAnchorNode.isAligned = false;

					matchedSubSSG->addNode(dbAnchorNode);
					dbSSG->m_dbNodeToSubNodeMap[dbAnchorId] = currSubSSGNodeNum;

					currSubSSGNodeNum++;
					currActNode = dbAnchorNode;
				}
				else
				{
					reachBaseObj = true;
				}

				suppNode.isAnnotated = true;
				suppNode.isAligned = false;

				matchedSubSSG->addNode(suppNode);
				dbSSG->m_dbNodeToSubNodeMap[suppNodeId] = currSubSSGNodeNum;
				currSubSSGNodeNum++;

				hasSuppNode = true;
			}
		}

		if (!hasSuppNode) reachBaseObj = true;
	}
}

double SemGraphMatcher::computeGeometrySimilarity(SceneSemGraph *testSSG, SceneSemGraph *refSSG)
{
	double simVal = 0;
	double sceneMetric = params::inst()->globalSceneUnitScale;
	double distTh = 0.1;

	std::vector<int> refNodeIndicators(refSSG->m_nodeNum, 0);  // indicator of whether a node is aligned

	for (int i = 0; i < testSSG->m_nodeNum; i++)
	{
		SemNode& tsgNode = testSSG->m_nodes[i];

		if (tsgNode.nodeType == "object")
		{
			MetaModel &testModel = testSSG->getModelWithNodeId(i);

			for (int j = 0; j < refSSG->m_nodeNum; j++)
			{
				SemNode& rsgNode = refSSG->m_nodes[j];

				if (!refNodeIndicators[j] && rsgNode.nodeType == "object" && tsgNode.nodeName == rsgNode.nodeName)
				{
					MetaModel &refModel = refSSG->getModelWithNodeId(j);

					// if hash name string is same
					if (testModel.name == refModel.name)
					{
						double posDiff = (refModel.position - testModel.position).length();

						if (posDiff < distTh / sceneMetric)
						{
							simVal += 1;
							refNodeIndicators[j] = 1;
						}
					}
				}
			}
		}
	}

	simVal = simVal / testSSG->m_metaScene.m_metaModellList.size();

	return simVal;
}

void SemGraphMatcher::loadModelBlackList()
{
	QString filename = "./SceneDB/model_blacklist.txt";
	QFile inFile(filename);
	QTextStream ifs(&inFile);

	if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;

	while (!ifs.atEnd())
	{
		QString currLine = ifs.readLine();
		QStringList parts = currLine.split(",");
		m_modelBlackList.insert(parts[0]);
	}
	inFile.close();
}

bool SemGraphMatcher::isModelInBlackList(const QString &s)
{
	if (m_modelBlackList.count(s))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void SemGraphMatcher::addSynthNodeToSubSSG(SceneSemGraph *matchedSubSSG, SceneSemGraph *dbSSG)
{
	std::map<int, int> &dbNodeToSubNodeMap = dbSSG->m_dbNodeToSubNodeMap;
	SemanticGraph *querySSG = dbSSG->m_alignedQuerySSG;
	std::map<int, int> queryToSubSsgNodeMap;

	//collect matched nodes into the map
	for (int ni = 0; ni < querySSG->m_nodeNum; ni++)
	{
		SemNode& sgNode = querySSG->m_nodes[ni];

		if (sgNode.isAligned && querySSG->m_nodeAlignMap.count(ni))
		{
			// first find the DB-SSG node corresponding to the query node
			int dbNodeId = querySSG->m_nodeAlignMap[ni];
			int subNodeId = dbNodeToSubNodeMap[dbNodeId];

			// then find the Sub-SSG node for the query node
			queryToSubSsgNodeMap[ni] = subNodeId;
		}
	}

	// insert synthesized object node
	for (int ni = 0; ni < querySSG->m_nodeNum; ni++)
	{
		SemNode& sgNode = querySSG->m_nodes[ni];

		if (!sgNode.isAligned && !querySSG->m_nodeAlignMap.count(ni))
		{
			if (sgNode.nodeType == "object")
			{
				matchedSubSSG->addNode(sgNode);
				int currNodeId = matchedSubSSG->m_nodeNum - 1;
				matchedSubSSG->m_nodes[currNodeId].isSynthesized = 1;
				queryToSubSsgNodeMap[ni] = currNodeId;

				// set meta data
				std::vector<int> instanceIds = matchedSubSSG->findExistingInstanceIds(sgNode.nodeName);

				if (!instanceIds.empty())
				{
					MetaModel& newMd = matchedSubSSG->m_metaScene.m_metaModellList[instanceIds[0]];
					matchedSubSSG->m_metaScene.m_metaModellList.push_back(newMd);
					matchedSubSSG->m_graphNodeToModelListIdMap[currNodeId] = matchedSubSSG->m_metaScene.m_metaModellList.size()-1;
				}
				else
				{
					std::vector<QString> attriNames = querySSG->getAttriNamesForNode(ni);
					MetaModel& newMd = retrieveForModelInstance(sgNode.nodeName, attriNames);
					if (!newMd.name.empty())
					{
						matchedSubSSG->m_metaScene.m_metaModellList.push_back(newMd);
						matchedSubSSG->m_graphNodeToModelListIdMap[currNodeId] = matchedSubSSG->m_metaScene.m_metaModellList.size() - 1;
					}
					else
					{
						qDebug() << QString("Failure: can not find model %1 in database").arg(sgNode.nodeName);
					}
				}
			}
		}
	}

	// insert synthesized attribute and relation node
	for (int ni = 0; ni < querySSG->m_nodeNum; ni++)
	{
		SemNode& sgNode = querySSG->m_nodes[ni];

		if (!sgNode.isAligned && sgNode.matchingStatus == SemNode::ExplicitNode 
			&&(sgNode.nodeType == "attribute" || sgNode.nodeType == "group_relation"))
		{
			if (!sgNode.outEdgeNodeList.empty())
			{
				int refNodeId = sgNode.outEdgeNodeList[0];

				// only insert the relationship node for the object that is matched
				if (queryToSubSsgNodeMap.count(refNodeId))
				{
					// insert node
					matchedSubSSG->addNode(sgNode);
					int currNodeId = matchedSubSSG->m_nodeNum - 1;
					matchedSubSSG->m_nodes[currNodeId].isSynthesized = 1;

					matchedSubSSG->addEdge(currNodeId, queryToSubSsgNodeMap[refNodeId]);
					queryToSubSsgNodeMap[ni] = currNodeId;

					// TODO: add group objects

				}
			}
		}

		if (!sgNode.isAligned && (sgNode.nodeType == "pair_relation"))
		{
			if (!sgNode.anchorNodeList.empty())
			{
				int refNodeId = sgNode.anchorNodeList[0];
				int actNodeId = sgNode.activeNodeList[0];

				if (queryToSubSsgNodeMap.count(refNodeId) && queryToSubSsgNodeMap.count(actNodeId))
				{
					// insert node
					matchedSubSSG->addNode(sgNode);
					int currNodeId = matchedSubSSG->m_nodeNum - 1;
					matchedSubSSG->m_nodes[currNodeId].isSynthesized = 1;

					matchedSubSSG->addEdge(currNodeId, queryToSubSsgNodeMap[refNodeId]);
					matchedSubSSG->addEdge(queryToSubSsgNodeMap[actNodeId], currNodeId);

					if (queryToSubSsgNodeMap[refNodeId] == queryToSubSsgNodeMap[actNodeId])
					{
						qDebug();
					}

					queryToSubSsgNodeMap[ni] = currNodeId;
				}
			}
		}
	}

	matchedSubSSG->parseNodeNeighbors();
}

// enrich subgraph with context
void SemGraphMatcher::addSuppParentNodesToSubSSG(SceneSemGraph *matchedSubSSG, SceneSemGraph *dbSSG)
{
	std::vector<int> insertedParentNodeIds;

	// add support parent
	//int currNodeNum = matchedSubSSG->m_nodes.size();
	for (int i = 0; i < matchedSubSSG->m_nodes.size(); i++)
	{
		SemNode &mActNode = matchedSubSSG->m_nodes[i];

		if (mActNode.nodeType == "object")// && mActNode.nodeName == "tv")
		{
			bool isInSpecialRelation = false;
			for (int r = 0; r < mActNode.outEdgeNodeList.size(); r++)
			{
				int relId = mActNode.outEdgeNodeList[r];
				SemNode &relNode = matchedSubSSG->m_nodes[relId];
				if (relNode.nodeType.contains("group") || relNode.nodeName =="under")
				{
					isInSpecialRelation = true;
				}
			}

			if(isInSpecialRelation) continue;

			int dbActNodeId = getKeyForValueInMap(dbSSG->m_dbNodeToSubNodeMap, i);
			int dbActNodeParentNodeId = dbSSG->findParentNodeIdForNode(dbActNodeId);
			int mActNodeParentNodeId = matchedSubSSG->findParentNodeIdForNode(i);


			if (dbActNodeParentNodeId != -1 && dbActNodeParentNodeId!=0 && mActNodeParentNodeId == -1)
			{
				SemNode &dbActNode = dbSSG->m_nodes[dbActNodeId];

				for (int j = 0; j < dbActNode.outEdgeNodeList.size(); j++)
				{
					int dbRelNodeId = dbActNode.outEdgeNodeList[j];
					SemNode &dbRelNode = dbSSG->m_nodes[dbRelNodeId];
					if (dbRelNode.nodeName.contains("support"))
					{
						// add support node
						matchedSubSSG->addNode(dbRelNode);
						int currRelNodeId = matchedSubSSG->m_nodeNum - 1;
						dbSSG->m_dbNodeToSubNodeMap[dbRelNodeId] = currRelNodeId;
						matchedSubSSG->addEdge(i, currRelNodeId); // e.g., (tv, support)
						matchedSubSSG->m_nodes[currRelNodeId].isAligned = true;
						matchedSubSSG->m_nodes[currRelNodeId].isSynthesized = false;

						// add parent node
						int currAnchorNodeId;
						if (std::find(insertedParentNodeIds.begin(), insertedParentNodeIds.end(), dbActNodeParentNodeId) != insertedParentNodeIds.end())
						{
							currAnchorNodeId = dbSSG->m_dbNodeToSubNodeMap[dbActNodeParentNodeId];
						}
						else
						{
							matchedSubSSG->addNode(dbSSG->m_nodes[dbActNodeParentNodeId]);
							currAnchorNodeId = matchedSubSSG->m_nodeNum - 1;
							matchedSubSSG->m_nodes[currAnchorNodeId].inferedType = SemNode::InferBySupport;
							matchedSubSSG->m_nodes[currAnchorNodeId].isInferred = true;
							matchedSubSSG->m_nodes[currAnchorNodeId].inferRefNodeId = i;
							dbSSG->m_dbNodeToSubNodeMap[dbActNodeParentNodeId] = currAnchorNodeId;
							
							insertedParentNodeIds.push_back(dbActNodeParentNodeId);

							// add meta model
							MetaModel& newMd = dbSSG->getModelWithNodeId(dbActNodeParentNodeId);
							matchedSubSSG->m_metaScene.m_metaModellList.push_back(newMd);
							matchedSubSSG->m_graphNodeToModelListIdMap[currAnchorNodeId] = matchedSubSSG->m_metaScene.m_metaModellList.size() - 1;
						}

						matchedSubSSG->addEdge(currRelNodeId, currAnchorNodeId); // e.g. (support, tv_stand)
					}
				}
			}
		}
	}

	matchedSubSSG->parseNodeNeighbors();
}

void SemGraphMatcher::addContextNodesToSubSSG(SceneSemGraph *matchedSubSSG, SceneSemGraph *dbSSG)
{
	int subSSGNodeNum = matchedSubSSG->m_nodes.size();
	for (int i = 0; i < subSSGNodeNum; i++)
	{
		SemNode &mActNode = matchedSubSSG->m_nodes[i];
		QString mActObjName = mActNode.nodeName;
		if (matchedSubSSG->m_nodes[i].nodeType == "object")
		{
			int mParentNodeId = matchedSubSSG->findParentNodeIdForNode(i);
			if(mParentNodeId == -1) continue;
			SemNode &mParentNode = matchedSubSSG->m_nodes[mParentNodeId];
			QString mParentObjName = matchedSubSSG->m_nodes[mParentNodeId].nodeName;

			for (int j=0; j < dbSSG->m_nodes.size(); j++)
			{
				int dbActNodeId = j;
				// skip if node is already inserted
				if (dbSSG->m_dbNodeToSubNodeMap.count(dbActNodeId)) continue;

				SemNode &dbActNode = dbSSG->m_nodes[dbActNodeId];
				if (dbActNode.nodeType == "object")
				{
					QString dbActObjName = dbActNode.nodeName;
					if (matchedSubSSG->hasObj(dbActObjName)) continue; // do not insert repeatably

					int dbParentNodeId = dbSSG->findParentNodeIdForNode(dbActNodeId);
					// parent node must be already inserted in sub-scene
					if (!dbSSG->m_dbNodeToSubNodeMap.count(dbParentNodeId)) continue;

					if (mParentNodeId == dbSSG->m_dbNodeToSubNodeMap[dbParentNodeId])
					{

						double coOccProb = m_relModelManager->getCoOccProbOnParent(mActObjName, dbActObjName, mParentObjName);

						if (coOccProb >= params::inst()->contextCoOccProb)
						{
							matchedSubSSG->addNode(dbActNode);
							int currActNodeId = matchedSubSSG->m_nodeNum - 1;
							matchedSubSSG->m_nodes[currActNodeId].inferedType = SemNode::InferByContext;
							matchedSubSSG->m_nodes[currActNodeId].isInferred = true;
							matchedSubSSG->m_nodes[currActNodeId].inferRefNodeId = i;
							dbSSG->m_dbNodeToSubNodeMap[dbActNodeId] = currActNodeId;

							// add meta model
							MetaModel& newMd = dbSSG->getModelWithNodeId(dbActNodeId);
							matchedSubSSG->m_metaScene.m_metaModellList.push_back(newMd);
							matchedSubSSG->m_graphNodeToModelListIdMap[currActNodeId] = matchedSubSSG->m_metaScene.m_metaModellList.size() - 1;

							// add support node and edges
							for (int r=0; r < dbActNode.outEdgeNodeList.size(); r++)
							{
								int dbRelNodeId = dbActNode.outEdgeNodeList[r];
								SemNode &dbRelNode = dbSSG->m_nodes[dbRelNodeId];
								bool isFindParent = false;

								if (dbRelNode.nodeName.contains("support"))
								{
									for (int k=0; k < dbRelNode.outEdgeNodeList.size(); k++)
									{
										int anchorNodeId = dbRelNode.outEdgeNodeList[k];
										if (anchorNodeId == dbParentNodeId)
										{
											isFindParent = true;
											break;
										}
									}

									if (isFindParent)
									{
										// add support node
										matchedSubSSG->addNode(dbRelNode);
										int currRelNodeId = matchedSubSSG->m_nodeNum - 1;
										matchedSubSSG->addEdge(currActNodeId, currRelNodeId); // e.g., (tv, support)
										matchedSubSSG->addEdge(currRelNodeId, mParentNodeId); // e.g., (tv, support)


										matchedSubSSG->m_nodes[currRelNodeId].isAligned = true;
										matchedSubSSG->m_nodes[currRelNodeId].isSynthesized = false;
										dbSSG->m_dbNodeToSubNodeMap[dbRelNodeId] = currRelNodeId;
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	matchedSubSSG->parseNodeNeighbors();
}

MetaModel& SemGraphMatcher::retrieveForModelInstance(const QString catName, const std::vector<QString> attriNames)
{
	std::vector<std::pair<int, int>> exactMatchMdIds;
	std::vector<std::pair<int, int>> otherCandiMdIds;

	for (int i = 0; i < m_sceneSemGraphManager->m_ssgNum; i++)
	{
		SceneSemGraph *currDBSSG = m_sceneSemGraphManager->getGraph(i);

		for (int qNi = 0; qNi < currDBSSG->m_nodeNum; qNi++)
		{
			SemNode& sgNode = currDBSSG->m_nodes[qNi];
			if (sgNode.nodeType == "object" && sgNode.nodeName == catName)
			{
				int modelId = currDBSSG->m_graphNodeToModelListIdMap[qNi];
				MetaModel &md = currDBSSG->m_metaScene.m_metaModellList[modelId];

				// test front dir for book to filter stand book
				if (catName == "book" && md.frontDir.dot(vec3(0, 0, 1)) < 0.5)
				{
					continue;
				}

				if (!isModelInBlackList(toQString(md.name)))
				{
					// collect attri names for current node
					std::vector<QString> currAttriNames = currDBSSG->getAttriNamesForNode(qNi);

					// match attributes
					int mAttriNum = 0;
					for (int t=0; t<attriNames.size(); t++)
					{
						if (std::find(currAttriNames.begin(), currAttriNames.end(), attriNames[t])!=currAttriNames.end())
						{
							mAttriNum++;
						}
					}

					if (mAttriNum == attriNames.size())
					{
						exactMatchMdIds.push_back(std::make_pair(i, modelId));  // (graphId, modelId)
					}
					else
					{
						otherCandiMdIds.push_back(std::make_pair(i, modelId));
					}
				}				
			}
		}
	}

	if (!exactMatchMdIds.empty())
	{
		int randId = GenRandomInt(0, exactMatchMdIds.size());
		std::pair<int, int> selectPair = exactMatchMdIds[randId];

		SceneSemGraph *currDBSSG = m_sceneSemGraphManager->getGraph(selectPair.first);
		return currDBSSG->m_metaScene.m_metaModellList[selectPair.second];
	}
	else if(!otherCandiMdIds.empty())
	{
		int randId = GenRandomInt(0, otherCandiMdIds.size());
		std::pair<int, int> selectPair = otherCandiMdIds[randId];

		SceneSemGraph *currDBSSG = m_sceneSemGraphManager->getGraph(selectPair.first);
		return currDBSSG->m_metaScene.m_metaModellList[selectPair.second];
	}
	else
	{
		// Failure case: return a room for debug
		return MetaModel();
	}
}






