#include "TextSemGraph.h"
#include "RelationModelManager.h"
#include "Utility.h"

TextSemGraph::TextSemGraph(SelSentence s, RelationModelManager *relManager):
m_sentence(s), m_relModelManager(relManager)
{
	buildGraphFromSEL();
}

TextSemGraph::~TextSemGraph()
{
}

void TextSemGraph::buildGraphFromSEL()
{
	// add object node
	for (int i = 0; i < m_sentence.entityCount; i++)
	{
		QString nameString = QString(m_sentence.m_entities[i].nameString);
		std::vector<std::string> parts = PartitionString(nameString.toStdString(), "-");
		QString entityName = toQString(parts[0]);
		m_sentence.m_entities[i].nameString = entityName;

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
				}
			}
			else
			{
				if (instanceCountString == "some")
				{
					instCount = GenRandomInt(2, 5);
				}

				if (instanceCountString == "many")
				{
					instCount = GenRandomInt(3, 6);
				}
			}

			// special handling for books on shelf or cabinet
			if (entityName == "book")
			{
				if (isOnObj(i, "shelf") || isWithObj(entityName, "shelf"))
				{
					entityName = "standbooks";
					instCount = GenRandomInt(2, 3);
				}
					
				if (isOnObj(i, "cabinet") || isWithObj(entityName, "cabinet"))
				{
					entityName = "stackbooks";
					instCount = 1;
				}				
			}

			m_sentence.m_entities[i].instanceCount = instCount;
			for (int j = 0; j < m_sentence.m_entities[i].instanceCount; j++)
			{
				addNode("object", entityName);
				m_isNodeCertain.push_back(0);
				m_sentence.m_entities[i].m_instanceNodeIds.push_back(m_nodeNum - 1);
			}

			// update entity name
			m_sentence.m_entities[i].nameString = entityName;
		}		
		else
		{
			addNode("object", entityName);
			m_isNodeCertain.push_back(1);

			m_sentence.m_entities[i].m_instanceNodeIds.push_back(m_nodeNum - 1);
			m_sentence.m_entities[i].instanceCount = 1;
		}
	}

	// add relationship node
	for (int i = 0; i < m_sentence.entityCount; i++)
	{
		int relationCount = m_sentence.m_entities[i].relationshipCount;

		for (int j = 0; j < relationCount; j++)
		{
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

				std::vector<int> refNodeIds = findNodeWithName(anchorEntityName);
				for (int k = 0; k < refNodeIds.size(); k++)
				{
					addNode("relation", m_sentence.m_entities[i].m_relationships[j].nameString);

					int instanceNodeId = m_sentence.m_entities[i].m_instanceNodeIds[n];
					int currNodeId = m_nodeNum - 1;
					addEdge(instanceNodeId, currNodeId);
					addEdge(currNodeId, refNodeIds[k]);

					if (anchorEntityName == "wall")
					{
						m_nodes[currNodeId].nodeName= "horizonsupport";						
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

			{
				addNode("attribute", m_sentence.m_entities[i].m_attributes[j].nameString);

				std::vector<int> nodeIds = findNodeWithName(entityName);
				for (int k = 0; k < nodeIds.size(); k++)
				{
					addEdge(m_nodeNum - 1, nodeIds[k]);
				}
			}
		}
	}

	this->parseNodeNeighbors();

	mapNodeNameToFixedNameSet();
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

void TextSemGraph::mapToFixedObjSet(QString &nodeName)
{
	if (nodeName == "wall")
	{
		nodeName = "room";
		return;
	}

	if (nodeName == "sofa")
	{
		nodeName = "couch";
		return;
	}

	if (nodeName == "pc")
	{
		nodeName = "computer";
		return;
	}

	if (nodeName == "mouse")
	{
		nodeName = "computermouse";
		return;
	}

	if (nodeName.contains("shelf"))
	{
		nodeName = "bookcase";
		return;
	}

	if (nodeName == "headphone")
	{
		nodeName = "headphones";
		return;
	}

	if (nodeName == "socket")
	{
		nodeName = "powerstrip";
		return;
	}

	if (nodeName == "clock")
	{
		nodeName = "tableclock";
		return;
	}

	if (nodeName == "frame")
	{
		//nodeName = "picturefame";
		nodeName = "framework";
		return;
	}

	if (nodeName == "lamp")
	{
		nodeName = "desklamp";
		return;
	}

	if (nodeName == "cabinet")
	{
		nodeName = "filecabinet";
		return;
	}
}

void TextSemGraph::mapToFixedRelationSet(SemNode &currNode, QString &nodeName, QString &nodeType /*= QString("")*/)
{
	if (nodeName.contains("on") && !nodeName.contains("front"))
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

		nodeType = SSGNodeType[2];
		return;
	}

	if (nodeName.contains("with"))
	{

		nodeType = SSGNodeType[2];

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

		QString anchorName = m_nodes[refNodeId].nodeName;
		QString actName = m_nodes[actNodeId].nodeName;
		if (actName.contains("book")) actName = "book";

		QString suppRelKey1 = anchorName + "_" + actName + "_vertsupport";
		QString suppRelKey2 = anchorName + "_" + actName + "_horizonsupport";


		if (m_relModelManager->m_supportRelations.count(suppRelKey1))
		{
			nodeName = "vertsupport";
		}
		else if (m_relModelManager->m_supportRelations.count(suppRelKey2))
		{
			nodeName = "horizonsupport";
		}
		else if (m_relModelManager->m_suppProbs.count(actName) && m_relModelManager->m_suppProbs[actName].beChildProb >= 0.5
			&& m_relModelManager->m_suppProbs.count(anchorName) && m_relModelManager->m_suppProbs[anchorName].beParentProb >= 0.5)
		{
			nodeName = "vertsupport";
		}
		else
		{
			nodeName = "near";
		}

		return;
	}

	if (nodeName == "next to" || nodeName == "close to" || nodeName == "near")
	{
		nodeName = "near";
		nodeType = SSGNodeType[2];
		return;
	}

	if (nodeName.contains("front"))
	{
		nodeName = "front";
		nodeType = SSGNodeType[2];
		return;
	}

	if (nodeName.contains("back") || nodeName.contains("behind"))
	{
		nodeName = "back";
		nodeType = SSGNodeType[2];
		return;
	}

	if (nodeName.contains("left"))
	{
		nodeName = "leftside";
		nodeType = SSGNodeType[2];
		return;
	}

	if (nodeName.contains("right"))
	{
		nodeName = "rightside";
		nodeType = SSGNodeType[2];
		return;
	}

	if (nodeName.contains("under") || nodeName.contains("below"))
	{
		nodeName = "under";
		nodeType = SSGNodeType[2];
		return;
	}

	if (nodeName.contains("center"))
	{
		nodeName = "oncenter";
		nodeType = SSGNodeType[2];
		return;
	}

	if (nodeName.contains("align"))
	{
		nodeName = "aligned";
		nodeType = SSGNodeType[3];
		return;
	}

	if (nodeName.contains("stack"))
	{
		nodeName = "stacked";
		nodeType = SSGNodeType[3];
		return;
	}

	if (nodeName.contains("each") && nodeName.contains("side"))
	{
		nodeName = "leftside";
		nodeType = SSGNodeType[2];
		return;
	}

	if (nodeName == "around")
	{
		nodeType = SSGNodeType[3];
		return;
	}
}

void TextSemGraph::mapToFixedAttributeSet(QString &nodeName, QString &nodeType /*= QString("")*/)
{
	if (nodeName == "office")
	{
		nodeType = SSGNodeType[1];
		return;
	}

	if (nodeName == "coffee")
	{
		nodeType = SSGNodeType[1];
		return;
	}

	if (nodeName == "dining")
	{
		nodeType = SSGNodeType[1];
		return;
	}

	if (nodeName == "file")
	{
		nodeType = SSGNodeType[1];
		return;
	}

	if (nodeName == "round")
	{
		nodeType = SSGNodeType[1];
		return;
	}

	if (nodeName == "rectangular")
	{
		nodeType = SSGNodeType[1];
		return;
	}

	if (nodeName == "sauce")
	{
		nodeType = SSGNodeType[1];
		return;
	}

	if (nodeName.contains("mess"))
	{
		nodeType = SSGNodeType[3];
		nodeName = "messy";
		return;
	}
		

	if (nodeName.contains("mess"))
	{
		nodeType = SSGNodeType[3];
		nodeName = "messy";
		return;
	}

	if (nodeName.contains("formal"))
	{
		nodeType = SSGNodeType[3];
		nodeName = "formal";
		return;
	}

	if (nodeName.contains("casual"))
	{
		nodeType = SSGNodeType[3];
		nodeName = "casual";
		return;
	}

	if (nodeName.contains("organize"))
	{
		nodeType = SSGNodeType[3];
		nodeName = "organized";
		return;
	}

	if (nodeName.contains("clean"))
	{
		nodeType = SSGNodeType[3];
		nodeName = "clean";
		return;
	}
}





