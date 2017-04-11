#include "GrammarBuilder.h"
#include "Defs.h"
#include "Symbol.h"
#include "Rule.h"
#include "Grammar.h"

grammarPtr buildGrammar(wstring& s, set<wstring>& terminal, set<wstring>& nonTerminal, vector<RuleDef>& rules) {
	map<wstring, symbolPtr> syms;
	// 首先构建出所有的符号
	symbolPtr start = make_shared<Symbol>(Symbol::NonTerminal, L"#start");
	symbolPtr end = make_shared<Symbol>(Symbol::Terminal, L"#end");
	for (auto it = terminal.begin(); it != terminal.end(); ++it) {
		syms[*it] = make_shared<Symbol>(Symbol::Terminal, *it);
	}
	for (auto it = nonTerminal.begin(); it != nonTerminal.end(); ++it) {
		syms[*it] = make_shared<Symbol>(Symbol::NonTerminal, *it);
	}
	// 第一条文法是增广文法,所谓增广文法,是我们自己添加的一条文法.添加的文法不影响原有文法,但是会给我们后来的处理带来方便.
	rulePtr r0 = make_shared<Rule>(start);
	r0->expansionAppend(syms[s]);
	r0->expansionAppend(end);
	grammarPtr g = make_shared<Grammar>(start); // start符号是我们增广的开始符号
	g->appendRule(r0);
	// 遍历文法
	for (auto it = rules.begin(); it != rules.end(); ++it) {
		rulePtr r = make_shared<Rule>(syms[it->origin]);
		for (auto elem : it->expansions) {
			r->expansionAppend(syms[elem]);
		}
		g->appendRule(r);
	}
	return g;
}
