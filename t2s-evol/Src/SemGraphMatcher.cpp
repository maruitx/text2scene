#include "SemGraphMatcher.h"
#include "SceneSemGraphManager.h"
#include "RelationModelManager.h"
#include "SceneSemGraph.h"
#include "TextSemGraph.h"

const bool UseContext = false;
const bool AddSynthesizeNode = true;


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

	for (int i = 0; i < m_sceneSemGraphManager->m_ssgNum; i++)
	{
		SceneSemGraph *currDBSSG = m_sceneSemGraphManager->getGraph(i);

		double matchingScore = 0;
		SceneSemGraph *subSSG = alignSSGWithDBSSG(m_querySSG, currDBSSG, matchingScore);

		if (matchingScore > 0 && !ssgContainsBlackListModel(subSSG))
		{
			scoredDBSubSSGs.push_back(make_pair(matchingScore, subSSG));
			if (matchingScore == exactMatchScore)
			{
				exactMatchNum++;
			}
		}
	}

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

	// align object nodes
	querySSG->m_nodeAlignMap.clear();
	querySSG->setNodesUnAligned();
	dbSSG->setNodesUnAligned();

	querySSG->alignObjectNodesWithGraph(dbSSG, matchingScore);

	if (matchingScore == 0) return NULL;

	// align relationship nodes
	querySSG->alignRelationNodesWithGraph(dbSSG, matchingScore);

	// collect matched nodes and generate subgraph
	std::vector<int> matchedDBSsgNodeList;
	for (int ni = 0; ni < querySSG->m_nodeNum; ni++)
	{
		SemNode& sgNode = querySSG->m_nodes[ni];
		
		if (sgNode.isAligned && querySSG->m_nodeAlignMap.count(ni))
		{
			int matchedDBSsgId = querySSG->m_nodeAlignMap[ni];
			matchedDBSsgNodeList.push_back(matchedDBSsgId);
		}
	}

	// add nodes by exact match and scene context
	matchedSubSSG = dbSSG->getSubGraph(matchedDBSsgNodeList, m_relModelManager, UseContext);

	if (AddSynthesizeNode)
	{
		addSynthNodeToSubSSG(querySSG, matchedSubSSG, dbSSG->m_dbNodeToSubNodeMap);
	}

	return matchedSubSSG;
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

void SemGraphMatcher::addSynthNodeToSubSSG(SemanticGraph *querySSG, SceneSemGraph *matchedSubSSG, std::map<int, int> &dbNodeToSubNodeMap)
{
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
					MetaModel& newMd = retrieveForModelInstance(sgNode.nodeName);
					matchedSubSSG->m_metaScene.m_metaModellList.push_back(newMd);
					matchedSubSSG->m_graphNodeToModelListIdMap[currNodeId] = matchedSubSSG->m_metaScene.m_metaModellList.size() - 1;
				}
			}
		}
	}

	// insert synthesized attribute and relation node
	for (int ni = 0; ni < querySSG->m_nodeNum; ni++)
	{
		SemNode& sgNode = querySSG->m_nodes[ni];

		if (!sgNode.isAligned && (sgNode.nodeType == "attribute" || sgNode.nodeType == "group_relation"))
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

MetaModel& SemGraphMatcher::retrieveForModelInstance(const QString catName)
{
	std::vector<std::pair<int, int>> candiMdIds;

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
				if (!isModelInBlackList(toQString(md.name)))
				{
					candiMdIds.push_back(std::make_pair(i, modelId));
				}				
			}
		}
	}

	int randId = GenRandomInt(0, candiMdIds.size());
	std::pair<int, int> selectPair = candiMdIds[randId];
	SceneSemGraph *currDBSSG = m_sceneSemGraphManager->getGraph(selectPair.first);
	return currDBSSG->m_metaScene.m_metaModellList[selectPair.second];
}






