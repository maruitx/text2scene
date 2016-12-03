
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

	// accept all nouns not on the reject list
	for (auto &t : s.tokens)
	{
		if (util::startsWith(t.tag, "NN") &&
			rejectList.count(t.index) == 0)
		{
			SceneEntity entity;
			entity.baseNoun = t.text;
			entity.tokenIndex = t.index;
			entity.plural = (t.tag == "NNS");
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
			e->determiner = u.pB;
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

void SELParser::assignAdjectives(ParsedSentence &s)
{
	// Adjectives from nsubj. Here the connecting verb is assumed to be "to be".
	// This may not always be the case; investigate further.
	// ex. "The chairs are wooden."
	for (auto &u : s.findUnits("nsubj", "JJ", "NN"))
	{
		auto e = s.getEntity(u.pBIndex);
		if (e != nullptr)
		{
			e->adjectives.push_back(u.pA);
		}
	}

	// Adjectives from compounds. ex. "There is a dining table.".
	for (auto &u : s.findUnits("compound"))
	{
		auto e = s.getEntity(u.pAIndex);
		if (e != nullptr)
		{
			e->adjectives.push_back(u.pB);
		}
	}
}

void SELParser::assignRelationships(ParsedSentence &s)
{
	for (auto &u : s.findUnits("nmod:", "NN", "NN"))
	{
		auto eL = s.getEntity(u.pAIndex);
		auto eR = s.getEntity(u.pBIndex);
		if (eL != nullptr && eR != nullptr)
		{
			EntityRelationship rel;
			rel.type = util::split(u.type, ':')[1];
			rel.referencedNoun = eR->baseNoun;
			rel.referencedTokenIndex = eR->tokenIndex;
			eL->relationships.push_back(rel);
		}
	}
}
