#pragma once
#include "Common.h"

//
// GrammarBuilder 用于构建出Grammar.
//
class GrammarBuilder
{
public:
	GrammarBuilder();
	~GrammarBuilder();
public:
	grammarPtr buildGrammar(set<wstring>&, set<wstring>&, vector<RuleDef>&);
};

