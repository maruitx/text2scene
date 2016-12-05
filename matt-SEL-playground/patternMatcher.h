
struct PatternMatchQuery
{
	PatternMatchQuery() {}
	PatternMatchQuery(const string &u0)
	{
		units.push_back(UnitEntry(u0));
		finalize();
	}
	PatternMatchQuery(const string &u0, const string &u1)
	{
		units.push_back(UnitEntry(u0));
		units.push_back(UnitEntry(u1));
		finalize();
	}
	PatternMatchQuery(const string &u0, const string &u1, const string &u2)
	{
		units.push_back(UnitEntry(u0));
		units.push_back(UnitEntry(u1));
		units.push_back(UnitEntry(u2));
		finalize();
	}
	struct UnitEntry
	{
		UnitEntry() {}
		explicit UnitEntry(string s)
		{
			//nmod:(0-VB, 2-NN)
			s = util::remove(util::remove(s, " "), ")");
			auto p0 = util::split(s, "(");
			typePrefix = p0[0];
			auto p1 = util::split(p0[1], ",");

			auto p2 = util::split(p1[0], "-");
			tAIndex = convert::toInt(p2[0]);
			tAPrefix = p2[1];

			auto p3 = util::split(p1[1], "-");
			tBIndex = convert::toInt(p3[0]);
			tBPrefix = p3[1];
		}

		bool matches(const ParsedSentence &s, const ParseUnit &u) const
		{
			if (util::startsWith(u.type, typePrefix) &&
				util::startsWith(s.tokens[u.pAIndex].posTag, tAPrefix) &&
				util::startsWith(s.tokens[u.pBIndex].posTag, tBPrefix))
			{
				return true;
			}
			return false;
		}

		string typePrefix;
		int tAIndex;
		string tAPrefix;
		int tBIndex;
		string tBPrefix;
	};

	vector<UnitEntry> units;
	int tokenCount;

private:
	void finalize()
	{
		tokenCount = -1;
		for (auto &u : units)
		{
			tokenCount = std::max(tokenCount, u.tAIndex);
			tokenCount = std::max(tokenCount, u.tBIndex);
		}
		tokenCount++;
	}
};

struct PatternMatchResult
{
	vector<int> units;
	vector<int> tokens;
};

struct PatternMatcher
{
	static vector<PatternMatchResult> match(const ParsedSentence &s, const PatternMatchQuery &query)
	{
		if (query.units.size() == 2)
		{
			return match2(s, query);
		}
		if (query.units.size() == 3)
		{
			return match3(s, query);
		}
		return vector<PatternMatchResult>();
	}

private:
	static vector<PatternMatchResult> match2(const ParsedSentence &s, const PatternMatchQuery &query)
	{
		vector<PatternMatchResult> results;
		for (auto &candidateU0 : iterate(s.units))
		{
			for (auto &candidateU1 : iterate(s.units))
			{
				if (query.units[0].matches(s, candidateU0.value) &&
					query.units[1].matches(s, candidateU1.value))
				{
					// parts-of-speech and types match, try to assign token indices

					auto getCandidateUnit = [&](size_t x) -> const ParseUnit&{
						if (x == 0) return candidateU0.value;
						else return candidateU1.value;
					};

					PatternMatchResult result;
					result.units.push_back((int)candidateU0.index);
					result.units.push_back((int)candidateU1.index);

					result.tokens.resize(query.tokenCount, -1);
					bool match = true;
					for (auto &uQuery : iterate(query.units))
					{
						auto assignToken = [&](int variableIndex, int tokenAssignment) {
							if (result.tokens[variableIndex] == -1)
							{
								// variable not yet assigned, give it new assignment
								result.tokens[variableIndex] = tokenAssignment;
							}
							else
							{
								// variable already assigned, varify assignment matches
								if (result.tokens[variableIndex] != tokenAssignment)
									match = false;
							}
						};

						assignToken(uQuery.value.tAIndex, getCandidateUnit(uQuery.index).pAIndex);
						assignToken(uQuery.value.tBIndex, getCandidateUnit(uQuery.index).pBIndex);
					}

					if (match)
					{
						results.push_back(result);
					}
				}
			}
		}
		return results;
	}

	static vector<PatternMatchResult> match3(const ParsedSentence &s, const PatternMatchQuery &query)
	{
		vector<PatternMatchResult> results;
		for (auto &candidateU0 : iterate(s.units))
		{
			for (auto &candidateU1 : iterate(s.units))
			{
				for (auto &candidateU2 : iterate(s.units))
				{
					if (query.units[0].matches(s, candidateU0.value) &&
						query.units[1].matches(s, candidateU1.value) &&
						query.units[2].matches(s, candidateU2.value))
					{
						// parts-of-speech and types match, try to assign token indices

						auto getCandidateUnit = [&](size_t x) -> const ParseUnit&{
							if (x == 0) return candidateU0.value;
							else if (x == 1) return candidateU1.value;
							else return candidateU2.value;
						};

						PatternMatchResult result;
						result.units.push_back((int)candidateU0.index);
						result.units.push_back((int)candidateU1.index);
						result.units.push_back((int)candidateU2.index);

						result.tokens.resize(query.tokenCount, -1);
						bool match = true;
						for (auto &uQuery : iterate(query.units))
						{
							auto assignToken = [&](int variableIndex, int tokenAssignment) {
								if (result.tokens[variableIndex] == -1)
								{
									// variable not yet assigned, give it new assignment
									result.tokens[variableIndex] = tokenAssignment;
								}
								else
								{
									// variable already assigned, varify assignment matches
									if (result.tokens[variableIndex] != tokenAssignment)
										match = false;
								}
							};

							assignToken(uQuery.value.tAIndex, getCandidateUnit(uQuery.index).pAIndex);
							assignToken(uQuery.value.tBIndex, getCandidateUnit(uQuery.index).pBIndex);
						}

						if (match)
						{
							results.push_back(result);
						}
					}
				}
			}
		}
		return results;
	}
};
