#include "TextSemGraph.h"
#include "RelationModelManager.h"
#include "Utility.h"

TextSemGraph::TextSemGraph(SelSentence s, RelationModelManager *relManager):
m_sentence(s), m_relModelManager(relManager)
{
	initAttriSets();
	buildGraphFromSEL();
}

TextSemGraph::~TextSemGraph()
{
}

void TextSemGraph::buildGraphFromSEL()
{
	// add object node
	std::vector<QString> addedObjNames;
	for (int i = 0; i < m_sentence.entityCount; i++)
	{
		QString nameString = QString(m_sentence.m_entities[i].nameString);
		if (nameString.contains("room") || nameString.contains("wall")) continue;  // do not consider room in current implementation

		std::vector<std::string> parts = PartitionString(nameString.toStdString(), "-");
		QString entityName = toQString(parts[0]);
		m_sentence.m_entities[i].nameString = entityName;
		QString determiner = m_sentence.m_entities[i].m_determiner;

		// for plural form
		if (m_sentence.m_entities[i].isPlural)
		{
			entityName = convertToSinglarForm(entityName);

			QString instanceCountString = m_sentence.m_entities[i].instanceCountString;
			bool isNumber;
			int instCount = instanceCountString.toInt(&isNumber);   // no specified instance count is also counted 1

			if (isNumber)
			{
				// since isPlural is true, add more instances; otherwise, use the given number
				if (instCount == 1)
				{
					instCount = GenRandomInt(2, 4);

					if (entityName == "monitor")
					{
						instCount = 1;
					}

					if (entityName == "keyboard")
					{
						instCount = 1;
					}
				}
			}
			else
			{
				if (instanceCountString == "some")
				{
					instCount = GenRandomInt(2, 4);
				}

				if (instanceCountString == "many")
				{
					instCount = GenRandomInt(3, 6);
				}

				if (entityName == "monitor")
				{
					instCount = 1;
				}

				if (entityName == "keyboard")
				{
					instCount = 1;
				}
			}

			// special handling for books on shelf or cabinet
			if (entityName == "book")
			{
				if (isOnObj(i, "shelf") || isWithObj(entityName, "shelf"))
				{
					entityName = "standbooks";
					instCount = GenRandomInt(2, 4);
				}
					
				if (isOnObj(i, "cabinet") || isWithObj(entityName, "cabinet"))
				{
					entityName = "stackbooks";
					instCount = 1;
				}				
			}

			// add one node if determiner is the, and adjust the actual count when binding with current scene
			if (determiner == "the")
			{
				instCount = 1;
			}


			m_sentence.m_entities[i].instanceCount = instCount;
			for (int j = 0; j < m_sentence.m_entities[i].instanceCount; j++)
			{
				addNode("object", entityName);
				m_isNodeCertain.push_back(0);

				int currNodeId = m_nodeNum - 1;
				m_sentence.m_entities[i].m_instanceNodeIds.push_back(currNodeId);

				SemNode &currNode = m_nodes[currNodeId];
				if (determiner != "the")
				{
					currNode.useNewInst = true;
				}
			}

			// update entity name
			m_sentence.m_entities[i].nameString = entityName;
			addedObjNames.push_back(entityName);
		}
		// for single form
		else
		{
			// if an entity appear multiple times, and some of them has determiner "the", then only insert one node for this instance
			if (determiner == "the" && 
				std::find(addedObjNames.begin(), addedObjNames.end(), entityName) != addedObjNames.end())
				continue;

			addNode("object", entityName);
			m_isNodeCertain.push_back(1);

			int currNodeId = m_nodeNum - 1;
			SemNode &currNode = m_nodes[currNodeId];
			if (determiner == "a")
			{
				currNode.useNewInst = true;
			}

			m_sentence.m_entities[i].m_instanceNodeIds.push_back(m_nodeNum - 1);
			m_sentence.m_entities[i].instanceCount = 1;
			addedObjNames.push_back(entityName);
		}
	}

	// add relationship node
	for (int i = 0; i < m_sentence.entityCount; i++)
	{
		QString nameString = QString(m_sentence.m_entities[i].nameString);
		if (nameString.contains("room") || nameString.contains("wall")) continue;  // do not consider room in current implementation

		int relationCount = m_sentence.m_entities[i].relationshipCount;

		std::vector<QString> currRelationStrings; // record current relation names; avoid duplicate
		for (int j = 0; j < relationCount; j++)
		{
			QString relationName = m_sentence.m_entities[i].m_relationships[j].nameString;
			QString relationRawString = m_sentence.m_entities[i].m_relationships[j].entityString;
			if (std::find(currRelationStrings.begin(), currRelationStrings.end(), relationRawString) != currRelationStrings.end())
			{
				continue;
			}

			currRelationStrings.push_back(relationRawString);

			for (int n = 0; n < m_sentence.m_entities[i].instanceCount; n++)
			{
				QString entityRawString = m_sentence.m_entities[i].m_relationships[j].entityString;
				std::vector<std::string> parts = PartitionString(entityRawString.toStdString(), "-");
				QString anchorEntityName = toQString(parts[0]);
				anchorEntityName = convertToSinglarForm(anchorEntityName);

				// special handling for books on shelf or cabinet
				if (anchorEntityName == "book")
				{
					if (isWithObj(anchorEntityName, "shelf")) anchorEntityName = "standbooks";
					if (isWithObj(anchorEntityName, "cabinet")) anchorEntityName = "stackbooks";
				}

				if (!relationName.contains("aligned"))
				{
					std::vector<int> refNodeIds = findNodeWithName(anchorEntityName);
					for (int k = 0; k < refNodeIds.size(); k++)
					{
						addNode("relation", relationName);

						int instanceNodeId = m_sentence.m_entities[i].m_instanceNodeIds[n];
						int currNodeId = m_nodeNum - 1;
						addEdge(instanceNodeId, currNodeId);
						addEdge(currNodeId, refNodeIds[k]);

						if (anchorEntityName == "wall")
						{
							m_nodes[currNodeId].nodeName = "horizonsupport";
						}
					}
				}
			}

			if (relationName.contains("aligned"))
			{
				std::vector<int> refNodeIds = findNodeWithName(nameString);

				if (refNodeIds.size() > 1)
				{
					int anchorNodeId = refNodeIds[0];
					for (int k = 1; k < refNodeIds.size(); k++)
					{
						addNode("relation", relationName);

						int currNodeId = m_nodeNum - 1;
						addEdge(refNodeIds[k], currNodeId);
						addEdge(currNodeId, refNodeIds[0]);
					}
				}
			}
		}
	}

	// add attribute node
	for (int i = 0; i < m_sentence.entityCount; i++)
	{
		int attributeCount = m_sentence.m_entities[i].attributeCount;
		QString entityName = QString(m_sentence.m_entities[i].nameString);

		for (int j = 0; j < attributeCount; j++)
		{

			QString attriName = m_sentence.m_entities[i].m_attributes[j].nameString;

			if(isGoodAttribute(attriName))
			{
				std::vector<int> nodeIds = findNodeWithName(entityName);
				for (int k = 0; k < nodeIds.size(); k++)
				{
					addNode("attribute", m_sentence.m_entities[i].m_attributes[j].nameString);
					addEdge(m_nodeNum - 1, nodeIds[k]);
				}
			}
		}
	}

	this->parseNodeNeighbors();

	mapNodeNameToFixedNameSet();

	addImplicitAttributes();
	addImplicitRelations();
	postProcessForSpecialRelations();
}

void TextSemGraph::mapNodeNameToFixedNameSet()
{
	for (int i = 0; i < m_nodeNum; i++)
	{
		if (m_nodes[i].nodeType == "object")
		{
			mapToFixedObjSet(m_nodes[i].nodeName);
		}

		if (m_nodes[i].nodeType.contains("relation"))
		{
			mapToFixedRelationSet(m_nodes[i], m_nodes[i].nodeName, m_nodes[i].nodeType);
		}

		if (m_nodes[i].nodeType == "attribute")
		{
			mapToFixedAttributeSet(m_nodes[i].nodeName, m_nodes[i].nodeType);
		}
	}

	parseNodeNeighbors();
}

QString TextSemGraph::convertToSinglarForm(const QString &s)
{
	QString singleS;
	// e.g. shelves
	if (s.endsWith("ves"))
	{
		singleS = s.left(s.length() - 3);
		singleS += "f";
	}
	// e.g. books
	else if (s.endsWith("s"))
	{
		singleS = s.left(s.length() - 1);
	}
	else
	{
		singleS = s;
	}

	return singleS;
}

bool TextSemGraph::isOnObj(int entityId, const QString &anchorName)
{
	int relationCount = m_sentence.m_entities[entityId].relationshipCount;

	for (int j = 0; j < relationCount; j++)
	{
		QString entityRawString = m_sentence.m_entities[entityId].m_relationships[j].entityString;
		if (entityRawString.contains(anchorName))
		{
			return true;
		}
	}

	return false;
}

bool TextSemGraph::isWithObj(const QString &currObjName, const QString &anchorName)
{
	// find anchor
	for (int i = 0; i < m_sentence.entityCount; i++)
	{
		QString nameString = QString(m_sentence.m_entities[i].nameString);
		if (nameString.contains(anchorName))
		{
			int relationCount = m_sentence.m_entities[i].relationshipCount;

			for (int j = 0; j < relationCount; j++)
			{
				QString entityRawString = m_sentence.m_entities[i].m_relationships[j].entityString;
				if (entityRawString.contains(currObjName))
				{
					return true;
				}
			}
		}
	}

	return false;
}

void TextSemGraph::initAttriSets()
{
	m_goodAttriSets.push_back("office");
	m_goodAttriSets.push_back("coffee");
	m_goodAttriSets.push_back("dining");
	m_goodAttriSets.push_back("file");
	m_goodAttriSets.push_back("round");
	m_goodAttriSets.push_back("rectangular");
	m_goodAttriSets.push_back("sauce");
	m_goodAttriSets.push_back("queen");
	m_goodAttriSets.push_back("sofa");

	m_goodAttriSets.push_back("messy");
	m_goodAttriSets.push_back("formal");
	m_goodAttriSets.push_back("casual");
	m_goodAttriSets.push_back("organized");
	m_goodAttriSets.push_back("clean");
}

bool TextSemGraph::isGoodAttribute(const QString &attriName)
{
	if (std::find(m_goodAttriSets.begin(), m_goodAttriSets.end(), attriName)!= m_goodAttriSets.end())
	{
		return true;
	}

	return false;
}

QString TextSemGraph::getDeterminerOfEntity(const QString &nodeName)
{
	QString determiner;

	for (int i = 0; i < m_sentence.entityCount; i++)
	{
		QString nameString = QString(m_sentence.m_entities[i].nameString);
		if (m_entityNameToNodeNameMap[nameString] == nodeName)
		{
			determiner = m_sentence.m_entities[i].m_determiner;
			return determiner;
		}
	}

	return determiner;
}

void TextSemGraph::mapToFixedObjSet(QString &nodeName)
{
	if (nodeName == "sofa")
	{
		nodeName = "couch";
		m_entityNameToNodeNameMap["sofa"] = "couch";
		return;
	}

	if (nodeName == "pc")
	{
		nodeName = "computer";
		m_entityNameToNodeNameMap["pc"] = "computer";

		return;
	}

	if (nodeName == "mouse")
	{
		nodeName = "computermouse";
		m_entityNameToNodeNameMap["mouse"] = "computermouse";
		return;
	}

	if (nodeName.contains("shelf"))
	{
		nodeName = "bookcase";
		m_entityNameToNodeNameMap["shelf"] = "bookcase";
		return;
	}

	if (nodeName == "headphone")
	{
		nodeName = "headphones";
		m_entityNameToNodeNameMap["headphone"] = "headphones";
		return;
	}

	if (nodeName == "socket")
	{
		nodeName = "powerstrip";
		m_entityNameToNodeNameMap["socket"] = "powerstrip";
		return;
	}

	if (nodeName == "clock")
	{
		nodeName = "tableclock";
		m_entityNameToNodeNameMap["clock"] = "tableclock";
		return;
	}

	if (nodeName == "frame")
	{
		//nodeName = "picturefame";
		nodeName = "framework";
		m_entityNameToNodeNameMap["frame"] = "framework";
		return;
	}

	if (nodeName == "lamp")
	{
		nodeName = "desklamp";
		m_entityNameToNodeNameMap["lamp"] = "desklamp";
		return;
	}

	if (nodeName == "cabinet")
	{
		nodeName = "filecabinet";
		m_entityNameToNodeNameMap["cabinet"] = "filecabinet";
		return;
	}
}

void TextSemGraph::mapToFixedRelationSet(SemNode &currNode, QString &nodeName, QString &nodeType /*= QString("")*/)
{
	if (nodeName.contains("on") && !nodeName.contains("front")&& !nodeName.contains("along"))
	{
		// TODO: infer for horizonsupport
		if (nodeName.contains("right"))
		{
			nodeName = "onright";
		}
		else if (nodeName.contains("left"))
		{
			nodeName = "onleft";
		}
		else
		{
			nodeName = "vertsupport";
		}

		nodeType = SSGNodeType[SemNode::Pair];
		return;
	}

	if (nodeName.contains("with"))
	{

		nodeType = SSGNodeType[SemNode::Pair];

		// reverse edge dir
		SemEdge &inEdge = getEdge(currNode.activeNodeList[0], currNode.nodeId);
		SemEdge &outEdge = getEdge(currNode.nodeId, currNode.anchorNodeList[0]);
		inEdge.reverseEdgeDir();
		outEdge.reverseEdgeDir();

		// reverse for current node
		std::swap(currNode.activeNodeList, currNode.anchorNodeList);
		std::swap(currNode.inEdgeNodeList, currNode.outEdgeNodeList);

		int refNodeId = currNode.anchorNodeList[0];
		int actNodeId = currNode.activeNodeList[0];

		// correct relationship node id saved in the object node
		int currNodeId = currNode.nodeId;
		EraseValueInVectorInt(m_nodes[refNodeId].outEdgeNodeList, currNodeId);
		m_nodes[refNodeId].inEdgeNodeList.push_back(currNodeId);

		EraseValueInVectorInt(m_nodes[actNodeId].inEdgeNodeList, currNodeId);
		m_nodes[actNodeId].outEdgeNodeList.push_back(currNodeId);

		QString anchorObjName = m_nodes[refNodeId].nodeName;
		QString actObjName = m_nodes[actNodeId].nodeName;
		if (actObjName.contains("book")) actObjName = "book";

		QString suppRelKey1 = anchorObjName + "_" + actObjName + "_vertsupport";
		QString suppRelKey2 = anchorObjName + "_" + actObjName + "_horizonsupport";


		if (m_relModelManager->m_supportRelations.count(suppRelKey1))
		{
			nodeName = "vertsupport";
		}
		else if (m_relModelManager->m_supportRelations.count(suppRelKey2))
		{
			nodeName = "horizonsupport";
		}
		else if (m_relModelManager->m_suppProbs.count(actObjName) && m_relModelManager->m_suppProbs[actObjName].beChildProb >= 0.5
			&& m_relModelManager->m_suppProbs.count(anchorObjName) && m_relModelManager->m_suppProbs[anchorObjName].beParentProb >= 0.5)
		{
			nodeName = "vertsupport";
		}
		else
		{
			nodeName = "near";
		}

		if ((anchorObjName == "tv" && actObjName== "speaker") ||
			(anchorObjName == "bed" && actObjName =="nightstand"))
		{
			nodeName = "side";
		}

		return;
	}

	if (nodeName == "next to" || nodeName == "close to" || nodeName == "near")
	{
		nodeName = "near";
		nodeType = SSGNodeType[SemNode::Pair];
		return;
	}

	if (nodeName.contains("front"))
	{
		nodeName = "front";
		nodeType = SSGNodeType[SemNode::Pair];
		return;
	}

	if (nodeName.contains("back") || nodeName.contains("behind"))
	{
		nodeName = "back";
		nodeType = SSGNodeType[SemNode::Pair];
		return;
	}

	if (nodeName.contains("left"))
	{
		nodeName = "leftside";
		nodeType = SSGNodeType[SemNode::Pair];
		return;
	}

	if (nodeName.contains("right"))
	{
		nodeName = "rightside";
		nodeType = SSGNodeType[SemNode::Pair];
		return;
	}

	if (nodeName.contains("under") || nodeName.contains("below"))
	{
		nodeName = "under";
		nodeType = SSGNodeType[SemNode::Pair];
		return;
	}

	if (nodeName.contains("center"))
	{
		nodeName = "oncenter";
		nodeType = SSGNodeType[SemNode::Pair];
		return;
	}

	if (nodeName.contains("stack"))
	{
		nodeName = "stacked";
		nodeType = SSGNodeType[SemNode::Group];
		return;
	}

	if (nodeName.contains("each") && nodeName.contains("side"))
	{
		nodeName = "leftside";
		nodeType = SSGNodeType[SemNode::Pair];
		return;
	}

	if (nodeName == "around")
	{
		nodeName = "pairaround";
		nodeType = SSGNodeType[SemNode::Pair];
		return;
	}

	if (nodeName.contains("align"))
	{
		nodeName = "pairaligned";
		nodeType = SSGNodeType[SemNode::Pair];
		return;
	}
}

void TextSemGraph::mapToFixedAttributeSet(QString &nodeName, QString &nodeType /*= QString("")*/)
{
	if (nodeName == "office")
	{
		nodeType = SSGNodeType[SemNode::Attri];
		return;
	}

	if (nodeName == "coffee")
	{
		nodeType = SSGNodeType[SemNode::Attri];
		return;
	}

	if (nodeName == "dining")
	{
		nodeType = SSGNodeType[SemNode::Attri];
		return;
	}

	if (nodeName == "file")
	{
		nodeType = SSGNodeType[SemNode::Attri];
		return;
	}

	if (nodeName == "round")
	{
		nodeType = SSGNodeType[SemNode::Attri];
		return;
	}

	if (nodeName == "rectangular")
	{
		nodeType = SSGNodeType[SemNode::Attri];
		return;
	}

	if (nodeName == "sauce")
	{
		nodeType = SSGNodeType[SemNode::Attri];
		return;
	}

	if (nodeName.contains("messy"))
	{
		nodeType = SSGNodeType[SemNode::Group];
		nodeName = "messy";
		return;
	}

	if (nodeName.contains("formal"))
	{
		nodeType = SSGNodeType[SemNode::Group];
		nodeName = "formal";
		return;
	}

	if (nodeName.contains("casual"))
	{
		nodeType = SSGNodeType[SemNode::Group];
		nodeName = "casual";
		return;
	}

	if (nodeName.contains("organize"))
	{
		nodeType = SSGNodeType[SemNode::Group];
		nodeName = "organized";
		return;
	}

	if (nodeName.contains("clean"))
	{
		nodeType = SSGNodeType[SemNode::Group];
		nodeName = "clean";
		return;
	}
}

void TextSemGraph::addImplicitAttributes()
{
	if (m_sentence.textString.contains("dining table"))
	{
		for (int i = 0; i < m_nodes.size(); i++)
		{
			SemNode &currNode = m_nodes[i];
			if (currNode.nodeName == "chair")
			{
				std::vector<QString> relNames;
				for (int r=0; r< currNode.inEdgeNodeList.size(); r++)
				{
					int relNodeId = currNode.inEdgeNodeList[r];
					SemNode &relNode = m_nodes[relNodeId];
					relNames.push_back(relNode.nodeName);
				}
			
				if (std::find(relNames.begin(), relNames.end(), "dining") == relNames.end())
				{
					addNode(SSGNodeType[SemNode::Attri], "dining");

					int currNodeId = m_nodeNum - 1;
					addEdge(currNodeId, i);
				}
			}
		}
	}

	parseNodeNeighbors();
}

void TextSemGraph::addImplicitRelations()
{
	for (int i=0; i < m_nodes.size(); i++)
	{
		SemNode &currNode = m_nodes[i];
		if (currNode.nodeName == "couch" && currNode.inEdgeNodeList.empty())
		{
			for (int j=0; j < m_nodes.size(); j++)
			{
				if(i==j) continue;
				SemNode &actNode = m_nodes[j];
				if (actNode.nodeName == "chair" && actNode.outEdgeNodeList.empty())
				{
					addNode(SSGNodeType[SemNode::Pair], "near");

					int currNodeId = m_nodeNum - 1;
					addEdge(j, currNodeId);
					addEdge(currNodeId, i);
				}
			}
		}
	}

	parseNodeNeighbors();
}

void TextSemGraph::postProcessForSpecialRelations()
{
	if (m_sentence.textString.contains("each side") || m_sentence.textString.contains("eachside"))
	{
		std::vector<int> relNodelIds;
		for (int i = 0; i < m_nodeNum; i++)
		{
			if (m_nodes[i].nodeName.contains("side"))
			{
				relNodelIds.push_back(i);
			}
		}

		for (int i=0; i< relNodelIds.size()/2; i++)
		{
			int relNodeId = relNodelIds[i];
			SemNode &relNode = m_nodes[relNodeId];
			
			if (!relNode.anchorNodeList.empty())
			{
				int anchorNodeId = relNode.anchorNodeList[0];
				SemNode &anchorNode = m_nodes[anchorNodeId];
				if (anchorNode.nodeName != "table")
				{
					m_nodes[relNodeId].nodeName = "leftside";
					m_nodes[relNodeId].nodeType = SSGNodeType[SemNode::Pair];
				}
			}
		}

		for (int i= relNodelIds.size()/2; i < relNodelIds.size(); i++)
		{

			int relNodeId = relNodelIds[i];
			SemNode &relNode = m_nodes[relNodeId];

			if (!relNode.anchorNodeList.empty())
			{
				int anchorNodeId = relNode.anchorNodeList[0];
				SemNode &anchorNode = m_nodes[anchorNodeId];
				if (anchorNode.nodeName != "table")
				{
					m_nodes[relNodeId].nodeName = "rightside";
					m_nodes[relNodeId].nodeType = SSGNodeType[SemNode::Pair];
				}
			}
		}
	}
}




