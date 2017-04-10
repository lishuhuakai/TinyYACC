#include "GrammarBuilder.h"
#include "Defs.h"
#include "Symbol.h"

GrammarBuilder::GrammarBuilder()
{
}


GrammarBuilder::~GrammarBuilder()
{
}

grammarPtr GrammarBuilder::buildGrammar(set<wstring>& terminal, set<wstring>& nonTerminal, vector<RuleDef>& rules) {
	map<wstring, symbolPtr> syms;
	// 首先构建出所有的符号
	for (auto it = terminal.begin(); it != terminal.end(); ++it) {
		syms[*it] = make_shared<Symbol>(Symbol::Terminal, *it);
	}
	for (auto it = nonTerminal.begin(); it != nonTerminal.end(); ++it) {
		syms[*it] = make_shared<Symbol>(Symbol::NonTerminal, *it);
	}
	return nullptr;
}
