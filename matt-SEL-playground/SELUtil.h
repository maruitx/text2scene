
class SELUtil
{
public:
	static string makeVerbApplied(const string &v)
	{
		if (v == "put") return v;
		if (util::endsWith(v, "e")) return v + "d";
		return v + "ed";
	}
	static string describeList(const vector<string> &list)
	{
		string result;
		for (size_t i = 0; i < list.size(); i++)
		{
			result += list[i];
			if (i != list.size() - 1) result += ",";
		}
		return result;
	}

	static string getUnitTypePhrase(const string &s)
	{
		if (util::contains(s, ':'))
			return util::split(s, ':')[1];
		return s;
	}
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

	static bool partOfSpeechCheck(const string &query, const string &prefixList)
	{
		auto prefixes = util::split(prefixList, '|');
		for (const string &prefix : prefixes)
		{
			if (util::startsWith(query, prefix)) return true;
		}
		return false;
	}
};