
#include "main.h"

void App::runTests()
{
	string debugText = "";
	//debugText += "Today I am walking to the bookstore. There are several red books on the top shelf.";
	//debugText += "There are four chairs around a dining table. The chairs are wooden.";
	//debugText += "On the desk there is a monitor, a keyboard, and a sleek and white laptop.";
	//debugText += "The soda is to the right of the laptop.";
	//debugText += "The power adapter is below the desk.";
	//debugText += "The power adapter is below the desk and near the chair.";
	//debugText += "The room also has large windows and wooden floors.";
	//debugText += "There are cream-colored built-in shelves displaying many knick-knacks along with a fireplace at the front of the room.";
	//debugText += "There�s a red patterned rug in the middle of the room surrounded by a grey sofa and two sofa chairs with throw pillows.";
	//debugText += "There�s a red patterned rug surrounded by a grey sofa and two sofa chairs with throw pillows.";
	//debugText += "In addition, there�s a wooden coffee table on top of the rug in the middle of the seating arrangement, as well as a side table between the two chairs.";
	//debugText += "The room also has several flower-related items -- bouquets on the tables and an abstract flowery painting above the fireplace.";
	//debugText += "It�s a fairly busy but warm-looking room.";
	//debugText += "A dark wood, rectangular coffee table is on top of the rug.";
	//debugText += "The coffee table has two levels; there are books on the lower level and some decorative plates on the top level.";
	//debugText += "This entertainment unit is long; it runs the entire length of the wall.";
	//debugText += "There are tall, thin, modern speakers flush up against the sides of the center part.";
	//debugText += "Above the entertainment unit, a flat screen TV is mounted on the wall.";
	//debugText += "A typical living room with a table in the middle of two sofas (one is a large sofa and the other is a single seat sofa) with a flower vase on its top.";
	debugText += "The shelves on either side of the wall have some showpiece items for aesthetic appeal.";
	//debugText += "The living room has a fireplace in the middle of two built-in shelves and cabinets.";
	//debugText += "In the center of the room is a large rectangular area rug.";
	//debugText += "There is a PC on the desk. To the right of the desk, there is a bookshelf with books.";
	//debugText += "";
	//debugText += "";
	//debugText += "";
	//debugText += "";

	//
	// commands
	//
	//debugText += "Move the chairs closer together.";
	//debugText += "Make the kitchen table messy.";
	//debugText += "Align all the chairs.";
	//debugText += "Remove the speakers from the desk.";
	//debugText += "Rearrange the chairs.";
	//debugText += "Move the PC on the desk to the floor.";
	debugText += "";
	debugText += "";
	debugText = util::replace(debugText, ".", ". ");
	const string parsedText = SELUtil::processText(debugText);
	cout << "input: " << debugText << endl;
	cout << "output: " << parsedText << endl;

	auto sentences = ParsedSentence::makeSentences(parsedText);
	for (auto &s : sentences)
	{
		cout << "Sentence:" << endl;
		cout << s.toString() << endl;
		//cout << s.text << endl;

		SELParser parser;
		parser.parse(s);

		cout << "Entity count: " << s.entities.size() << endl;
		for (auto &e : iterate(s.entities))
		{
			cout << e.value.toString() << endl;
		}

		cout << "Command count: " << s.commands.size() << endl;
		for (auto &c : iterate(s.commands))
		{
			cout << c.value.toString() << endl;
		}
	}
}

void App::go()
{
	runTests();
}
