
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
		descriptor = defaultDescriptor();
		count = 1;
	}
	EntityCount(const string &value)
	{
		count = -1;
		descriptor = defaultDescriptor();

		for(int i = 0; i <= 10; i++)
			if (value == describeNumber(i) || value == to_string(i)) count = i;

		if (count != -1)
			descriptor = value;
	}
	static string defaultDescriptor()
	{
		return "*";
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

struct Attribute
{
	Attribute() {
		name = "<invalid>";
	}
	explicit Attribute(const string &_name) {
		name = _name;
	}
	string toString() const
	{
		if(modifiers.size() == 0)
			return name;
		return name + "(" + SELUtil::describeList(util::toStdVector(modifiers)) + ")";
	}
	string toStringSerial() const
	{
		if (modifiers.size() == 0)
			return name;
		return name + "|" + SELUtil::describeList(util::toStdVector(modifiers));
	}

	// base name of attribute. ex. "messy", "wooden"
	string name;

	// modifiers on the attribute. ex. "more", "less"
	set<string> modifiers;
};

struct AttributeList
{
	string toString() const
	{
		string result;
		for (auto &s : list)
		{
			result += s.toString() + ",";
		}
		return result;
	}

	bool hasAttribute(const string &name) const
	{
		for (auto &a : list)
			if (a.name == name)
				return true;
		return false;
	}

	Attribute& getAttribute(const string &name)
	{
		for (auto &a : list)
			if (a.name == name)
				return a;
		cout << "ERROR: Attribute not found: " << name << endl;
		return list[0];
	}

	void addAttribute(const string &attribute)
	{
		if (hasAttribute(attribute)) return;
		list.push_back(Attribute(attribute));
	}

	void addAttributeModifier(const string &attribute, const string &attributeModifier)
	{
		if (!hasAttribute(attribute))
		{
			cout << "Attribute not found: " << attribute << endl;
			return;
		}
		getAttribute(attribute).modifiers.insert(attributeModifier);
	}

	vector<Attribute> list;
};

struct SceneEntity
{
	SceneEntity()
	{
		baseNoun = "<invalid>";
		plural = false;
		tokenIndex = -1;
	}

	string toString() const
	{
		string result;
		const string pluralDesc = plural ? " (plural)" : "";
		string relationshipDesc;
		for (auto &s : relationships)
		{
			relationshipDesc += s.toString() + ",";
		}
		string determinerDesc;
		for (auto &s : determiners)
		{
			determinerDesc += s + ",";
		}

		result += "entity: " + baseNoun + "-" + to_string(tokenIndex) + pluralDesc + "\n";
		
		if(attributes.list.size() > 0)
			result += "  attributes: " + attributes.toString() + "\n";

		if(count.count != 1)
			result += "  count: " + count.toString() + "\n";

		if(determiners.size() > 0)
			result += "  determiners: " + determinerDesc + "\n";

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

	// attributes describing the object. ex. "dining", "wooden"
	AttributeList attributes;

	// describes the number of objects being referenced
	// ex. "there are four chairs"
	EntityCount count;

	// determiners used (ex. "a", "the", etc.)
	vector<string> determiners;

	vector<EntityRelationship> relationships;
};
