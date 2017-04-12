#include "GrammarBuilder.h"
#include "Defs.h"
#include "Rule.h"
#include "Grammar.h"
#include "SymbolTable.h"
#include "LALRParserTable.h"
#include "GrammarLoader.h"

namespace tinyYACC {
	//
	// buildSymbolTable 构造一张Symbol Table.
	//
	void buildSymbolTable(set<wstring>& terminal, set<wstring>& nonTerminal)
	{
		g_symbolTable.clear();
		for (auto it = terminal.begin(); it != terminal.end(); ++it) {
			g_symbolTable.appendType(*it, true);
		}
		for (auto it = nonTerminal.begin(); it != nonTerminal.end(); ++it) {
			g_symbolTable.appendType(*it, false);
		}
	}
	
	
	grammarPtr buildGrammar(wstring& s, set<wstring>& terminal, set<wstring>& nonTerminal, vector<RuleDef>& rules) {
		// 首先构建出所有的符号, 并往g_typeMapping中添加.
		// 第一条文法是增广文法,所谓增广文法,是我们自己添加的一条文法.添加的文法不影响原有文法,但是会给我们后来的处理带来方便.
		rulePtr r0 = make_shared<Rule>(g_symbolTable.startMark);
		r0->expansionAppend(g_symbolTable[s]);
		r0->expansionAppend(g_symbolTable.endMark);
		grammarPtr g = make_shared<Grammar>(g_symbolTable.startMark); // start符号是我们增广的开始符号
		g->appendRule(r0);
		// 遍历文法
		for (auto it = rules.begin(); it != rules.end(); ++it) {
			rulePtr r = make_shared<Rule>(g_symbolTable[it->origin]);
			for (auto elem : it->expansions) {
				r->expansionAppend(g_symbolTable[elem]);
			}
			g->appendRule(r);
		}
		return g;
	}

	//
	// buildParsingTable 构造出Parsing Table.
	//
	shared_ptr<LALRParserTable> buildParsingTable(CollectDefsAndRules& coll) {
		buildSymbolTable(coll.terminal_, coll.nonTerminal_);
		auto grammar = buildGrammar(coll.start_[0], coll.terminal_, coll.nonTerminal_, coll.rules_);
		shared_ptr<LALRParserTable> table = make_shared<LALRParserTable>(*grammar);
		table->computerLookAhead();
		return table;
	}
}
