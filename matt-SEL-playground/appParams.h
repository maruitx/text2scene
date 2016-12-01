
struct Constants
{
	static const int a = 160;
};

struct AppParameters
{
	AppParameters()
	{
		debugDir = R"(C:\speech2speech\debug\run_)";
	}

	string debugDir;
};
