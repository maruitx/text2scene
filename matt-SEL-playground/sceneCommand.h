
struct CommandTarget
{
	// the type of connection:
	//  dobj = direct object
	string type;

	// the noun and index of sentence token of the targeted object
	string referencedNoun;
	int referencedTokenIndex;

	string toString() const
	{
		return type + ":" + referencedNoun + "-" + to_string(referencedTokenIndex);
	}
};

struct SceneCommand
{
	SceneCommand()
	{
		baseVerb = "<invalid>";
		tokenIndex = -1;
	}

	string toString() const
	{
		string result;
		result += "command: " + baseVerb + "-" + to_string(tokenIndex) + "\n";
		
		string adverbDesc;
		for (auto &s : adverbs)
		{
			adverbDesc += s + ",";
		}
		string targetDesc;
		for (auto &t : targets)
		{
			targetDesc += t.toString() + ",";
		}

		if(adverbs.size() > 0)
			result += "  adverbs: " + adverbDesc + "\n";

		if (targets.size() > 0)
			result += "  targets: " + targetDesc + "\n";

		return result;
	}

	// the base verb describing the object. ex. "move", "rearrange"
	string baseVerb;

	// the index of baseVerb in the sentence token list.
	int tokenIndex;

	// adverbs modifying the verb. ex. "closer", "together"
	vector<string> adverbs;

	vector<CommandTarget> targets;
};
