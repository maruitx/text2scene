
struct Constants
{
	static const int a = 160;
};

struct AppParameters
{
	AppParameters()
	{
		ParameterFile params("SEL-params.txt");

		params.readParameter("gitRoot", gitRoot);
		params.readParameter("inputFile", inputFile);
		params.readParameter("outputFile", outputFile);
		

		javaSceneDir = gitRoot + "JavaScene/";
		dataDir = gitRoot + "data/SEL/";
		debugDir = R"(TODO)";
		
		auto readFileList = [&](const string &filename) {
			set<string> result;
			for (auto &s : util::getFileLines(dataDir + filename, 2))
			{
				result.insert(s);
			}
			return result;
		};

		spatialNouns = readFileList("spatialNouns.txt");
		abstractNouns = readFileList("abstractNouns.txt");
		countingAdjectives = readFileList("countingAdjectives.txt");
		applicableVerbs = readFileList("applicableVerbs.txt");
		stopVerbs = readFileList("stopVerbs.txt");
		groupNouns = readFileList("groupNouns.txt");
	}

	bool isSpatialNoun(const string &s) const 
	{
		return (spatialNouns.count(s) != 0);
	}
	bool isAbstractNoun(const string &s) const
	{
		return (abstractNouns.count(s) != 0);
	}
	bool isGroupNoun(const string &s) const
	{
		return (groupNouns.count(s) != 0);
	}
	bool isSpecialNoun(const string &s) const
	{
		return (spatialNouns.count(s) != 0 || abstractNouns.count(s) != 0 || groupNouns.count(s) != 0);
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

	string gitRoot;
	string javaSceneDir;
	string debugDir;
	string dataDir;

	string inputFile;
	string outputFile;

	set<string> spatialNouns, abstractNouns, countingAdjectives, applicableVerbs, stopVerbs, groupNouns;
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
