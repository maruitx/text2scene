
#include "main.h"

void SELParser::parse(ParsedSentence &s)
{
	//
	// entities
	//
	extractEntities(s);
	assignDeterminers(s);
	assignCounts(s);
	assignAdjectives(s);
	assignRelationships(s);

	//
	// commands
	//
	extractCommands(s);
	assignAdverbs(s);
	assignTargets(s);
}

void SELParser::extractCommands(ParsedSentence &s)
{
	set<int> rejectList;

	for (auto &t : s.tokens)
	{
		if (//util::startsWith(t.posTag, "VB") &&
			t.posTag == "VB" &&
			rejectList.count(t.index) == 0)
		{
			SceneCommand command;
			command.baseVerb = t.text;
			command.tokenIndex = t.index;
			s.commands.push_back(command);
		}
	}
}

void SELParser::assignAdverbs(ParsedSentence &s)
{
	// Adjectives from advmod. ex. "Move the chairs together."
	for (auto &u : s.findUnits("advmod", "VB", "RB|JJ"))
	{
		addAdverb(s, u.pAIndex, u.pB);
	}

	// Move the chairs closer together.
	//advmod(together-5, closer-4~RBR)
	//advmod(Move-1, together-5~RB)
	//advmod(0-RB, 1-RB)
	//advmod(2-VB, 0-RB)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("advmod(0-RB, 1-RB)", "advmod(2-VB, 0-RB)")))
	{
		addAdverb(s, r.tokens[2], s.tokens[r.tokens[1]].text);
	}
}

void SELParser::assignTargets(ParsedSentence &s)
{
	for (auto &u : s.findUnits("dobj", "VB", "NN"))
	{
		addTarget(s, u.pAIndex, u.pBIndex, u.type);
	}
}

void SELParser::extractEntities(ParsedSentence &s)
{
	set<int> rejectList;

	// reject nouns that are compounds (ex. "dining" in "dining table").
	for (auto &u : s.findUnits("compound"))
	{
		rejectList.insert(u.pBIndex);
	}

	for (auto &u : s.findUnits("amod"))
	{
		rejectList.insert(u.pBIndex);
	}

	auto &params = appParams();
	// accept all nouns not on the reject list
	for (auto &t : s.tokens)
	{
		if (util::startsWith(t.posTag, "NN") &&
			!appParams().isAbstractOrSpatialNoun(t.text) &&
			rejectList.count(t.index) == 0)
		{
			SceneEntity entity;
			entity.baseNoun = t.text;
			entity.tokenIndex = t.index;
			entity.plural = (t.posTag == "NNS");
			s.entities.push_back(entity);
		}
	}
}

void SELParser::assignDeterminers(ParsedSentence &s)
{
	// determiners from det
	for (auto &u : s.findUnits("det"))
	{
		// don't worry about determiners for spatial nouns.
		if (appParams().spatialNouns.count(u.pA) != 0)
			continue;

		if (appParams().isCountingAdjective(u.pB))
			addAdjective(s, u.pAIndex, util::toLower(u.pB));
		else
		{
			auto e = s.getEntity(u.pAIndex);
			if (e != nullptr)
			{
				e->determiners.push_back(util::toLower(u.pB));
			}
		}
	}
}

void SELParser::assignCounts(ParsedSentence &s)
{
	// counts from nummod
	for (auto &u : s.findUnits("nummod"))
	{
		auto e = s.getEntity(u.pAIndex);
		if (e != nullptr)
		{
			e->count = EntityCount(u.pB);
		}
	}
}

void SELParser::addAdverb(ParsedSentence &s, int commandToken, const string &adverb)
{
	auto c = s.getCommand(commandToken);
	if (c != nullptr)
	{
		c->adverbs.push_back(adverb);
	}
}

void SELParser::addTarget(ParsedSentence &s, int commandToken, int entityToken, string type)
{
	if (s.tokens[commandToken].posTag != "VB")
		return;

	auto c = s.getCommand(commandToken);
	auto e = s.getEntity(entityToken);
	if (c != nullptr && e != nullptr)
	{
		CommandTarget target;
		target.type = type;
		target.referencedNoun = e->baseNoun;
		target.referencedTokenIndex = e->tokenIndex;
		c->targets.push_back(target);
	}
}

void SELParser::addAdjective(ParsedSentence &s, int tokenIndex, const string &adjective)
{
	if (appParams().isAbstractOrSpatialNoun(s.tokens[tokenIndex].text))
		return;

	auto e = s.getEntity(tokenIndex);
	if (e != nullptr)
	{
		if (appParams().isCountingAdjective(adjective))
		{
			if (e->count.count != 1 || e->count.descriptor != EntityCount::defaultDescriptor())
			{
				cout << "Unexpected count double-assignment" << endl;
			}
			e->count.count = -1;
			e->count.descriptor = adjective;
		}
		else
		{
			e->adjectives.push_back(adjective);
		}
	}
}

void SELParser::addRelationship(ParsedSentence &s, int tokenIndexA, int tokenIndexB, string relationshipType)
{
	if (util::contains(relationshipType, '_'))
		relationshipType = util::replace(relationshipType, '_', ' ');

	// ignore recording spatial noun relationships.
	if (appParams().isAbstractOrSpatialNoun(s.tokens[tokenIndexA].text) ||
		appParams().isAbstractOrSpatialNoun(s.tokens[tokenIndexB].text))
		return;

	auto eL = s.getEntity(tokenIndexA);
	auto eR = s.getEntity(tokenIndexB);
	if (eL != nullptr && eR != nullptr)
	{
		EntityRelationship rel;
		rel.type = relationshipType;
		rel.referencedNoun = eR->baseNoun;
		rel.referencedTokenIndex = eR->tokenIndex;
		eL->relationships.push_back(rel);
	}
}

void SELParser::assignAdjectives(ParsedSentence &s)
{
	// Adjectives from nsubj. Here the connecting verb is assumed to be "to be".
	// This may not always be the case; investigate further.
	// ex. "The chairs are wooden."
	for (auto &u : s.findUnits("nsubj", "JJ|RB", "NN"))
	{
		addAdjective(s, u.pBIndex, u.pA);
	}

	// Adjectives from compounds. ex. "There is a dining table."
	for (auto &u : s.findUnits("compound", "NN", ""))
	{
		addAdjective(s, u.pAIndex, u.pB);
	}

	// Adjectives from amod. ex. "a sleek and white laptop"
	for (auto &u : s.findUnits("amod"))
	{
		addAdjective(s, u.pAIndex, u.pB);
	}
}

void SELParser::assignRelationships(ParsedSentence &s)
{
	for (auto &u : s.findUnits("nmod:", "NN", "NN"))
	{
		addRelationship(s, u.pAIndex, u.pBIndex, u.getTypeSuffix());
	}

	//On the desk there is a monitor, a keyboard, and a sleek and white laptop.
	//nmod:on(is-5, desk-3~NN)
	//nsubj(is-5, monitor-7~NN)
	//conj:and(monitor-7, keyboard-10~NN)
	//conj:and(monitor-7, laptop-17~NN)
	//nmod:on(0-VBZ, 1-NN)
	//nsubj(0-VBZ, 2-NN)
	//conj:and(2-NN, 3-NN)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("nmod:(0-VB, 1-NN)", "nsubj(0-VB, 2-NN)")))
	{
		string prefix = s.tokens[r.tokens[0]].text + " ";
		if (prefix == "is ") prefix = "";
		addRelationship(s, r.tokens[2], r.tokens[1], prefix + s.units[r.units[0]].getTypeSuffix());
	}
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("nmod:(0-VB, 1-NN)", "nsubj(0-VB, 2-NN)", "conj:and(2-NN, 3-NN)")))
	{
		string prefix = s.tokens[r.tokens[0]].text + " ";
		if (prefix == "is ") prefix = "";
		addRelationship(s, r.tokens[3], r.tokens[1], prefix + s.units[r.units[0]].getTypeSuffix());
	}

	// the soda is to the right of the laptop
	//nsubj(right-6, soda-2~NN)
	//nmod:of(right-6, laptop-9~NN)
	//nsubj(0-NN, 1-NN)
	//nmod:(0-NN, 2-NN)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("nsubj(0-NN, 1-NN)", "nmod:(0-NN, 2-NN)")))
	{
		addRelationship(s, r.tokens[1], r.tokens[2], s.tokens[r.tokens[0]].text + " " + s.units[r.units[1]].getTypeSuffix());
	}

	//The power adapter is below the desk.
	//nsubj(desk-7, adapter-3~NN)
	//case(desk-7, below-5~IN)
	//nsubj(0-NN, 1-NN)
	//case(0-NN, 2-IN)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("nsubj(0-NN, 1-NN)", "case(0-NN, 2-IN)")))
	{
		addRelationship(s, r.tokens[1], r.tokens[0], s.tokens[r.tokens[2]].text);
	}

	//The power adapter is below the desk and near the chair.
	//nsubj(desk-7, adapter-3~NN)
	//conj:and(desk-7, chair-11~NN)
	//case(chair-11, near-9~IN)
	//nsubj(0-NN, 1-NN)
	//conj:and(0-NN, 2-NN)
	//case(2-NN, 3-IN)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("nsubj(0-NN, 1-NN)", "conj:and(0-NN, 2-NN)", "case(2-NN, 3-IN)")))
	{
		addRelationship(s, r.tokens[1], r.tokens[2], s.tokens[r.tokens[3]].text);
	}

	//There are cream-colored built-in shelves displaying many knick-knacks
	//acl(shelves-5, displaying-6~VBG)
	//dobj(displaying-6, knick-knacks-8~NNS)
	//acl(0-NN, 1-VB)
	//dobj(1-VB, 2-NN)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("acl(0-NN, 1-VB)", "dobj(1-VB, 2-NN)")))
	{
		addRelationship(s, r.tokens[0], r.tokens[2], s.tokens[r.tokens[1]].text);
	}

	//There’s a red patterned rug in the middle of the room surrounded by a grey sofa and two sofa chairs with throw pillows.
	//nmod:in(rug-6, middle-9~NN)
	//nmod:of(middle-9, room-12~NN)
	//nmod:in(0-NN, 1-NN)
	//nmod:of(1-NN, 2-NN)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("nmod:in(0-NN, 1-NN)", "nmod:of(1-NN, 2-NN)")))
	{
		addRelationship(s, r.tokens[0], r.tokens[2], "in " + s.tokens[r.tokens[1]].text + " of");
	}

	//There’s a red patterned rug in the middle of the room surrounded by a grey sofa and two sofa chairs with throw pillows.
	//acl(rug-6, surrounded-7~VBN)
	//nmod:(surrounded-7, sofa-11~NN)
	//acl(0-NN, 1-VB)
	//nmod:(1-VB, 2-NN)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("acl(0-NN, 1-VB)", "nmod:(1-VB, 2-NN)")))
	{
		addRelationship(s, r.tokens[0], r.tokens[2], s.tokens[r.tokens[1]].text + " " + s.units[r.units[1]].getTypeSuffix());
	}

	//The coffee table has two levels
	//nsubj(has-4, table-3~NN)
	//dobj(has-4, levels-6~NNS)
	//nsubj(0-VB, 1-NN)
	//dobj(0-VB, 2-NN)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("nsubj(0-VB, 1-NN)", "dobj(0-VB, 2-NN)")))
	{
		addRelationship(s, r.tokens[1], r.tokens[2], s.tokens[r.tokens[0]].text);
	}

	//In the center of the room is a large rectangular area rug.
	//nmod:in(is-7, center-3~NN)
	//nmod:of(center-3, room-6~NN)
	//nsubj(is-7, rug-12~NN)
	//nmod:(0-VB, 1-NN)
	//nmod:(1-NN, 2-NN)
	//nsubj(0-VB, 3-NN)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("nmod:(0-VB, 1-NN)", "nmod:(1-NN, 2-NN)", "nsubj(0-VB, 3-NN)")))
	{
		addRelationship(s, r.tokens[3], r.tokens[2], s.units[r.units[0]].getTypeSuffix() + " " + s.tokens[r.tokens[1]].text + " " + s.units[r.units[1]].getTypeSuffix());
	}

	//Remove the speakers from the desk.
	//dobj(Remove-1, speakers-3~NNS)
	//nmod:from(Remove-1, desk-6~NN)
	//dobj(0-VB, 1-NN)
	//nmod:(0-VB, 2-NN)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("dobj(0-VB, 1-NN)", "nmod:(0-VB, 2-NN)")))
	{
		addRelationship(s, r.tokens[1], r.tokens[2], s.units[r.units[1]].getTypeSuffix());
	}
}
