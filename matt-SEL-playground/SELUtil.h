
class SELUtil
{
public:
	static string processText(const string &document)
	{
		util::deleteFile(appParams().javaSceneDir + "output.txt");
		util::writeToFile(document, appParams().javaSceneDir + "input.txt");
		
		const string command = appParams().javaSceneDir + "processDocument.py";
		system(command.c_str());

		return util::getFileLines(appParams().javaSceneDir + "output.txt")[0];
	}
};