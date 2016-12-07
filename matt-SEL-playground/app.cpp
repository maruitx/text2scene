
#include "main.h"

void App::runTests()
{
	//const string debugText = "Today I am walking to the bookstore. There are several red books on the top shelf.";
	//const string debugText = "There are four chairs around a dining table. The chairs are wooden.";
	//const string debugText = "On the desk there is a monitor, a keyboard, and a sleek and white laptop.";
	//const string debugText = "The soda is to the right of the laptop.";
	//const string debugText = "The power adapter is below the desk.";
	//const string debugText = "The power adapter is below the desk and near the chair.";
	//const string debugText = "The room also has large windows and wooden floors.";
	//const string debugText = "There are cream-colored built-in shelves displaying many knick-knacks along with a fireplace at the front of the room.";
	//const string debugText = "There’s a red patterned rug in the middle of the room surrounded by a grey sofa and two sofa chairs with throw pillows.";
	//const string debugText = "There’s a red patterned rug surrounded by a grey sofa and two sofa chairs with throw pillows.";
	//const string debugText = "In addition, there’s a wooden coffee table on top of the rug in the middle of the seating arrangement, as well as a side table between the two chairs.";
	//const string debugText = "The room also has several flower-related items -- bouquets on the tables and an abstract flowery painting above the fireplace.";
	//const string debugText = "It’s a fairly busy but warm-looking room.";
	//const string debugText = "A dark wood, rectangular coffee table is on top of the rug.";
	//const string debugText = "The coffee table has two levels; there are books on the lower level and some decorative plates on the top level.";
	//const string debugText = "This entertainment unit is long; it runs the entire length of the wall.";
	//const string debugText = "There are tall, thin, modern speakers flush up against the sides of the center part.";
	//const string debugText = "Above the entertainment unit, a flat screen TV is mounted on the wall.";
	//const string debugText = "A typical living room with a table in the middle of two sofas (one is a large sofa and the other is a single seat sofa) with a flower vase on its top.";
	//const string debugText = "The shelves on either side of the wall have some showpiece items for aesthetic appeal.";
	//const string debugText = "The living room has a fireplace in the middle of two built-in shelves and cabinets.";
	//const string debugText = "In the center of the room is a large rectangular area rug.";
	const string debugText = "There is a PC on the desk. To the right of the desk, there is a bookshelf with books.";
	//const string debugText = "";
	//const string debugText = "";
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
