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

private:
	int m_sentence_id;
	int m_sentence_string;

	SelSentence m_sentence;   // only handle one sentence per graph	
};

