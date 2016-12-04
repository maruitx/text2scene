
struct SELParser
{
	void parse(ParsedSentence &s);

private:
	void extractEntities(ParsedSentence &s);
	void assignDeterminers(ParsedSentence &s);
	void assignCounts(ParsedSentence &s);
	void assignAdjectives(ParsedSentence &s);
	void assignRelationships(ParsedSentence &s);

	void addAdjective(ParsedSentence &s, int tokenIndex, const string &adjective);
	void addRelationship(ParsedSentence &s, int tokenIndexA, int tokenIndexB, string relationshipType);
};