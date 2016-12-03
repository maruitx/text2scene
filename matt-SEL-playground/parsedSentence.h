
struct ParseUnit
{
	ParseUnit() {}
	explicit ParseUnit(const string &s)
	{
		//nsubj@3@walking@0@Today@pB-tag
		const vector<string> parts = util::split(s, '@');
		type = parts[0];
		
		pAIndex = convert::toInt(parts[1]);
		pA = parts[2];

		pBIndex = convert::toInt(parts[3]);
		pB = parts[4];

		pBTag = parts[5];
	}
	string toString() const
	{
		return type + "(" + pA + "-" + to_string(pAIndex) + ", " + pB + "-" + to_string(pBIndex) + "~" + pBTag + ")";
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
	Token(int _index, string _text, string _tag)
	{
		index = _index;
		text = _text;
		tag = _tag;
	}
	int index;
	string text;
	string tag;
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

	ParsedSentence() {}
	explicit ParsedSentence(const string &s)
	{
		//Today I am walking to the bookstore.#nsubj@3@walking@0@Today^nummod@0@Today@1@I^aux@3@walking@2@am^root@-1@ROOT@3@walking^case@6@bookstore@4@to^det@6@bookstore@5@the^nmod:to@3@walking@6@bookstore^punct@3@walking@7@.^
		auto partsA = util::split(s, '#');
		text = partsA[0];
		auto partsB = util::split(partsA[1], '^');
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

	string text;
	vector<ParseUnit> units;
	vector<Token> tokens;
};
