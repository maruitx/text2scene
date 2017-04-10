#include "TextSemGraph.h"
#include "Utility.h"


TextSemGraph::TextSemGraph(SelSentence s):
m_sentence(s)
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
			QString instanceCountString = m_sentence.m_entities[i].instanceCountString;

			if (entityName != "book" && entityName != "books" && instanceCountString != "some")
			{
				entityName = convertToSinglarForm(entityName);
			}

			bool isNumber;
			int instCount = instanceCountString.toInt(&isNumber);

			if (isNumber)
			{
				if (instCount == 1 && entityName != "books")
				{
					instCount = GenRandomInt(2, 5);
				}

				m_sentence.m_entities[i].instanceCount = instCount;
				for (int j = 0; j < m_sentence.m_entities[i].instanceCount; j++)
				{
					addNode("object", entityName);
					m_isNodeCertain.push_back(1);

					m_sentence.m_entities[i].m_instanceNodeIds.push_back(m_nodeNum-1);
				}
			}
			else
			{
				if (instanceCountString == "some")
				{
					// Debug: temp set instance number to be 4 for uncertain node
					instCount = GenRandomInt(2, 4);
					m_sentence.m_entities[i].instanceCount = instCount;
					for (int j = 0; j < m_sentence.m_entities[i].instanceCount; j++)
					{
						addNode("object", entityName);
						m_isNodeCertain.push_back(0);
						m_sentence.m_entities[i].m_instanceNodeIds.push_back(m_nodeNum - 1);
					}
				}
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

				std::vector<int> refNodeIds = findNodeWithName(anchorEntityName);
				for (int k = 0; k < refNodeIds.size(); k++)
				{
					addNode("relation", m_sentence.m_entities[i].m_relationships[j].nameString);

					int instanceNodeId = m_sentence.m_entities[i].m_instanceNodeIds[n];
					addEdge(instanceNodeId, m_nodeNum - 1);
					addEdge(m_nodeNum - 1, refNodeIds[k]);

					if (anchorEntityName == "wall")
					{
						m_nodes[m_nodeNum - 1].nodeType = "relation:horizon_support";
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

void TextSemGraph::mapToFixedObjSet(QString &nodeName)
{
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
		nodeName = "powersocket";
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
	if (nodeName.contains("on"))
	{
		if (nodeType.contains("horizon"))
		{
			nodeName = "horizonsupport";
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
		nodeName = "vertsupport";

		// reverse for current node
		std::swap(currNode.activeNodeList, currNode.anchorNodeList);
		std::swap(currNode.inEdgeNodeList, currNode.outEdgeNodeList);

		int refNodeId = currNode.anchorNodeList[0];
		int actNodeId = currNode.activeNodeList[0];

		// correct relationship node id saved in the object node
		int currNodeId = currNode.nodeId;
		EraseValueInVectorInt(m_nodes[refNodeId].outEdgeNodeList, currNodeId);
		m_nodes[refNodeId].inEdgeNodeList.push_back(currNodeId);

		EraseValueInVectorInt(m_nodes[refNodeId].inEdgeNodeList, currNodeId);
		m_nodes[actNodeId].outEdgeNodeList.push_back(currNodeId);

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
		nodeName = "left";
		nodeType = SSGNodeType[2];
		return;
	}

	if (nodeName.contains("right"))
	{
		nodeName = "right";
		nodeType = SSGNodeType[2];
		return;
	}

	if (nodeName.contains("under || below"))
	{
		nodeName = "under";
		nodeType = SSGNodeType[2];
		return;
	}

	if (nodeName.contains("center"))
	{
		nodeName = "center";
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
		nodeName = "left";
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





