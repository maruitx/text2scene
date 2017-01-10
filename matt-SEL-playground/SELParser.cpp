
#include "main.h"

void SELParser::parse(ParsedSentence &s)
{
	//
	// entities
	//
	extractEntities(s);
	assignDeterminers(s);
	assignCounts(s);
	assignEntityAttributes(s);
	assignEntityAttributeModifiers(s);
	assignRelationships(s);

	//
	// commands
	//
	extractCommands(s);
	assignCommandAttributes(s);
	assignCommandAttributeModifiers(s);
	assignTargets(s);

	applyCommands(s);
}

void SELParser::applyCommands(ParsedSentence &s)
{
	for (auto &c : s.commands)
	{
		if (appParams().isApplicableVerb(c.baseVerb))
		{
			applyCommand(s, c);
			c.applied = true;
		}
		if (c.baseVerb == "make")
		{
			for (auto &t : c.targets)
			{
				for (auto &a : c.attributes.list)
				{
					addEntityAttribute(s, t.referencedTokenIndex, a.name);
					for(auto &m : a.modifiers)
						addEntityAttributeModifier(s, t.referencedTokenIndex, a.name, m);
				}
			}
			c.applied = true;
		}
	}
}

void SELParser::applyCommand(ParsedSentence &s, const SceneCommand &c)
{
	for (auto &t : c.targets)
	{
		if (t.type == "dobj" || t.type == "nsubj")
		{
			const string appliedVerb = SELUtil::makeVerbApplied(c.baseVerb);
			addEntityAttribute(s, t.referencedTokenIndex, appliedVerb);
			for (auto &a : c.attributes.list)
			{
				addEntityAttributeModifier(s, t.referencedTokenIndex, appliedVerb, a.name);
			}
			for (auto &tOther : c.targets)
			{
				if (tOther.type != "dobj" && tOther.type != "nsubj")
				{
					addRelationship(s, t.referencedTokenIndex, tOther.referencedTokenIndex, tOther.type);
				}
			}
		}
	}
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

void SELParser::assignCommandAttributes(ParsedSentence &s)
{
	// Adjectives from advmod. ex. "Move the chairs together."
	for (auto &u : s.findUnits("advmod", "VB", "RB|JJ"))
	{
		addCommandAttribute(s, u.pAIndex, u.pB);
	}

	// Adjectives from xcomp. ex. "Make the kitchen table and the desk more messy."
	//xcomp(Make-1, messy-9~JJ)
	for (auto &u : s.findUnits("xcomp", "VB", "JJ"))
	{
		addCommandAttribute(s, u.pAIndex, u.pB);
	}
}

void SELParser::assignCommandAttributeModifiers(ParsedSentence &s)
{
	// Make the kitchen table and the desk more messy.
	//advmod(messy-9, more-8~RBR)
	//xcomp(Make-1, messy-9~JJ)
	//advmod(0-JJ, 1-RB)
	//xcomp(2-VB, 0-JJ)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("advmod(0-JJ, 1-RB)", "xcomp(2-VB, 0-JJ)")))
	{
		addCommandAttributeModifier(s, r.tokens[2], s.tokens[r.tokens[0]].text, s.tokens[r.tokens[1]].text);
	}

	// Move the chairs closer together.
	//advmod(together-5, closer-4~RBR)
	//advmod(Move-1, together-5~RB)
	//advmod(0-RB, 1-RB)
	//advmod(2-VB, 0-RB)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("advmod(0-RB, 1-RB)", "advmod(2-VB, 0-RB)")))
	{
		addCommandAttributeModifier(s, r.tokens[2], s.tokens[r.tokens[0]].text, s.tokens[r.tokens[1]].text);
	}
}

void SELParser::assignTargets(ParsedSentence &s)
{
	for (auto &u : s.findUnits("dobj", "VB", "NN"))
	{
		addTarget(s, u.pAIndex, u.pBIndex, u.type);
	}

	for (auto &u : s.findUnits("nsubj", "NN", "VB"))
	{
		addTarget(s, u.pBIndex, u.pAIndex, u.type);
	}

	//transfer some of the books on the desk to the table.
	//nmod:to(transfer-10, table-20~NN)
	for (auto &u : s.findUnits("nmod:", "VB", "NN"))
	{
		addTarget(s, u.pAIndex, u.pBIndex, u.getTypeSuffix());
	}

	// Make the kitchen table and the desk more messy.
	//nsubj(messy-9, table-4~NN)
	//xcomp(Make-1, messy-9~JJ)
	//nsubj(0-JJ, 1-NN)
	//xcomp(2-VB, 0-JJ)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("nsubj(0-JJ, 1-NN)", "xcomp(2-VB, 0-JJ)")))
	{
		//addAdverb(s, r.tokens[2], s.tokens[r.tokens[1]].text);
		addTarget(s, r.tokens[2], r.tokens[1], "nsubj");
	}

	// Put a laptop and a lamp on the right of the desk.
	//dobj(put-1, laptop-3~NN)
	//conj:and(laptop-3, lamp-6~NN)
	//dobj(0-VB, 1-NN)
	//conj:and(1-NN, 2-NN)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("dobj(0-VB, 1-NN)", "conj:and(1-NN, 2-NN)")))
	{
		addTarget(s, r.tokens[0], r.tokens[2], "dobj");
	}

	// Put a laptop and a lamp on the right of the desk.
	//nmod:on(put-1, right-9~NN)
	//nmod:of(right-9, desk-12~NN)
	//nmod:(0-VB, 1-NN)
	//nmod:(1-NN, 2-NN)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("nmod:(0-VB, 1-NN)", "nmod:(1-NN, 2-NN)")))
	{
		addTarget(s, r.tokens[0], r.tokens[2], s.units[r.units[0]].getTypeSuffix() + " " + s.tokens[r.tokens[1]].text + " " + s.units[r.units[1]].getTypeSuffix());
	}
}

void SELParser::extractEntities(ParsedSentence &s)
{
	set<int> rejectList;

	// reject nouns that are compounds (ex. "dining" in "dining table").
	for (auto &u : s.findUnits("compound", "NN", "NN"))
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
			!appParams().isSpecialNoun(t.text) &&
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
			addEntityAttribute(s, u.pAIndex, u.pB);
		else
		{
			auto e = s.getEntity(u.pAIndex);
			if (e != nullptr)
			{
				e->determiners.push_back(u.pB);
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

void SELParser::addCommandAttribute(ParsedSentence &s, int commandToken, const string &attribute)
{
	auto c = s.getCommand(commandToken);
	if (c != nullptr)
	{
		c->attributes.addAttribute(attribute);
	}
}

void SELParser::addCommandAttributeModifier(ParsedSentence &s, int commandToken, const string &attribute, const string &modifier)
{
	auto c = s.getCommand(commandToken);
	if (c != nullptr)
	{
		c->attributes.addAttributeModifier(attribute, modifier);
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

void SELParser::addEntityAttribute(ParsedSentence &s, int tokenIndex, const string &attribute)
{
	if (appParams().isSpecialNoun(s.tokens[tokenIndex].text))
		return;

	if (appParams().isStopVerb(attribute))
		return;

	auto e = s.getEntity(tokenIndex);
	if (e != nullptr)
	{
		if (appParams().isCountingAdjective(attribute))
		{
			if (e->count.count != 1 || e->count.descriptor != EntityCount::defaultDescriptor())
			{
				cout << "Unexpected count double-assignment" << endl;
			}
			e->count.count = -1;
			e->count.descriptor = attribute;
		}
		else
		{
			e->attributes.addAttribute(attribute);
		}
	}
}

void SELParser::addEntityAttributeModifier(ParsedSentence &s, int entityToken, const string &attribute, const string &modifier)
{
	auto e = s.getEntity(entityToken);
	if (e != nullptr)
	{
		e->attributes.addAttributeModifier(attribute, modifier);
	}
}

void SELParser::addRelationship(ParsedSentence &s, int tokenIndexA, int tokenIndexB, string relationshipType)
{
	if (util::contains(relationshipType, '_'))
		relationshipType = util::replace(relationshipType, '_', ' ');

	// ignore recording spatial noun relationships.
	if (appParams().isSpecialNoun(s.tokens[tokenIndexA].text) ||
		appParams().isSpecialNoun(s.tokens[tokenIndexB].text))
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

void SELParser::assignEntityAttributes(ParsedSentence &s)
{
	// Adjectives from nsubj. Here the connecting verb is assumed to be "to be".
	// This may not always be the case; investigate further.
	// ex. "The chairs are wooden."
	for (auto &u : s.findUnits("nsubj", "JJ|RB|VB", "NN"))
	{
		addEntityAttribute(s, u.pBIndex, u.pA);
	}

	// Adjectives from compounds. ex. "There is a dining table."
	for (auto &u : s.findUnits("compound", "NN", ""))
	{
		addEntityAttribute(s, u.pAIndex, u.pB);
	}

	// Adjectives from amod. ex. "a sleek and white laptop"
	for (auto &u : s.findUnits("amod"))
	{
		addEntityAttribute(s, u.pAIndex, u.pB);
	}

	// The chairs are all aligned.
	//nsubjpass(aligned-5, chairs-2~NNS)
	//advmod(aligned-5, all-4~DT)
	//nsubj(0-VB, 1-NN)
	//advmod(0-VB, 2-DT)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("nsubj(0-VB, 1-NN)", "advmod(0-VB, 2-DT)")))
	{
		addEntityAttribute(s, r.tokens[1], s.tokens[r.tokens[2]].text);
	}

	// Distribute utensils formally on the dining table.
	// advmod(utensils-2, formally-3~RB)
	for (auto &u : s.findUnits("advmod", "NN", "RB"))
	{
		addEntityAttribute(s, u.pAIndex, u.pB);
	}

	// Parser failures should be taken as attributes.
	// Clean the desk.
	//dep(clean-1, desk-3~NN)
	for (auto &u : s.findUnits("dep", "JJ", "NN"))
	{
		addEntityAttribute(s, u.pBIndex, u.pA);
	}

	// A plate, a knife and a fork is placed formally in front of each chair.
	//nsubjpass(placed-10, plate-2~NN)
	//conj:and(plate-2, knife-5~NN)
	//nsubjpass(0-VB, 1-NN)
	//conj:and(1-NN, 2-NN)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("nsubjpass(0-VB, 1-NN)", "conj:and(1-NN, 2-NN)")))
	{
		addEntityAttribute(s, r.tokens[2], s.tokens[r.tokens[0]].text);
	}

	//a pile of stacked books
	//nmod:of(pile-14, books-17~NNS)
	//nmod:(pile-14, books-17~NNS)
	for (auto &u : s.findUnits("nmod:", "NN", "NN"))
	{
		if (appParams().isGroupNoun(u.pA))
		{
			addEntityAttribute(s, u.pBIndex, SELUtil::makeVerbApplied(u.pA));
		}
	}
}

void SELParser::assignEntityAttributeModifiers(ParsedSentence &s)
{
	// Make the kitchen table more messy.
	//nsubj(messy-6, table-4~NN)
	//advmod(messy-6, more-5~RBR)
	//nsubj(0-JJ, 1-NN)
	//advmod(0-JJ, 2-RB)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("nsubj(0-JJ, 1-NN)", "advmod(0-JJ, 2-RB)")))
	{
		addEntityAttributeModifier(s, r.tokens[1], s.tokens[r.tokens[0]].text, s.tokens[r.tokens[2]].text);
	}

	// The art is casually distributed on the wall.
	// nsubjpass(distributed-5, art-2~NN)
	// advmod(distributed-5, casually-4~RB)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("nsubj(0-VB, 1-NN)", "advmod(0-VB, 2-RB)")))
	{
		addEntityAttributeModifier(s, r.tokens[1], s.tokens[r.tokens[0]].text, s.tokens[r.tokens[2]].text);
	}

	// A plate, a knife and a fork is placed formally in front of each chair.
	//nsubjpass(placed-10, plate-2~NN)
	//conj:and(plate-2, knife-5~NN)
	//advmod(placed-10, formally-11~RB)
	//nsubjpass(0-VB, 1-NN)
	//conj:and(1-NN, 2-NN)
	//advmod(0-VB, 3-RB)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("nsubjpass(0-VB, 1-NN)", "conj:and(1-NN, 2-NN)", "advmod(0-VB, 3-RB)")))
	{
		addEntityAttributeModifier(s, r.tokens[2], s.tokens[r.tokens[0]].text, s.tokens[r.tokens[3]].text);
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

	//There is a desk with two monitors, a keyboard, and a mouse.
	//nmod:with(desk-4, monitors-7~NNS)
	//conj:and(monitors-7, keyboard-10~NN)
	//nmod:(0-NN, 1-NN)
	//conj:and(1-NN, 2-NN)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("nmod:(0-NN, 1-NN)", "conj:and(1-NN, 2-NN)")))
	{
		addRelationship(s, r.tokens[0], r.tokens[2], s.units[r.units[0]].getTypeSuffix());
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
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("nmod:(0-NN, 1-NN)", "nmod:(1-NN, 2-NN)")))
	{
		addRelationship(s, r.tokens[0], r.tokens[2], s.units[r.units[0]].getTypeSuffix() + " " + s.tokens[r.tokens[1]].text + " " + s.units[r.units[1]].getTypeSuffix());
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
		string determiners;
		//det(side-7, each-6~DT)
		for (auto &u : s.findUnits("det", r.tokens[1], -1))
		{
			determiners += u.pB + " ";
		}
		addRelationship(s, r.tokens[3], r.tokens[2], s.units[r.units[0]].getTypeSuffix() + " " + determiners + s.tokens[r.tokens[1]].text + " " + s.units[r.units[1]].getTypeSuffix());
	}

	//In the center of the table, there is a vase and some sauce bottles.
	//nmod:in(is-9, center-3~NN)
	//nmod:of(center-3, table-6~NN)
	//nsubj(is-9, vase-11~NN)
	//conj:and(vase-11, bottles-15~NNS)
	//nmod:(0-VB, 1-NN)
	//nmod:(1-NN, 2-NN)
	//nsubj(0-VB, 3-NN)
	//conj:and(3-NN, 4-NN)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("nmod:(0-VB, 1-NN)", "nmod:(1-NN, 2-NN)", "nsubj(0-VB, 3-NN)", "conj:and(3-NN, 4-NN)")))
	{
		addRelationship(s, r.tokens[4], r.tokens[2], s.units[r.units[0]].getTypeSuffix() + " " + s.tokens[r.tokens[1]].text + " " + s.units[r.units[1]].getTypeSuffix());
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

	//There is a keyboard and a mouse on the desk.
	//conj:and(keyboard-4, mouse-7~NN)
	//nmod:on(mouse-7, desk-10~NN)
	//conj:and(0-NN, 1-NN)
	//nmod:(1-NN, 2-NN)
	for (auto &r : PatternMatcher::match(s, PatternMatchQuery("conj:and(0-NN, 1-NN)", "nmod:(1-NN, 2-NN)")))
	{
		addRelationship(s, r.tokens[0], r.tokens[2], s.units[r.units[1]].getTypeSuffix());
	}
}
