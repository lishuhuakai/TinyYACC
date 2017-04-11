#pragma once
#include "Common.h"

struct RuleDef;

//
// buildGrammar 用于构建出Grammar.
//
grammarPtr buildGrammar(wstring&, set<wstring>&, set<wstring>&, vector<RuleDef>&);

