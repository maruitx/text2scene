
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

	// One of the two values will be valid.

	// ex. "many", "some", etc.
	string descriptor;

	int count;
};

struct EntityRelationship
{
	// description of relationship. ex. "around", "on"
	string type;

	// the noun and index of sentence token of the referenced object.
	string referencedNoun;
	int referencedTokenIndex;
};

struct SceneEntity
{
	// the base nount describing the object. ex. "table", "computer"
	string baseNoun;

	// the index of baseNoun in the sentence token list.
	int tokenIndex;

	// adjectives describing the object. ex. "dining", "wooden"
	vector<string> adjectives;

	// describes the number of objects being referenced
	// ex. "there are four chairs"
	EntityCount count;

	// determiner used (ex. "a", "the", etc.)
	string determiner;

	vector<EntityRelationship> relationships;
};
