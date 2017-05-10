#pragma once
#include "Headers.h"

class TextSemGraph;
class RelationModelManager;

struct SelRelationship
{
	int id;
	QString nameString;
	QString entityString;
};

struct SelAttribute
{
	int id;
	QString nameString;
};

struct SelCommand
{
	int id;
	QString verbString;
	bool isApplied;

	int attributeCount;
	vector<QString> attributeStrings;

	int targetCount;
	vector<QString> targetStrings;
};

struct SelEntity
{
	SelEntity() : attributeCount(0), relationshipCount(0), commandCount(0){};

	int id;
	QString nameString;  // e.g. chair-4

	bool isPlural;
	QString instanceCountString; // could be a number, or string e.g., many some
	int instanceCount;

	int attributeCount;
	int relationshipCount;
	int commandCount;

	vector<SelRelationship> m_relationships;
	vector<SelAttribute> m_attributes;
	vector<SelCommand> m_commands;

	QString m_determiner;  // empty, a or the

	vector<int> m_instanceNodeIds;  // the node id of current instance
};

struct SelSentence
{
	int id;
	QString textString;

	int entityCount;
	vector<SelEntity> m_entities;
};

class TextSemGraphManager
{
public:
	TextSemGraphManager();
	~TextSemGraphManager();

	void loadSELFromOutput(const QString &filename, RelationModelManager *relManager);
	void clearPreviousLoadSEL();

	void updateActiveGraphId();
	TextSemGraph* getActiveGraph() { return m_textSemGraphs[m_activeGraphId]; };

	
	int m_textGraphNum;

private:
	QString m_SEL_version;
	int m_sentence_count;

	vector<TextSemGraph*> m_textSemGraphs;
	int m_activeGraphId;
};

