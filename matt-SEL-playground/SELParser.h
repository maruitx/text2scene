
struct SELParser
{
	void parse(ParsedSentence &s);

private:
	void extractEntities(ParsedSentence &s);
	void assignDeterminers(ParsedSentence &s);
	void assignCounts(ParsedSentence &s);
	void assignAdjectives(ParsedSentence &s);
	void assignRelationships(ParsedSentence &s);
};