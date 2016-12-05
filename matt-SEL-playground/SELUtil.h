
class SELUtil
{
public:
	static string processText(const string &document)
	{
		const string outputFilename = appParams().javaSceneDir + "output.txt";
		util::deleteFile(outputFilename);
		util::writeToFile(document, appParams().javaSceneDir + "input.txt");
		
		const string command = appParams().javaSceneDir + "processDocument.py";
		system(command.c_str());

		if (!util::fileExists(outputFilename))
		{
			cout << "CoreNLP output file not found; Java parsing likely failed, check Java console." << endl;
			cin.get();
			return "";
		}

		auto lines = util::getFileLines(outputFilename);

		if (lines.size() == 0)
		{
			cout << "CoreNLP output file is empty; Java parsing likely failed, check Java console." << endl;
			cin.get();
			return "";
		}

		return lines[0];
	}
};