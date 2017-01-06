#pragma once

#include "Headers.h"
#include "SemanticGraph.h"
#include "TextSemGraphManager.h"

class TextSemGraph : public SemanticGraph
{
public:
	TextSemGraph(SelSentence s);
	~TextSemGraph();

	void buildGraphFromSEL();
	void mapNodeNameToFixedNameSet();
	void mapToFixedObjSet(QString &s);
	void mapToFixedRelationSet(QString &nodeName, QString &nodeType = QString(""));
	void mapToFixedAttributeSet(QString &nodeName, QString &nodeType = QString(""));
	void checkEdgeDir();

	QString convertToSinglarForm(const QString &s);

private:
	int m_sentence_id;
	int m_sentence_string;

	SelSentence m_sentence;   // only handle one sentence per graph	

	std::vector<int> m_isNodeCertain;
	std::vector<int> m_isNodeGrounded;
	std::vector<int> m_isEdgeGrounded;

};

