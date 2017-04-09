#include "Common.h"
#include "LALRParser.h"
#include "GrammarLoader.h"
#include "TokenDef.h"
#include "Lexer.h"

/*
 * 这个文件定义一些全局的变量.
 */
static vector<TokenDef> _tokens = {
	TokenDef(wstring(L"DOT"), wstring(L".")),
	TokenDef(wstring(L"COMMA"), wstring(L",")),
	TokenDef(wstring(L"COLON"), wstring(L":")),
	TokenDef(wstring(L"SEMICOLON"), wstring(L";")),
	TokenDef(wstring(L"PLUS"), wstring(L"+")),
	TokenDef(wstring(L"MINUS"), wstring(L"-")),
	TokenDef(wstring(L"STAR"), wstring(L"*")),
	TokenDef(wstring(L"SLASH"), wstring(L"/")),
	TokenDef(wstring(L"BACKSLASH"), wstring(L"\\")),
	TokenDef(wstring(L"VBAR"), wstring(L"|")),
	TokenDef(wstring(L"QMARK"), wstring(L"?")),
	TokenDef(wstring(L"BANG"), wstring(L"!")),
	TokenDef(wstring(L"AT"), wstring(L"@")),
	TokenDef(wstring(L"HASH"), wstring(L"#")),
	TokenDef(wstring(L"DOLLAR"), wstring(L"$")),
	TokenDef(wstring(L"PERCENT"), wstring(L"%")),
	TokenDef(wstring(L"CIRCUMFLEX"), wstring(L"^")),
	TokenDef(wstring(L"AMPERSAND"), wstring(L"&")),
	TokenDef(wstring(L"UNDERSCORE"), wstring(L"_")),
	TokenDef(wstring(L"LESSTHAN"), wstring(L"<")),
	TokenDef(wstring(L"EQUAL"), wstring(L"=")),
	TokenDef(wstring(L"DBLQUOTE"), wstring(L"\"")),
	TokenDef(wstring(L"QUOTE"), wstring(L"'")),
	TokenDef(wstring(L"LPAR"), wstring(L"(")),
	TokenDef(wstring(L"RPAR"), wstring(L")")),
	TokenDef(wstring(L"LBRACE"), wstring(L"{")),
	TokenDef(wstring(L"RBRACE"), wstring(L"}")),
	TokenDef(wstring(L"LSQB"), wstring(L"[")),
	TokenDef(wstring(L"RSQB"), wstring(L"]")),
	TokenDef(wstring(L"NEWLINE"), wstring(L"\n")),
	TokenDef(wstring(L"CRLF"), wstring(L"\r\n")),
	TokenDef(wstring(L"TAB"), wstring(L"\t")),
	TokenDef(wstring(L"SPACE"), wstring(L" "))
};

static vector<TokenDef> _grammarTokens = {
	TokenDef(wstring(L"COLON"), wstring(L":")),		/* 冒号 */
	TokenDef(wstring(L"NAME"), wstring(L"[0-9a-zA-Z_]*")),  /*  */
	TokenDef(wstring(L"STRING"), wstring(L"\"(\"|\\\\|[^\"/\n])*?\"")),
	TokenDef(wstring(L"REGEXP"), wstring(L"/\\s([^\n])+\\s/")), /* */
	TokenDef(wstring(L"NL"), wstring(L"(\\r?\\n)+")), /* 回车,换行,NL表示new line */
	TokenDef(wstring(L"WS"), wstring(L"[\\t\\s]+")),
	TokenDef(wstring(L"COMMENT"), wstring(L"//[^\\n]*")),
	TokenDef(wstring(L"TO"), wstring(L"-->"))
};


/*
 * 在构造函数里面需要加载最初的文法规则.
 */
GrammarLoader::GrammarLoader() {
	/* 在这里,我需要重新构建一套文法 */
	vector<wstring> ignore = { wstring(L"WS"), wstring(L"COMMENT") };
	lexer_ = make_shared<Lexer>(_grammarTokens, ignore);
	symbolPtr start			=		make_shared<Symbol>(Symbol::NonTerminal, L"start");
	symbolPtr def			=		make_shared<Symbol>(Symbol::NonTerminal, L"def");
	symbolPtr lst			=		make_shared<Symbol>(Symbol::NonTerminal, L"lst");
	symbolPtr item			=		make_shared<Symbol>(Symbol::NonTerminal, L"item");
	symbolPtr rule			=		make_shared<Symbol>(Symbol::NonTerminal, L"rule");
	symbolPtr token			=		make_shared<Symbol>(Symbol::NonTerminal, L"token");
	symbolPtr expansions	=		make_shared<Symbol>(Symbol::NonTerminal, L"expansions");
	symbolPtr atom			=		make_shared<Symbol>(Symbol::NonTerminal, L"atom");
	symbolPtr name			=		make_shared<Symbol>(Symbol::Terminal, L"NAME");
	symbolPtr end			=		make_shared<Symbol>(Symbol::Terminal, L"#");
	symbolPtr nl			=		make_shared<Symbol>(Symbol::Terminal, L"NL");
	symbolPtr to			=		make_shared<Symbol>(Symbol::Terminal, L"TO");
	symbolPtr _string		=		make_shared<Symbol>(Symbol::Terminal, L"STRING");
	symbolPtr regular		=		make_shared<Symbol>(Symbol::Terminal, L"REGEXP");
	symbolPtr colon			=		make_shared<Symbol>(Symbol::Terminal, L"COLON");

	// start -> lst #
	rulePtr r0 = make_shared<Rule>(start);
	r0->expansionAppend(lst);
	r0->expansionAppend(end);
	// lst -> lst item
	rulePtr r1 = make_shared<Rule>(lst);
	r1->expansionAppend(lst);
	r1->expansionAppend(item);
	// lst -> ε
	rulePtr r2 = make_shared<Rule>(lst);
	// item -> rule
	rulePtr r3 = make_shared<Rule>(item);
	r3->expansionAppend(rule);
	// item -> token
	rulePtr r4 = make_shared<Rule>(item);
	r4->expansionAppend(token);
	// item -> NL [换行符]
	rulePtr r5 = make_shared<Rule>(item);
	r5->expansionAppend(nl);
	// rule -> NAME TO expansions NL
	rulePtr r6 = make_shared<Rule>(rule);
	r6->expansionAppend(name);
	r6->expansionAppend(to);
	r6->expansionAppend(expansions);
	r6->expansionAppend(nl);
	// expansions -> expansions atom
	rulePtr r7 = make_shared<Rule>(expansions);
	r7->expansionAppend(expansions);
	r7->expansionAppend(atom);
	// expansions -> ε
	rulePtr r8 = make_shared<Rule>(expansions);
	// atom -> NAME
	rulePtr r9 = make_shared<Rule>(atom);
	r9->expansionAppend(name);
	// atom -> STRING
	rulePtr r10 = make_shared<Rule>(atom);
	r10->expansionAppend(_string);
	// token -> NAME COLON def nl
	rulePtr r11 = make_shared<Rule>(token);
	r11->expansionAppend(name);
	r11->expansionAppend(colon);
	r11->expansionAppend(def);
	r11->expansionAppend(nl);
	// def -> REGEXP
	rulePtr r12 = make_shared<Rule>(def);
	r12->expansionAppend(regular);
	// def -> STRING
	rulePtr r13 = make_shared<Rule>(def);
	r13->expansionAppend(_string);

	g_ = make_shared<Grammar>(start);
	g_->appendRule(r0);
	g_->appendRule(r1);
	g_->appendRule(r2);
	g_->appendRule(r3);
	g_->appendRule(r4);
	g_->appendRule(r5);
	g_->appendRule(r6);
	g_->appendRule(r7);
	g_->appendRule(r8);
	g_->appendRule(r9);
	g_->appendRule(r10);
	g_->appendRule(r11);
	g_->appendRule(r12);
	g_->appendRule(r13);
	assert(g_);
	parser_ = make_shared<LALRParser>(*g_);
}

GrammarLoader::~GrammarLoader()
{
}

/**********************************************************************
	parse相关
**********************************************************************/
wstring readFile(wstring fileName) {
	wifstream f(fileName);
	return wstring(istreambuf_iterator<wchar_t>(f), istreambuf_iterator<wchar_t>());
}

grammarNodePtr parse(GrammarLoader& loader, const wstring& fileName) {
	typedef LALRParser::Action Action;
	auto parser = loader.parser_;
	auto lexer = loader.lexer_;
	parser->computerLookAhead();
	//loader.g_->printSets();
	//parser->printStats();
	lexer->setStream(readFile(fileName));
	bool over = false;
	// two stacks are necessary, one for state, another for value
	stack<shared_ptr<GrammarNode>> tree;
	stack<size_t> stateStack;
	stack<Token> valueStack;
	// firstly, we should push the first state onto stateStack.
	stateStack.push(parser->start_);
	size_t state;
	while (true) {
		state = stateStack.top(); 
		Token tk = lexer->peek();
		// query the table to know how to manipulate the elemt in valueStack
		Action act = parser->queryAction(state, loader.g_->strToSymbol(tk.kind));
		if (act.act == Action::shift) { // shift required!
			auto kk = lexer->next();
			if (kk.kind == L"#") break; 
			stateStack.push(act.index);
			valueStack.push(tk);
		}
		else { // reduce required!
			//stateStack.pop();
			rulePtr r = parser->queryRule(act.index);
			auto origin = r->origin();
			if (origin->content_ == L"atom") {
				// atom -> STRING
				// atom -> NAME
				auto pattern = valueStack.top(); stateStack.pop(); 
				tree.push(make_shared<AtomNode>(pattern)); valueStack.pop();
			}
			else if (origin->content_ == L"def") {
				// def -> REGEXP
				// def -> STRING
				auto pattern = valueStack.top(); stateStack.pop(); 
				tree.push(make_shared<DefNode>(pattern)); valueStack.pop();
			}
			else if (origin->content_ == L"token") {
				// token -> NAME COLON def NL
				valueStack.pop(); stateStack.pop(); // NL
				valueStack.pop(); stateStack.pop(); // def
				valueStack.pop(); stateStack.pop(); // COLON
				auto NAME = valueStack.top(); valueStack.pop(); stateStack.pop(); // NAME
				auto def = tree.top(); tree.pop();

				tree.push(make_shared<TokenNode>(NAME, def));
			}
			else if (origin->content_ == L"rule") {
				// rule -> NAME TO expansions NL
				valueStack.pop(); stateStack.pop(); // NL
				valueStack.pop(); stateStack.pop(); // expansions
				valueStack.pop(); stateStack.pop(); // TO
				auto NAME = valueStack.top(); valueStack.pop(); stateStack.pop(); // NAME
				auto exps = tree.top(); tree.pop();
				tree.push(make_shared<RuleNode>(NAME, exps));
			}
			else if (origin->content_ == L"expansions") {
				// expansions -> expansions atom | ε
				if (r->expansionLength() == 0) {
					tree.push(make_shared<ExpansionsNode>());
				}
				else {
					valueStack.pop(); stateStack.pop(); // atom
					valueStack.pop(); stateStack.pop(); // expansions
					auto atom = tree.top(); tree.pop();
					auto exps = tree.top(); tree.pop();
					tree.push(make_shared<ExpansionsNode>(exps, atom));
				}
			}
			else if (origin->content_ == L"item") {
				// item -> rule | token | NL
				valueStack.pop(); stateStack.pop(); // rule or token or NL
				if (r->findNthElem(0)->content_ != L"NL") {
					auto ruleOrToken = tree.top(); tree.pop();
					tree.push(make_shared<ItemNode>(ruleOrToken));
				}
				else tree.push(make_shared<ItemNode>());
			}
			else if (origin->content_ == L"lst") {
				// lst -> lst item | ε
				if (r->isEpsRule()) {
					tree.push(make_shared<LstNode>());
				}
				else {
					valueStack.pop(); stateStack.pop(); // item
					valueStack.pop(); stateStack.pop(); // lst
					auto item = tree.top(); tree.pop();
					auto lst = tree.top(); tree.pop();
					tree.push(make_shared<LstNode>(lst, item));
				}
			}
			else assert(0);
			valueStack.push(Token(r->origin()->content_));
			auto act = parser->queryAction(stateStack.top(), loader.g_->strToSymbol(valueStack.top().kind));
			assert(act.act == Action::shift);
			stateStack.push(act.index);
		}
	}
	assert(tree.size() == 1);
	return tree.top();
}

/*************************************************************************************
 展示一棵ast,通过将节点内容. 下面的内容只是为了查看方便而已,和真正的核心代码没有任何关系.
**************************************************************************************/
void printTree(GrammarNode& g) {
	PrintTree p(g);
	p.visitTree();
}

PrintTree::PrintTree(GrammarNode& root) :
	root_(root)
{}

/*
* getRaomLabel  获取一个随机的,但是每次都不一样的label,实际上也不是每次都不一样,只是有一个周期而已.
*/
wstring PrintTree::getRandomLabel() {
	wstring label;
	static wchar_t first = L'a';
	static wchar_t mid = L'A';
	static wchar_t num1 = L'0';
	static wchar_t num2 = L'1';
	label += first; first++;
	label += mid; mid++;
	label += num1; num1++;
	label += num2; num2++;
	if (first > L'z') first = L'a';
	if (mid > L'Z') mid = L'A';
	if (num1 > L'9') num1 = L'0';
	if (num2 > L'9') num2 = L'0';
	return label;
}

void PrintTree::visitTree() {
	wofstream file("SyntaxTree.gv", wios::out);
	root_.evaluate(*this);
	if (file.is_open()) {
		file << L"digraph syntaxTree {" << endl;
		file << labels_.c_str();
		file << relations_.c_str();
		file << L"}" << endl;
	}
	file.close();
}

void PrintTree::visit(LstNode& lst) {
	wstring lb = getRandomLabel();
	if (!lst.item_)
		labels_ = labels_ + lb + L"[label=\"LstNode(epsilon)" + L"\"];\n";
	else {
		// lst -> lst item
		labels_ = labels_ + lb + L"[label=\"LstNode\"];\n";
		lst.list_->evaluate(*this);
		relations_ = relations_ + lb + L"->" + lb_ + L"\n";
		lst.item_->evaluate(*this);
		relations_ = relations_ + lb + L"->" + lb_ + L"\n";
	}
	lb_ = lb;
}

void PrintTree::visit(RuleNode& r) {
	wstring lb = getRandomLabel();
	labels_ = labels_ + lb + L"[label=\"RuleNode(" + r.origin_ + L")\"];\n";
	r.expansion_->evaluate(*this);
	relations_ = relations_ + lb + L"->" + lb_ + L"\n";
	lb_ = lb;
}

void PrintTree::visit(DefNode& def) {
	wstring lb = getRandomLabel();
	labels_ = labels_ + lb + L"[label=\"DefNode(" + def.name + L")\"];\n";
	lb_ = lb;
}

void PrintTree::visit(TokenNode& tk) {
	wstring lb = getRandomLabel();
	labels_ = labels_ + lb + L"[label=\"TokenNode(" + tk.name_ + L")\"];\n";
	tk.def_->evaluate(*this);
	relations_ = relations_ + lb + L"->" + lb_ + L"\n";
	lb_ = lb;
}

void PrintTree::visit(ExpansionsNode& exps) {
	wstring lb = getRandomLabel();
	if (!exps.expansions_)
		labels_ = labels_ + lb + L"[label=\"ExpansionsNode(epsilon)\"];\n";
	else {
		labels_ = labels_ + lb + L"[label=\"ExpansionsNode\"];\n";
		exps.expansions_->evaluate(*this);
		relations_ = relations_ + lb + L"->" + lb_ + L"\n";
		exps.atom_->evaluate(*this);
		relations_ = relations_ + lb + L"->" + lb_ + L"\n";
	}
	lb_ = lb;
}

void PrintTree::visit(AtomNode& at) {
	wstring lb = getRandomLabel();
	labels_ = labels_ + lb + L"[label=\"AtomNode(" + at.pattern_ + L")\"];\n";
	lb_ = lb;
}

void PrintTree::visit(ItemNode& it) {
	wstring lb = getRandomLabel();
	if (!it.sub_)
		labels_ = labels_ + lb + L"[label=\"ItemNode(epsilon)\"];\n";
	else {
		labels_ = labels_ + lb + L"[label=\"ItemNode\"];\n";
		it.sub_->evaluate(*this);
		relations_ = relations_ + lb + L"->" + lb_ + L"\n";
	}
	lb_ = lb;
}