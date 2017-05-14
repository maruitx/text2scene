#pragma once

#include "Headers.h"
#include "SemanticGraph.h"
#include "TextSemGraphManager.h"

class RelationModelManager;

class TextSemGraph : public SemanticGraph
{
public:
	TextSemGraph(SelSentence s, RelationModelManager *relManager);
	~TextSemGraph();

	void buildGraphFromSEL();
	void mapNodeNameToFixedNameSet();
	void mapToFixedObjSet(QString &s);
	void mapToFixedRelationSet(SemNode &currNode, QString &nodeName, QString &nodeType = QString(""));
	void mapToFixedAttributeSet(QString &nodeName, QString &nodeType = QString(""));

	void addImplicitRelations(); // add implicit relations if no relations specified in the input
	void postProcessForSpecialRelations();

	QString convertToSinglarForm(const QString &s);
	bool isOnObj(int entityId, const QString &anchorName);
	bool isWithObj(const QString &currObjName, const QString &anchorName);

	void initAttriSets();
	bool isGoodAttribute(const QString &attriName);

	SelSentence m_sentence;   // only handle one sentence per graph	

private:
	int m_sentence_id;

	std::vector<int> m_isNodeCertain;
	std::vector<int> m_isNodeGrounded;
	std::vector<int> m_isEdgeGrounded;

	RelationModelManager *m_relModelManager;
	std::vector<QString> m_goodAttriSets;
};

