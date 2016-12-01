
struct Constants
{
	static const int a = 160;
};

struct AppParameters
{
	AppParameters()
	{
		javaSceneDir = R"(C:\Code\text2scene\JavaScene\)";
		debugDir = R"(C:\speech2speech\debug\run_)";
	}

	string javaSceneDir;
	string debugDir;
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
