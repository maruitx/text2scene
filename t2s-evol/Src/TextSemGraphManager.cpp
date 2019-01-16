#include "TextSemGraphManager.h"
#include "Utility.h"
#include "TextSemGraph.h"
#include "RelationModelManager.h"

TextSemGraphManager::TextSemGraphManager()
{
	m_activeGraphId = 0;
	m_textGraphNum = 0;
}

TextSemGraphManager::~TextSemGraphManager()
{

}

void TextSemGraphManager::loadSELFromOutput(const QString &filename, RelationModelManager *relManager)
{
	clearPreviousLoadSEL();

	cout << "\nTextSemGraphManager:Loading SEL parsing result file ... ";
	QFile inFile(filename);
	QTextStream ifs(&inFile);

	if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		cout << "\nTextSemGraphManager: cannot open SEL output file\n";
		return;
	}

	bool isCommand = false;

	m_SEL_version = ifs.readLine();

	QString currLine = ifs.readLine();
	if (currLine.contains("sentence-count"))
	{
		m_sentence_count = StringToIntegerList(currLine.toStdString(), "sentence-count:")[0];
		currLine = ifs.readLine();
	}

	for (int si = 0; si < m_sentence_count; si++)
	{
		SelSentence newSentence;

		// read sentence id and string		
		if (currLine.contains("sentence-"))
		{
			newSentence.id = StringToIntegerList(currLine.toStdString(), "sentence-")[0];
			newSentence.textString = ifs.readLine();
		}

		// read entity count
		currLine = ifs.readLine();
		if (currLine.contains("entity-count"))
		{
			newSentence.entityCount = StringToIntegerList(currLine.toStdString(), "entity-count:")[0];
			currLine = ifs.readLine();
		}

		for (int ei = 0; ei < newSentence.entityCount; ei++)
		{
			if (currLine.contains("entity-"))
			{
				SelEntity newEntity;
				newEntity.id = StringToIntegerList(currLine.toStdString(), "entity-")[0];

				currLine = ifs.readLine();

				while (!currLine.contains("sentence-") &&
					!currLine.contains("entity-") &&
					!currLine.contains("***") &&
					!currLine.contains("command"))
				{
					if (currLine.contains("id:"))
					{
						std::vector<std::string> parts = PartitionString(currLine.toStdString(), ":");
						newEntity.nameString = QString(parts[1].c_str());
					}

					if (currLine.contains("plural"))
					{
						newEntity.isPlural = StringToIntegerList(currLine.toStdString(), "plural:")[0];
					}

					if (currLine.left(5) == "count")
					{
						std::vector<std::string> parts = PartitionString(currLine.toStdString(), ":");
						newEntity.instanceCountString =  QString(parts[1].c_str());
					}

					if (currLine.contains("determiners"))
					{
						std::vector<std::string> parts = PartitionString(currLine.toStdString(), ":");
						if (parts.size()>  1)
						{
							newEntity.m_determiner = QString(parts[1].c_str());
							newEntity.m_determiner.remove(",");
						}
					}

					if (currLine.contains("attributes"))
					{
						newEntity.attributeCount = StringToIntegerList(currLine.toStdString(), "attributes:")[0];

						for (int ai = 0; ai < newEntity.attributeCount; ai++)
						{
							SelAttribute newAttribute;
							newAttribute.id = ai;

							currLine = ifs.readLine();
							newAttribute.nameString = currLine;
							newEntity.m_attributes.push_back(newAttribute);
						}
					}

					if (currLine.contains("relationship-count"))
					{
						newEntity.relationshipCount = StringToIntegerList(currLine.toStdString(), "relationship-count:")[0];

						for (int ri = 0; ri < newEntity.relationshipCount; ri++)
						{
							currLine = ifs.readLine();
							SelRelationship newRelationship;
							newRelationship.id = ri;
							std::vector<std::string> parts = PartitionString(currLine.toStdString(), ":");
							newRelationship.nameString = QString(parts[0].c_str());
							newRelationship.entityString = QString(parts[1].c_str());

							newEntity.m_relationships.push_back(newRelationship);
						}
					}

					currLine = ifs.readLine();
				}

				newSentence.m_entities.push_back(newEntity);
			}
		}   // end of loop for entity

		// parse for command
		//currLine = ifs.readLine();
		if (currLine.contains("command-count"))
		{
			int commandCount = StringToIntegerList(currLine.toStdString(), "command-count:")[0];
			newSentence.commandCount = commandCount;

			for (int ci = 0; ci < commandCount; ci++)
			{
				currLine = ifs.readLine();
				SelCommand newCommand;
				newCommand.id = StringToIntegerList(currLine.toStdString(), "command-")[0];

				currLine = ifs.readLine();
				std::vector<std::string> parts = PartitionString(currLine.toStdString(), ":");
				newCommand.verbString = QString(parts[1].c_str());

				currLine = ifs.readLine();
				newCommand.isApplied = StringToIntegerList(currLine.toStdString(), "applied:")[0];

				currLine = ifs.readLine();
				newCommand.attributeCount = StringToIntegerList(currLine.toStdString(), "attribute-count:")[0];

				for (int cai = 0; cai < newCommand.attributeCount; cai++)
				{
					currLine = ifs.readLine();
					newCommand.attributeStrings.push_back(currLine);
				}

				currLine = ifs.readLine();
				newCommand.targetCount = StringToIntegerList(currLine.toStdString(), "target-count:")[0];

				for (int ti = 0; ti < newCommand.targetCount; ti++)
				{
					currLine = ifs.readLine();
					newCommand.targetStrings.push_back(currLine);
				}

				// only consider the command that is not applied
				if (newCommand.isApplied == false)
				{
					newSentence.m_commands.push_back(newCommand);
					isCommand = true;
				}
			}
		}

		TextSemGraph* newTextSemGraph = new TextSemGraph(newSentence, relManager, isCommand);
		m_textSemGraphs.push_back(newTextSemGraph);
		m_textGraphNum++;

	} // end of loop for sentence

	inFile.close();
	cout << "done\n";
}

void TextSemGraphManager::clearPreviousLoadSEL()
{
	for (int i = 0; i < m_textSemGraphs.size(); i++)
	{
		delete m_textSemGraphs[i];
	}

	m_textSemGraphs.clear();
}

void TextSemGraphManager::updateActiveGraphId()
{
	m_activeGraphId++;
	
	if (m_activeGraphId == m_textSemGraphs.size())
	{
		m_activeGraphId = m_textSemGraphs.size() -1;
	}
}
