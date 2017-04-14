#pragma once
#include "Common.h"

namespace tinyYACC {
	struct RuleDef;
	class SymbolTable;
	//
	// buildGrammar 用于构建出Grammar.
	//
	void buildSymbolTable(const set<wstring>& terminal,const set<wstring>& nonTerminal);
	grammarPtr buildGrammar(const wstring& start, const set<wstring>& terminal, const set<wstring>& nonTerminal, const vector<RuleDef>& rules);
}

