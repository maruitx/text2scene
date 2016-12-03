
#include "main.h"

void App::runTests()
{
	//const string debugText = "Today I am walking to the bookstore. There are several red books on the top shelf.";
	//const string debugText = "There are four chairs around a dining table. The chairs are wooden.";
	const string debugText = "On the desk there is a monitor, a keyboard, and a sleek and white laptop.";
	const string parsedText = SELUtil::processText(debugText);
	cout << "input: " << debugText << endl;
	cout << "output: " << parsedText << endl;

	auto sentences = ParsedSentence::makeSentences(parsedText);
	for (auto &s : sentences)
	{
		cout << "Sentence:" << endl;
		cout << s.toString() << endl;

		SELParser parser;
		parser.parse(s);

		cout << "Entity count: " << s.entities.size() << endl;
		for (auto &e : iterate(s.entities))
		{
			cout << e.value.toString() << endl;
		}
	}
}

void App::go()
{
	runTests();
}
