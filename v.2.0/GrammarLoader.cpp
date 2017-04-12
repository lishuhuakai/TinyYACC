#include "Common.h"
#include "LALRParser.h"
#include "GrammarLoader.h"
#include "Defs.h"
#include "Lexer.h"
#include "GrammarBuilder.h"
#include "SymbolTable.h"

namespace tinyYACC {
	static wstring _start = wstring(L"lst");

	static vector<TokenDef> _tokens = {
		{ wstring(L"COLON"), wstring(L":")},					// 冒号
		{ wstring(L"NAME"), wstring(L"[0-9a-zA-Z_]*")},
		{ wstring(L"STRING"), wstring(L"\"(\"|\\\\|[^\"/\n])*?\"")},
		{ wstring(L"REGEXP"), wstring(L"/\\s([^\n])+\\s/")},
		{ wstring(L"NL"), wstring(L"(\\r?\\n)+")},				// 回车,换行,NL表示new line
		{ wstring(L"WS"), wstring(L"[\\t\\s]+")},
		{ wstring(L"COMMENT"), wstring(L"//[^\\n]*")},
		{ wstring(L"TO"), wstring(L"-->")},
		{ wstring(L"IGNORE"), wstring(L"%ignore")},
		{ wstring(L"START"), wstring(L"%start")}
	};

	static set<wstring> _nonTerminal = {
		wstring(L"def"), wstring(L"lst"), 
		wstring(L"item"), wstring(L"rule"),
		wstring(L"token"), wstring(L"expansions"), 
		wstring(L"atom"), wstring(L"statement"), 
		wstring(L"ignore"), wstring(L"start")
	};

	static set<wstring> _terminal = {
		wstring(L"NAME"), wstring(L"NL"), 
		wstring(L"TO"), wstring(L"STRING"), 
		wstring(L"REGEXP"), wstring(L"COLON"),
		wstring(L"IGNORE"), wstring(L"START"),
		wstring(L"WS"), wstring(L"COMMENT")
	};

	static vector<RuleDef> _rules = {
		{ wstring(L"lst"), { wstring(L"lst"), wstring(L"item")}},
		{ wstring(L"lst"), {}},
		{ wstring(L"item"), { wstring(L"rule")}},
		{ wstring(L"item"), { wstring(L"token")}},
		{ wstring(L"item"), { wstring(L"NL")}},
		{ wstring(L"item"), { wstring(L"statement")}},
		{ wstring(L"statement"), { wstring(L"ignore")}},
		{ wstring(L"statement"), { wstring(L"start")}},
		{ wstring(L"ignore"), { wstring(L"IGNORE"), wstring(L"NAME"), wstring(L"NL")}},
		{ wstring(L"start"), { wstring(L"START"), wstring(L"NAME"), wstring(L"NL")}},
		{ wstring(L"rule"), { wstring(L"NAME"), wstring(L"TO"), wstring(L"expansions"), wstring(L"NL")}},
		{ wstring(L"expansions"), { wstring(L"expansions"), wstring(L"atom")}},
		{ wstring(L"expansions"), {}},
		{ wstring(L"atom"), { wstring(L"NAME")}},
		{ wstring(L"atom"), { wstring(L"STRING")}},
		{ wstring(L"token"), { wstring(L"NAME"), wstring(L"COLON"), wstring(L"def"), wstring(L"NL")}},
		{ wstring(L"def"), { wstring(L"REGEXP")}},
		{ wstring(L"def"), { wstring(L"STRING")}}
	};

	vector<wstring> _ignore = { wstring(L"WS"), wstring(L"COMMENT") };

	GrammarLoader::GrammarLoader() {
		buildSymbolTable(_terminal, _nonTerminal);
		lexer_ = make_shared<Lexer>(_tokens, _ignore);
		g_ = buildGrammar(_start, _terminal, _nonTerminal, _rules);
		parser_ = make_shared<LALRParser>(*g_);
	}

	GrammarLoader::~GrammarLoader()
	{
	}

	//
	//	parse相关.
	//
	wstring readFile(wstring fileName) {
		wifstream f(fileName);
		return wstring(istreambuf_iterator<wchar_t>(f), istreambuf_iterator<wchar_t>());
	}

	grammarNodePtr parseGrammar(GrammarLoader& loader, const wstring& fileName) {
		typedef LALRParser::Action Action;
		auto parser = loader.parser_;
		auto lexer = loader.lexer_;
		parser->computerLookAhead();
		//loader.g_->printSets();
		//parser->printStats();
		lexer->setStream(readFile(fileName));
		// two stacks are necessary, one for state, another for value
		stack<shared_ptr<GrammarNode>> tree;
		stack<size_t> stateStack;
		stack<Token> valueStack;
		// firstly, we should push the first state onto stateStack.
		stateStack.push(parser->start_);
		size_t state;
		try {
			while (true) {
				state = stateStack.top();
				Token tk = lexer->peek();
				// query the table to know how to manipulate the elemt in valueStack
				Action act = parser->queryAction(state, tk.mark);
				if (act.act == Action::shift) { // shift required!
					auto kk = lexer->next();
					if (kk.mark == g_symbolTable.endMark) break;
					stateStack.push(act.index);
					valueStack.push(tk);
				}
				else { // reduce required!
					rulePtr r = parser->queryRule(act.index);
					auto origin = r->origin();
					if (origin == g_symbolTable[L"atom"]) {
						// atom -> STRING
						// atom -> NAME
						auto pattern = valueStack.top(); stateStack.pop();
						tree.push(make_shared<AtomNode>(pattern)); valueStack.pop();
					}
					else if (origin == g_symbolTable[L"def"]) {
						// def -> REGEXP
						// def -> STRING
						auto pattern = valueStack.top(); stateStack.pop();
						tree.push(make_shared<DefNode>(pattern)); valueStack.pop();
					}
					else if (origin == g_symbolTable[L"token"]) {
						// token -> NAME COLON def NL
						valueStack.pop(); stateStack.pop(); // NL
						valueStack.pop(); stateStack.pop(); // def
						valueStack.pop(); stateStack.pop(); // COLON
						auto NAME = valueStack.top(); valueStack.pop(); stateStack.pop(); // NAME
						auto def = tree.top(); tree.pop();

						tree.push(make_shared<TokenNode>(NAME, def));
					}
					else if (origin == g_symbolTable[L"rule"]) {
						// rule -> NAME TO expansions NL
						valueStack.pop(); stateStack.pop(); // NL
						valueStack.pop(); stateStack.pop(); // expansions
						valueStack.pop(); stateStack.pop(); // TO
						auto NAME = valueStack.top(); valueStack.pop(); stateStack.pop(); // NAME
						auto exps = tree.top(); tree.pop();
						tree.push(make_shared<RuleNode>(NAME, exps));
					}
					else if (origin == g_symbolTable[L"expansions"]) {
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
					else if (origin == g_symbolTable[L"item"]) {
						// item -> rule | token | NL
						valueStack.pop(); stateStack.pop(); // rule or token or NL
						if (r->findNthElem(0) != g_symbolTable[L"NL"]) {
							auto ruleOrToken = tree.top(); tree.pop();
							tree.push(make_shared<ItemNode>(ruleOrToken));
						}
						else tree.push(make_shared<ItemNode>());
					}
					else if (origin == g_symbolTable[L"lst"]) {
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
					else if (origin == g_symbolTable[L"statement"]) {
						// statement -> ignore | start
						valueStack.pop(); stateStack.pop();	// ignore or start
						auto ign_or_start = tree.top(); tree.pop();
						tree.push(make_shared<StatementNode>(ign_or_start));
					}
					else if (origin == g_symbolTable[L"ignore"]) {
						// ignore -> IGNORE NAME NL
						valueStack.pop(); stateStack.pop(); // IGNORE
						auto NAME = valueStack.top();
						valueStack.pop(); stateStack.pop(); // NAME
						valueStack.pop(); stateStack.pop(); // NL
						tree.push(make_shared<IgnoreNode>(NAME));
					}
					else if (origin == g_symbolTable[L"start"]) {
						// start -> START NAME NL
						valueStack.pop(); stateStack.pop(); // START
						auto NAME = valueStack.top();
						valueStack.pop(); stateStack.pop(); // NAME
						valueStack.pop(); stateStack.pop(); // NL
						tree.push(make_shared<StartNode>(NAME));
					}
					else assert(0);

					valueStack.push(Token(r->origin()));
					auto act = parser->queryAction(stateStack.top(), valueStack.top().mark);
					assert(act.act == Action::shift);
					stateStack.push(act.index);
				}
			}
		}
		catch (GeneralError& err) {
			wcout << err.msg << endl;
			return nullptr;
		}
		catch (GrammarError& err) {
			wcout << err.what() << endl;
			return nullptr;
		}
		assert(tree.size() == 1);
		return tree.top();
	}

	//
	// 展示一棵ast. 下面的内容只是为了Debug,和真正的核心代码关系不大.
	//
	void printTree(GrammarNode& g) {
		PrintTree().visitTree(g);
	}


	//
	// getRaomLabel 获取一个随机的,但是每次都不一样的label,实际上也不是每次都不一样,只是有一个周期而已.
	//
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

	void PrintTree::visitTree(GrammarNode& root) {
		wofstream file("SyntaxTree.gv", wios::out);
		root.evaluate(*this);
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
			lst.lst_->evaluate(*this);
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
			labels_ = labels_ + lb + L"[label=\"ItemNode(NL)\"];\n";
		else {
			labels_ = labels_ + lb + L"[label=\"ItemNode\"];\n";
			it.sub_->evaluate(*this);
			relations_ = relations_ + lb + L"->" + lb_ + L"\n";
		}
		lb_ = lb;
	}

	void PrintTree::visit(StatementNode& stat) {
		wstring lb = getRandomLabel();
		labels_ = labels_ + lb + L"[label=\"StatementNode\"]\n";
		stat.sub_->evaluate(*this);
		relations_ = relations_ + lb + L"->" + lb_ + L"\n";
		lb_ = lb;
	}

	void PrintTree::visit(StartNode& start) {
		wstring lb = getRandomLabel();
		labels_ = labels_ + lb + L"[label=\"StartNode(" + start.name_ + L")\"]";
		lb_ = lb;
	}

	void PrintTree::visit(IgnoreNode& ign) {
		wstring lb = getRandomLabel();
		labels_ = labels_ + lb + L"[label=\"IgnoreNode(" + ign.name_ + L")\"]";
		lb_ = lb;
	}


	//
	//	CollectDefAndRule主要从ast中获取token的定义和文法.
	//
	void CollectDefsAndRules::printDefsAndRules() {
		wcout << L"##########Rules" << endl;
		for (auto r : rules_)
			wcout << r << endl;
		wcout << L"##########Tokens" << endl;
		for (auto tk : tokens_)
			wcout << tk << endl;
		wcout << L"####start: " << start_[0] << endl;
		wcout << L"##########ignore: " << endl;
		for (auto ign : ignore_) {
			wcout << ign << endl;
		}
	}

	void CollectDefsAndRules::collect(GrammarNode & root)
	{
		root.evaluate(*this);
		// printDefsAndRules();
		// 接下里需要对收集到的Rule的Token进行检查
		for (auto tk = tokens_.begin(); tk != tokens_.end(); ) {
			if (terminal_.find(tk->name) != terminal_.end()) {
				if (tk->name[0] == L'#') {	// anonymous, it doesn't matter. 
					list<TokenDef>::iterator temp = tk;
					++tk;
					tokens_.erase(temp);
				}
				else {
					GeneralError error;
					error.msg = L"Token " + tk->name + L" 存在多重定义!";
					throw error;
				}
			}
			else {
				terminal_.insert(tk->name);
				++tk;
			}
		}

		// 接下来对每一条规则进行分析
		for (auto r : rules_) {
			if (terminal_.find(r.origin) != terminal_.end()) {
				GeneralError error;
				error.msg = L"# 重复的类型定义:" + r.origin + L"(既是终结符,也是非终结符). #";
				throw error;
			}
			nonTerminal_.insert(r.origin);
		}

		// 每一个expansion的元素,要么是终结符,要么是非终结符
		for (auto r : rules_) {
			for (auto elem : r.expansions) {
				bool isTerminal = terminal_.find(elem) != terminal_.end();
				bool isNonTerminal = nonTerminal_.find(elem) != nonTerminal_.end();
				if (!isTerminal && !isNonTerminal) {
					GeneralError error;
					error.msg = L"# 未定义的类型:" + elem + L". #";
					throw error;
				}
			}
		}

		if (start_.size() != 1) {
			GeneralError error;
			error.msg = L"# 存在多个开始符号或者不存在开始符号,这是不被允许的! #";
			throw error;
		}
		if (nonTerminal_.find(start_[0]) == nonTerminal_.end()) {
			GeneralError error;
			error.msg = L"# 找不到开始符号的定义! #";
			throw error;
		}

		// 接下里要保证ignore里面的定义全部是非终结符
		for (auto sym : ignore_) {
			if (terminal_.find(sym) == terminal_.end()) {
				GeneralError error;
				error.msg = L"# ignore语句中的" + sym + L"没有定义! #";
				throw error;
			}
		}
	}

	void CollectDefsAndRules::visit(ItemNode &it)
	{
		// item -> rule | token
		if (it.sub_) {
			it.sub_->evaluate(*this);
		}
	}

	void CollectDefsAndRules::visit(LstNode &lst)
	{
		// lst -> lst item | ε
		if (lst.lst_) {
			lst.lst_->evaluate(*this);
			lst.item_->evaluate(*this);
		}
	}

	void CollectDefsAndRules::visit(AtomNode& at)
	{
		// at -> STRING | NAME
		if (at.attp_ == AtomNode::STRING) {
			// anoymous
			patternOrName_ = L"#anoymous_" + at.pattern_;
			TokenDef tk = { patternOrName_, at.pattern_ };
			tokens_.push_back(tk);
		}
		else
			patternOrName_ = at.pattern_;
	}

	void CollectDefsAndRules::visit(DefNode& def)
	{
		// def -> STRING | REGEXP
		patternOrName_ = def.name;
	}

	void CollectDefsAndRules::visit(RuleNode& r)
	{
		// rule -> NAME TO expansions NL
		r_ = make_shared<RuleDef>();
		r_->origin = r.origin_;
		// 接下来是expansions
		r.expansion_->evaluate(*this);
		rules_.push_back(*r_);
		r_.reset();
	}

	void CollectDefsAndRules::visit(ExpansionsNode& exps)
	{
		// expansions -> expansions atom | ε
		if (exps.expansions_) {
			exps.expansions_->evaluate(*this);
			exps.atom_->evaluate(*this);
			r_->expansions.push_back(patternOrName_);
		}
	}

	void CollectDefsAndRules::visit(TokenNode& tk)
	{
		TokenDef token;
		token.name = tk.name_;
		tk.def_->evaluate(*this);
		token.pattern = patternOrName_;
		tokens_.push_back(token);
	}

	void CollectDefsAndRules::visit(StartNode& start) {
		start_.push_back(start.name_);
	}

	void CollectDefsAndRules::visit(IgnoreNode& start) {
		ignore_.insert(start.name_);
	}

	void CollectDefsAndRules::visit(StatementNode& stat) {
		stat.sub_->evaluate(*this);
	}

	void collectDefsAndRules(GrammarNode& nd) {
		try {
			CollectDefsAndRules().collect(nd);
		}
		catch (GeneralError& error) {
			wcout << error.msg << endl;
		}
	}
}
