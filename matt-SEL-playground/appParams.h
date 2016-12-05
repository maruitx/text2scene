
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
	}

	string javaSceneDir;
	string debugDir;
	string dataDir;

	set<string> spatialNouns;
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
