
struct Constants
{
	static const int a = 160;
};

struct AppParameters
{
	AppParameters()
	{
		javaSceneDir = R"(C:\Code\text2scene\JavaScene\)";
		debugDir = R"(TODO)";
		dataDir = R"(C:\Code\text2scene\data\SEL\)";

		for (auto &s : util::getFileLines(dataDir + "spatialNouns.txt", 2))
		{
			spatialNouns.insert(s);
		}

		for (auto &s : util::getFileLines(dataDir + "abstractNouns.txt", 2))
		{
			abstractNouns.insert(s);
		}
		for (auto &s : util::getFileLines(dataDir + "countingAdjectives.txt", 2))
		{
			countingAdjectives.insert(s);
		}
		for (auto &s : util::getFileLines(dataDir + "applicableVerbs.txt", 2))
		{
			applicableVerbs.insert(s);
		}
		for (auto &s : util::getFileLines(dataDir + "stopVerbs.txt", 2))
		{
			stopVerbs.insert(s);
		}
	}

	string javaSceneDir;
	string debugDir;
	string dataDir;

	bool isSpatialNoun(const string &s) const 
	{
		return (spatialNouns.count(s) != 0);
	}
	bool isAbstractNoun(const string &s) const
	{
		return (abstractNouns.count(s) != 0);
	}
	bool isAbstractOrSpatialNoun(const string &s) const
	{
		return (spatialNouns.count(s) != 0 || abstractNouns.count(s) != 0);
	}
	bool isCountingAdjective(const string &s) const
	{
		return (countingAdjectives.count(s) != 0);
	}
	bool isStopVerb(const string &s) const
	{
		return (stopVerbs.count(s) != 0);
	}
	bool isApplicableVerb(const string &s) const
	{
		return (applicableVerbs.count(s) != 0);
	}
	set<string> spatialNouns, abstractNouns, countingAdjectives, applicableVerbs, stopVerbs;
};

extern AppParameters* g_appParams;
inline const AppParameters& appParams()
{
	return *g_appParams;
}

inline AppParameters& appParamsMutable()
{
	return *g_appParams;
}
