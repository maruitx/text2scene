
struct SELParser
{
	void parse(ParsedSentence &s);

private:
	//
	// entities
	//
	void extractEntities(ParsedSentence &s);
	void assignDeterminers(ParsedSentence &s);
	void assignCounts(ParsedSentence &s);
	void assignEntityAttributes(ParsedSentence &s);
	void assignRelationships(ParsedSentence &s);
	void applyCommands(ParsedSentence &s);
	void applyCommand(ParsedSentence &s, const SceneCommand &c);

	void addEntityAttribute(ParsedSentence &s, int tokenIndex, const string &adjective);
	void addEntityAttributeModifier(ParsedSentence &s, int commandToken, const string &attribute, const string &modifier);
	void addRelationship(ParsedSentence &s, int tokenIndexA, int tokenIndexB, string relationshipType);

	//
	// commands
	//
	void extractCommands(ParsedSentence &s);
	void assignCommandAttributes(ParsedSentence &s);
	void assignCommandAttributeModifiers(ParsedSentence &s);
	void assignTargets(ParsedSentence &s);

	void addCommandAttribute(ParsedSentence &s, int commandToken, const string &attribute);
	void addCommandAttributeModifier(ParsedSentence &s, int commandToken, const string &attribute, const string &modifier);
	void addTarget(ParsedSentence &s, int commandToken, int entityToken, string type);
};