
#include "main.h"

void App::runTests()
{
	const string debugText = "Today I am walking to the bookstore. There are several red books on the top shelf.";
	const string parsedText = SELUtil::processText(debugText);
	cout << "input: " << debugText << endl;
	cout << "output: " << parsedText << endl;
}

void App::go()
{
	runTests();
}
