
#include "main.h"

void SELParser::parse(ParsedSentence &s)
{
	extractEntities(s);
	assignDeterminers(s);
	assignCounts(s);
	assignAdjectives(s);
	assignRelationships(s);
}

void SELParser::extractEntities(ParsedSentence &s)
{
	set<int> rejectList;

	// reject nouns that are compounds (ex. "dining" in "dining table").
	for (auto &u : s.findUnits("compound"))
	{
		rejectList.insert(u.pBIndex);
	}

	
	//auto &params = appParams();
	// accept all nouns not on the reject list
	for (auto &t : s.tokens)
	{
		if (util::startsWith(t.posTag, "NN") &&
			appParams().spatialNouns.count(t.text) == 0 &&
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
		auto e = s.getEntity(u.pAIndex);
		if (e != nullptr)
		{
			e->determiner = util::toLower(u.pB);
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

void SELParser::addAdjective(ParsedSentence &s, int tokenIndex, const string &adjective)
{
	auto e = s.getEntity(tokenIndex);
	if (e != nullptr)
	{
		e->adjectives.push_back(adjective);
	}
}

void SELParser::addRelationship(ParsedSentence &s, int tokenIndexA, int tokenIndexB, string relationshipType)
{
	if (util::contains(relationshipType, ':'))
		relationshipType = util::split(relationshipType, ':')[1];
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
	for (auto &u : s.findUnits("nsubj", "JJ", "NN"))
	{
		addAdjective(s, u.pBIndex, u.pA);
	}

	// Adjectives from compounds. ex. "There is a dining table.".
	for (auto &u : s.findUnits("compound"))
	{
		addAdjective(s, u.pAIndex, u.pB);
	}

	// Adjectives from amod. ex. "a sleek and white laptop".
	for (auto &u : s.findUnits("amod"))
	{
		addAdjective(s, u.pAIndex, u.pB);
	}
}

void SELParser::assignRelationships(ParsedSentence &s)
{
	for (auto &u : s.findUnits("nmod:", "NN", "NN"))
	{
		addRelationship(s, u.pAIndex, u.pBIndex, u.type);
	}

	//On the desk there is a monitor, a keyboard, and a sleek and white laptop.
	//nmod:on(is-5, desk-3~NN)
	//nsubj(is-5, monitor-7~NN)
	//conj:and(monitor-7, keyboard-10~NN)
	//conj:and(monitor-7, laptop-17~NN)
	for (auto &vbUnit : s.findUnits("nmod:", "VB", "NN"))
	{
		const int verbTokenIndex = vbUnit.pAIndex;
		for (auto &baseNounUnit : s.findUnits("nsubj", verbTokenIndex, -1))
		{
			vector<int> nounTokens;
			const int baseNounTokenIndex = baseNounUnit.pBIndex;
			nounTokens.push_back(baseNounTokenIndex);
			for (auto &conjNounUnit : s.findUnits("conj:and", baseNounTokenIndex, -1))
			{
				nounTokens.push_back(conjNounUnit.pBIndex);
			}

			for (int nounToken : nounTokens)
			{
				addRelationship(s, nounToken, vbUnit.pBIndex, vbUnit.type);
			}
		}
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
}
