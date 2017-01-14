
#include "main.h"

void App::runTests()
{
	string debugText = "";

	for (auto &s : util::getFileLines(appParams().inputFile))
		debugText += s + " ";

	//debugText += "Today I am walking to the bookstore. There are several red books on the top shelf.";
	//debugText += "There are four chairs around a dining table. The chairs are wooden.";
	//debugText += "On the desk there is a monitor, a keyboard, and a sleek and white laptop.";
	//debugText += "The soda is to the right of the laptop.";
	//debugText += "The power adapter is below the desk.";
	//debugText += "The power adapter is below the desk and near the chair.";
	//debugText += "The room also has large windows and wooden floors.";
	//debugText += "There are cream-colored built-in shelves displaying many knick-knacks along with a fireplace at the front of the room.";
	//debugText += "There’s a red patterned rug in the middle of the room surrounded by a grey sofa and two sofa chairs with throw pillows.";
	//debugText += "There’s a red patterned rug surrounded by a grey sofa and two sofa chairs with throw pillows.";
	//debugText += "In addition, there’s a wooden coffee table on top of the rug in the middle of the seating arrangement, as well as a side table between the two chairs.";
	//debugText += "The room also has several flower-related items -- bouquets on the tables and an abstract flowery painting above the fireplace.";
	//debugText += "It’s a fairly busy but warm-looking room.";
	//debugText += "A dark wood, rectangular coffee table is on top of the rug.";
	//debugText += "The coffee table has two levels; there are books on the lower level and some decorative plates on the top level.";
	//debugText += "This entertainment unit is long; it runs the entire length of the wall.";
	//debugText += "There are tall, thin, modern speakers flush up against the sides of the center part.";
	//debugText += "Above the entertainment unit, a flat screen TV is mounted on the wall.";
	//debugText += "A typical living room with a table in the middle of two sofas (one is a large sofa and the other is a single seat sofa) with a flower vase on its top.";
	//debugText += "The shelves on either side of the wall have some showpiece items for aesthetic appeal.";
	//debugText += "The living room has a fireplace in the middle of two built-in shelves and cabinets.";
	//debugText += "In the center of the room is a large rectangular area rug.";
	//debugText += "There is a PC on the desk. To the right of the desk, there is a bookshelf with books.";
	//debugText += "There are two sleek, white laptops on the desk. To the right of the wooden desk, there is a big bookshelf with many books.";
	//debugText += "Books are sparsely distributed on the bookshelf.";
	//debugText += "In the center of the table is a flower vase.";
	//debugText += "There is a group of books on the desk.";
	//debugText += "Put a stack of books on the desk.";
	//debugText += "Stack books on the desk.";
	//debugText += "There are books stacked on the desk.";
	//debugText += "";

	//
	// commands
	//
	//debugText += "Move the chairs around the dining table further apart and transfer some of the books on the desk to the table.";
	//debugText += "Make the kitchen table more messy.";
	//debugText += "Make the kitchen table and the desk more messy.";
	//debugText += "Remove the speakers from the desk.";
	//debugText += "Rearrange the chairs.";
	//debugText += "Move the PC on the desk to the floor.";
	//debugText += "Move the PC to the floor.";
	//debugText += "The table is surrounded with chairs. Surround the table with chairs.";
	//debugText += "The chairs are all aligned. Align all the chairs.";
	//debugText += "Stack the books. Pile the books. The books are stacked.";
	//debugText += "Distribute chairs on the floor. The chairs are scattered on the floor.";
	//debugText += "Collect the items on the table. The items are grouped on the table.";
	//debugText += "Make the kitchen table messy. There is a messy kitchen table.";
	//debugText += "Clean the desk. The desk is clean.";
	//debugText += "Put utensils formally on the dining table. The formal utensils are on the dining table.";
	//debugText += "Put the art casually on the wall. The art is casually distributed on the wall.";
	//debugText += "Put books sparsely on the shelf. Books are sparsely distributed on the shelf.";
	//debugText += "The meeting room is crowded with chairs.";
	//debugText += "Put books on the desk and rearrange the chairs.";
	//debugText += "Move two chairs closer to the table.";
	//debugText += "There is a desk with two monitors, a keyboard, and a mouse.";
	//debugText += "A cellphone, a headphone and a lamp are on the desk.";
	//debugText += "Under the desk there is a PC, a speaker, and a power socket.";
	//debugText += "There is a desk with some books and papers.";
	//debugText += "Put a laptop and a lamp on the right of the desk.";
	//debugText += "To the left of the desk, there is a cabinet with a pile of stacked books and a clock.";
	//debugText += "There is an organized computer desk. Next to the desk, there is a file cabinet with a printer on top. A bookshelf with books is to the right of the desk.";
	//debugText += "There are two desks aligned along the wall. On the desks, there are monitors and keyboards. In front of the desks, there is a sofa with three pillows on it.";
	//debugText += "There is a bed in the left of the room. Next to the bed, there is a messy office desk. There are books scattered on the bed.";
	//debugText += "There is a queen size bed with two nightstands on each side. On each nightstand, there is a lamp. There is one dresser to the left of the bed, and a sofa chair is near the dresser.";
	//debugText += "On the table, there is a vase, a teapot, and dinner plates. There is plenty of food on the plates.";
	//debugText += "Two chairs are aligned on each side of a rectangular dining table. A plate, a knife and a fork is placed formally in front of each chair. In the center of the table, there is a vase and some sauce bottles.";
	//debugText += "In the center of the table, there is a vase and some sauce bottles.";
	//debugText += "There is a tv in front of the sofa. To the right of the sofa, there is a shelf with books and photo frames. Some pictures are casually placed on the wall.";
	//debugText += "There is a couch and two sofa chairs in the room. In front of the couch is a messy coffee table. On the other side of the room, there is a tv with two speakers on each side.";
	//debugText += "On the other side of the room, there is a tv with two speakers on each side.";
	//debugText += "To the left of the desk, there is a cabinet with a pile of stacked books and a clock.";
	//debugText += "A messy coffee table is in front of the couch.";
	//debugText += "Two chairs are aligned on each side of a rectangular dining table.";
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
	ParsedSentence::serializeSentences(sentences, appParams().outputFile);
}

void App::go()
{
	runTests();
}
