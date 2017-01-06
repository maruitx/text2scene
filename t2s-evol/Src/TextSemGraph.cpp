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
		QString entityName = QString(parts[0].c_str());

		if (m_sentence.m_entities[i].isPlural)
		{
			entityName = convertToSinglarForm(entityName);
			
			QString instanceCountString = m_sentence.m_entities[i].instanceCountString;

			bool isNumber;
			m_sentence.m_entities[i].instanceCount = instanceCountString.toInt(&isNumber);

			if (isNumber)
			{
				for (int j = 0; j < m_sentence.m_entities[i].instanceCount; j++)
				{
					addNode("object", entityName);
					m_isNodeCertain.push_back(1);

					m_sentence.m_entities[i].m_instanceNodeIds.push_back(m_nodeNum-1);
				}
			}
			else
			{
				// Debug: temp set instance number to be 4 for uncertain node 
				m_sentence.m_entities[i].instanceCount = 4;
				for (int j = 0; j < m_sentence.m_entities[i].instanceCount; j++)
				{
					addNode("object", entityName);
					m_isNodeCertain.push_back(0);

					m_sentence.m_entities[i].m_instanceNodeIds.push_back(m_nodeNum - 1);
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
				addNode("relation", m_sentence.m_entities[i].m_relationships[j].nameString);

				QString entityRawString = m_sentence.m_entities[i].m_relationships[j].entityString;
				std::vector<std::string> parts = PartitionString(entityRawString.toStdString(), "-");
				QString passiveEntityName = QString(parts[0].c_str());
				passiveEntityName = convertToSinglarForm(passiveEntityName);

				if (passiveEntityName == "wall")
				{
					m_nodes[m_nodeNum - 1].nodeType = "relation:horizon_support";
				}

				std::vector<int> passiveNodeIds = findNodeWithName(passiveEntityName);
				for (int k = 0; k < passiveNodeIds.size(); k++)
				{
					int instanceNodeId = m_sentence.m_entities[i].m_instanceNodeIds[n];
					addEdge(m_nodeNum - 1, instanceNodeId);
					addEdge(passiveNodeIds[k], m_nodeNum - 1);
				}
			}
		}
	}

	// add attribute node
	for (int i = 0; i < m_sentence.entityCount; i++)
	{
		int attributeCount = m_sentence.m_entities[i].attributeCount;
		QString currEntityname = m_sentence.m_entities[i].nameString;

		for (int j = 0; j < attributeCount; j++)
		{
			addNode("attribute", m_sentence.m_entities[i].m_attributes[j].nameString);

			std::vector<int> nodeIds = findNodeWithName(currEntityname);
			for (int k = 0; k < nodeIds.size(); k++)
			{
				addEdge(m_nodeNum - 1, nodeIds[k]);
			}
		}
	}

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
			mapToFixedRelationSet(m_nodes[i].nodeName, m_nodes[i].nodeType);
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
	}

	if (nodeName == "pc")
	{
		nodeName = "computer";
	}

	if (nodeName == "mouse")
	{
		nodeName = "computermouse";
	}
}

void TextSemGraph::mapToFixedRelationSet(QString &nodeName, QString &nodeType /*= QString("")*/)
{
	if (nodeName.contains("on"))
	{
		if (nodeType.contains("horizon"))
		{
			nodeName = "horizon_support";
		}
		else
		{
			nodeName = "vert_support";
		}
		nodeType = "pairwise_relationship";
	}

	if (nodeName == "around")
	{
		nodeType = "group_relationship";
	}
}

void TextSemGraph::mapToFixedAttributeSet(QString &nodeName, QString &nodeType /*= QString("")*/)
{

}

void TextSemGraph::checkEdgeDir()
{

}

