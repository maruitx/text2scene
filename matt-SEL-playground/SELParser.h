
struct SELParser
{
	void parse(ParsedSentence &s);

private:
	void extractEntities(ParsedSentence &s);
	void assignDeterminers(ParsedSentence &s);
	void assignCounts(ParsedSentence &s);
	void assignAdjectives(ParsedSentence &s);
	void assignRelationships(ParsedSentence &s);

	void extractCommands(ParsedSentence &s);
	void assignAdverbs(ParsedSentence &s);
	void assignTargets(ParsedSentence &s);

	void addAdjective(ParsedSentence &s, int tokenIndex, const string &adjective);
	void addRelationship(ParsedSentence &s, int tokenIndexA, int tokenIndexB, string relationshipType);

	void addAdverb(ParsedSentence &s, int commandToken, const string &adverb);
	void addTarget(ParsedSentence &s, int commandToken, int entityToken, string type);
};