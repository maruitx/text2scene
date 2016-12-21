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
	for (int i = 0; i < m_sentence.entityCount; i++)
	{
		QString nameString = QString(m_sentence.m_entities[i].nameString);
		std::vector<std::string> parts = PartitionString(nameString.toStdString(), "-");

		addNode("object", QString(parts[0].c_str()));
	}
}

