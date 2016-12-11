
struct ParseUnit
{
	ParseUnit() {}
	explicit ParseUnit(const string &s)
	{
		//nsubj@3@walking@0@Today@pB-tag
		const vector<string> parts = util::split(s, '@');
		type = parts[0];
		
		pAIndex = convert::toInt(parts[1]);
		pA = util::toLower(parts[2]);

		pBIndex = convert::toInt(parts[3]);
		pB = util::toLower(parts[4]);

		pBTag = parts[5];
	}
	string toString() const
	{
		return type + "(" + pA + "-" + to_string(pAIndex) + ", " + pB + "-" + to_string(pBIndex) + "~" + pBTag + ")";
	}
	string getTypeSuffix() const
	{
		return util::replace(util::split(type, ':')[1], '_', ' ');
	}

	string type;
	string pA;
	int pAIndex;
	string pB;
	int pBIndex;

	string pBTag;
};

struct Token
{
	Token() {}
	Token(int _index, const string &_text, const string &_posTag)
	{
		index = _index;
		text = util::toLower(_text);
		posTag = _posTag;
	}
	int index;
	string text;
	string posTag; // part-of-speech tag
};

struct ParsedSentence
{
	static vector<ParsedSentence> makeSentences(const string &input)
	{
		//Example input:
		//Today I am walking to the bookstore.#nsubj@3@walking@0@Today^nummod@0@Today@1@I^aux@3@walking@2@am^root@-1@ROOT@3@walking^case@6@bookstore@4@to^det@6@bookstore@5@the^nmod:to@3@walking@6@bookstore^punct@3@walking@7@.^|There are several red books on the top shelf.#expl@1@are@0@There^root@-1@ROOT@1@are^amod@4@books@2@several^amod@4@books@3@red^nsubj@1@are@4@books^case@8@shelf@5@on^det@8@shelf@6@the^amod@8@shelf@7@top^nmod:on@4@books@8@shelf^punct@1@are@9@.^|
		vector<ParsedSentence> result;
		for (auto &s : util::split(input, '|'))
		{
			if(s.size() >= 3)
				result.push_back(ParsedSentence(s));
		}
		return result;
	}

	static void serializeSentences(const vector<ParsedSentence> &sentences, const string &filenameOut)
	{
		ofstream file(filenameOut);
		file << "scene-editing-language-version:1.0" << endl;
		file << "sentence-count:" << sentences.size() << endl;
		for (auto &s : iterate(sentences))
		{
			file << "sentence-" << s.index << endl;
			s.value.writeToStream(file);
		}
		file << endl << "*** begin pretty print versions" << endl;
		for (auto &s : iterate(sentences))
		{
			file << "sentence-" << s.index << endl;
			file << s.value.text << endl;
			file << "entity count: " << s.value.entities.size() << endl;
			for (auto &e : iterate(s.value.entities))
			{
				file << e.value.toString();
			}

			if(s.value.commands.size() > 0) file << "command count: " << s.value.commands.size() << endl;
			for (auto &c : iterate(s.value.commands))
			{
				file << c.value.toString();
			}
			file << endl;
		}
	}

	ParsedSentence() {}
	explicit ParsedSentence(const string &s)
	{
		//Today I am walking to the bookstore.#nsubj@3@walking@0@Today^nummod@0@Today@1@I^aux@3@walking@2@am^root@-1@ROOT@3@walking^case@6@bookstore@4@to^det@6@bookstore@5@the^nmod:to@3@walking@6@bookstore^punct@3@walking@7@.^
		auto partsA = util::split(s, '#');
		text = partsA[0];
		auto partsB = util::split(partsA[1], '^');
		tokens.push_back(Token(0, "root", "root"));
		for (auto &unitDesc : partsB)
		{
			ParseUnit u = ParseUnit(unitDesc);
			units.push_back(u);
			tokens.push_back(Token(u.pBIndex, u.pB, u.pBTag));
		}
	}

	string toString() const
	{
		string result = text + "\n";
		for (auto &u : units)
		{
			result += u.toString() + "\n";
		}
		return result;
	}

	void writeToStream(ostream &os) const
	{
		os << text << endl;
		
		os << "entity-count:" << entities.size() << endl;
		for (auto &e : iterate(entities))
		{
			os << "entity-" << e.index << endl;
			os << "id:" << e.value.baseNoun << "-" << e.value.tokenIndex << endl;
			os << "plural:" << convert::toString(e.value.plural) << endl;
			os << "count:" << e.value.count.toString() << endl;
			os << "determiners:" << SELUtil::describeList(e.value.determiners) << endl;
			os << "attributes:" << e.value.attributes.list.size() << endl;
			for (auto &a : e.value.attributes.list)
			{
				os << a.toStringSerial() << endl;
			}
			os << "relationship-count:" << e.value.relationships.size() << endl;
			for (auto &r : e.value.relationships)
			{
				os << r.toString() << endl;
			}
		}

		os << "command-count:" << commands.size() << endl;
		for (auto &c : iterate(commands))
		{
			os << "command-" << c.index << endl;
			os << "verb:" << c.value.baseVerb << endl;
			os << "applied:" << convert::toString(c.value.applied) << endl;
			os << "attribute-count:" << c.value.attributes.list.size() << endl;
			for (auto &a : c.value.attributes.list)
			{
				os << a.toStringSerial() << endl;
			}
			os << "target-count:" << c.value.targets.size() << endl;
			for (auto &t : iterate(c.value.targets))
			{
				os << t.value.type << "|" << t.value.referencedNoun << "-" << t.value.referencedTokenIndex << endl;
			}
		}
	}

	vector<ParseUnit> findUnits(const string &typePrefix) const
	{
		vector<ParseUnit> result;
		for (auto &u : units)
		{
			if (util::startsWith(u.type, typePrefix))
				result.push_back(u);
		}
		return result;
	}

	vector<ParseUnit> findUnits(const string &typePrefix, const string &pATagPrefixes, const string &pBTagPrefixes) const
	{
		vector<ParseUnit> result;
		for (auto &u : units)
		{
			if (util::startsWith(u.type, typePrefix) &&
				SELUtil::partOfSpeechCheck(tokens[u.pAIndex].posTag, pATagPrefixes) &&
				SELUtil::partOfSpeechCheck(tokens[u.pBIndex].posTag, pBTagPrefixes))
				result.push_back(u);
		}
		return result;
	}

	vector<ParseUnit> findUnits(const string &typePrefix, int tokenIndexA, int tokenIndexB) const
	{
		vector<ParseUnit> result;
		for (auto &u : units)
		{
			if (util::startsWith(u.type, typePrefix) &&
				(tokenIndexA == -1 || tokenIndexA == u.pAIndex) &&
				(tokenIndexB == -1 || tokenIndexB == u.pBIndex))
				result.push_back(u);
		}
		return result;
	}

	SceneEntity* getEntity(int tokenIndex)
	{
		for (auto &e : entities)
		{
			if (e.tokenIndex == tokenIndex) return &e;
		}
		cout << "(WARNING) Entity not found: " << tokenIndex << endl;
		return nullptr;
	}

	SceneCommand* getCommand(int tokenIndex)
	{
		for (auto &c : commands)
		{
			if (c.tokenIndex == tokenIndex) return &c;
		}
		cout << "(WARNING) Command not found: " << tokenIndex << endl;
		return nullptr;
	}

	string text;
	vector<ParseUnit> units;
	vector<Token> tokens;

	vector<SceneEntity> entities;
	vector<SceneCommand> commands;
};
