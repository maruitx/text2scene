
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
		applied = false;
	}

	string toString() const
	{
		string result;

		const string appliedDesc = applied ? " (applied)" : "";
		result += "command: " + baseVerb + "-" + to_string(tokenIndex) + appliedDesc + "\n";
		
		string targetDesc;
		for (auto &t : targets)
		{
			targetDesc += t.toString() + ",";
		}

		if(attributes.list.size() > 0)
			result += "  attributes: " + attributes.toString() + "\n";

		if (targets.size() > 0)
			result += "  targets: " + targetDesc + "\n";

		return result;
	}

	// the base verb describing the object. ex. "move", "rearrange"
	string baseVerb;

	// True if the command has been successfully applied to the corresponding entities, making the command largely unimportant.
	bool applied;

	// the index of baseVerb in the sentence token list.
	int tokenIndex;

	// attributes modifying the verb. ex. "closer", "together", "more messy"
	AttributeList attributes;
	
	vector<CommandTarget> targets;
};
