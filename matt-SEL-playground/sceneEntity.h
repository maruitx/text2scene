
struct EntityCount
{
	static string describeNumber(int x)
	{
		if (x == 0) return "zero";
		if (x == 1) return "one";
		if (x == 2) return "two";
		if (x == 3) return "three";
		if (x == 4) return "four";
		if (x == 5) return "five";
		if (x == 6) return "six";
		if (x == 7) return "seven";
		if (x == 8) return "eight";
		if (x == 9) return "nine";
		if (x == 10) return "ten";
		return "error";
	}
	EntityCount()
	{
		descriptor = "*";
		count = 1;
	}
	EntityCount(const string &value)
	{
		count = -1;
		descriptor = "*";

		for(int i = 0; i <= 10; i++)
			if (value == describeNumber(i) || value == to_string(i)) count = i;

		if (count != -1)
			descriptor = value;
	}

	string toString() const
	{
		if (count == -1) return descriptor;
		else return to_string(count);
	}

	// One of the two values will be valid.

	// ex. "many", "some", etc.
	string descriptor;

	// -1 = invalid count
	int count;
};

struct EntityRelationship
{
	// description of relationship. ex. "around", "on"
	string type;

	// the noun and index of sentence token of the referenced object.
	string referencedNoun;
	int referencedTokenIndex;

	string toString() const
	{
		return type + ":" + referencedNoun + "-" + to_string(referencedTokenIndex);
	}
};

struct SceneEntity
{
	SceneEntity()
	{
		baseNoun = "<invalid>";
		plural = false;
		tokenIndex = -1;
		determiner = "<not specified>";
	}

	string toString() const
	{
		string result;
		const string pluralDesc = plural ? " (plural)" : "";
		string adjectiveDesc;
		for (auto &s : adjectives)
		{
			adjectiveDesc += s + ",";
		}
		string relationshipDesc;
		for (auto &s : relationships)
		{
			relationshipDesc += s.toString() + ",";
		}

		result += "entity: " + baseNoun + "-" + to_string(tokenIndex) + pluralDesc + "\n";
		
		if(adjectives.size() > 0)
			result += "  adjectives: " + adjectiveDesc + "\n";

		if(count.count != 1)
			result += "  count: " + count.toString() + "\n";

		if(determiner != "<not specified>")
			result += "  determiner: " + determiner + "\n";

		if(relationships.size() > 0)
			result += "  relationships: " + relationshipDesc + "\n";

		return result;
	}

	// the base nount describing the object. ex. "table", "computer"
	string baseNoun;

	// the index of baseNoun in the sentence token list.
	int tokenIndex;

	// whether the noun is plural
	bool plural;

	// adjectives describing the object. ex. "dining", "wooden"
	vector<string> adjectives;

	// describes the number of objects being referenced
	// ex. "there are four chairs"
	EntityCount count;

	// determiner used (ex. "a", "the", etc.)
	string determiner;

	vector<EntityRelationship> relationships;
};
