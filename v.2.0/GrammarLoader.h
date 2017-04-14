#pragma once
#include "Common.h"
#include "Grammar.h"
#include "Lexer.h"
#include "Defs.h"

namespace tinyYACC {
	struct RuleDef;
	struct TokenDef;
	class Lexer;
	class LALRParserTable;
	class GrammarNode;
	class RuleNode;
	class LstNode;
	class TokenNode;
	class AtomNode;
	class ExpansionsNode;
	class DefNode;
	class ItemNode;
	class StatementNode;
	class IgnoreNode;
	class StartNode;
	typedef shared_ptr<GrammarNode> grammarNodePtr;

	//
	// GrammarLoader主要用来加载文法.
	//
	class GrammarLoader {
	public:
		GrammarLoader();
		~GrammarLoader();
	private:
		shared_ptr<Grammar>  g_;
	public:
		shared_ptr<SymbolTable> symbols_;
		shared_ptr<Lexer> lexer_;
		shared_ptr<LALRParserTable> parser_;
		friend grammarNodePtr parseGrammar(GrammarLoader&, const wstring&);
	};

	//
	// 接下来是关于一些Grammar节点的定义.
	//
	class GrammarNode {
	public:
		GrammarNode(const wstring& tp) :
			type_(tp)
		{}
		virtual ~GrammarNode()
		{}
	public:
		class IVisitor {
		public:
			virtual void visit(LstNode&) = 0;
			virtual void visit(RuleNode&) = 0;
			virtual void visit(ExpansionsNode&) = 0;
			virtual void visit(TokenNode&) = 0;
			virtual void visit(DefNode&) = 0;
			virtual void visit(AtomNode&) = 0;
			virtual void visit(ItemNode&) = 0;
			virtual void visit(StartNode&) = 0;
			virtual void visit(IgnoreNode&) = 0;
			virtual void visit(StatementNode&) = 0;
		};
		wstring type_;
	public:
		virtual void evaluate(IVisitor& v) = 0;
	};

	class LstNode : public GrammarNode {
	public:
		LstNode(grammarNodePtr& lst, grammarNodePtr& it) :
			GrammarNode(L"lst"), lst_(lst), item_(it)
		{}
		LstNode() :
			GrammarNode(L"lst")
		{}
		void evaluate(IVisitor& v) {
			v.visit(*this);
		}
		grammarNodePtr lst_;
		grammarNodePtr item_;
	};

	class RuleNode : public GrammarNode {
	public:
		RuleNode(Token& tk, grammarNodePtr& exp) :
			GrammarNode(L"rule"), origin_(tk.content), expansion_(exp)
		{}
		void evaluate(IVisitor& v) {
			v.visit(*this);
		}
		grammarNodePtr expansion_;
		wstring origin_;
	};

	class ExpansionsNode : public GrammarNode {
	public:
		ExpansionsNode(grammarNodePtr& et, grammarNodePtr& at) :
			GrammarNode(L"expansions"), atom_(at), expansions_(et)
		{}
		ExpansionsNode() :
			GrammarNode(L"expansions")
		{}
		void evaluate(IVisitor& v) {
			v.visit(*this);
		}
		grammarNodePtr atom_;
		grammarNodePtr expansions_;
	};

	class TokenNode : public GrammarNode {
	public:
		TokenNode(Token& tk, grammarNodePtr& def) :
			GrammarNode(L"token"), name_(tk.content), def_(def)
		{}
		void evaluate(IVisitor& v) {
			v.visit(*this);
		}
		wstring name_;
		grammarNodePtr def_;
	};

	class DefNode : public GrammarNode {
	public:
		DefNode(Token& tk) :
			GrammarNode(L"def")
		{
			if (tk.mark == g_symbolTable.queryMark(L"STRING"))
				name = tk.content.substr(1, tk.content.size() - 2); // 去掉两侧的""
			else if (tk.mark == g_symbolTable.queryMark(L"REGEXP")) {
				name = tk.content.substr(2, tk.content.size() - 4); // 去掉正则标记/  /
			}
			else
				assert(0);
		}
		void evaluate(IVisitor& v) {
			v.visit(*this);
		}
		wstring name;
	};

	class AtomNode : public GrammarNode {
	public:
		enum Attp { STRING, NAME };
		AtomNode(Token& tk) :
			GrammarNode(L"atom")
		{
			if (tk.mark == g_symbolTable.queryMark(L"STRING")) {
				pattern_ = tk.content.substr(1, tk.content.size() - 2); // 去掉两侧的""
				attp_ = STRING;
			}
			else if (tk.mark == g_symbolTable.queryMark(L"NAME")) {
				pattern_ = tk.content;
				attp_ = NAME;
			}
			else
				assert(0);
		}

		void evaluate(IVisitor& v) {
			v.visit(*this);
		}
		wstring pattern_;
		Attp attp_;
	};

	class ItemNode : public GrammarNode {
	public:
		ItemNode(grammarNodePtr& g) :
			GrammarNode(L"item"), sub_(g)
		{}
		ItemNode() :
			GrammarNode(L"item")
		{}
		void evaluate(IVisitor& v) {
			v.visit(*this);
		}
		grammarNodePtr sub_;
	};

	class StatementNode : public GrammarNode {
	public:
		StatementNode(grammarNodePtr& nd) :
			GrammarNode(L"statement"), sub_(nd)
		{}
		grammarNodePtr sub_;
		void evaluate(IVisitor& v) {
			v.visit(*this);
		}
	};

	class IgnoreNode : public GrammarNode {
	public:
		IgnoreNode(Token& tk) :
			GrammarNode(L"ignore"), name_(tk.content)
		{}
		void evaluate(IVisitor& v) {
			v.visit(*this);
		}
		wstring name_;
	};

	class StartNode : public GrammarNode {
	public:
		StartNode(Token& tk) :
			GrammarNode(L"start"), name_(tk.content)
		{}
		void evaluate(IVisitor& v) {
			v.visit(*this);
		}
		wstring name_;
	};

	//
	//	打印一棵树.
	//
	class PrintTree : public GrammarNode::IVisitor {
	private:
		wstring labels_;
		wstring relations_;
		wstring lb_;
	public:
		PrintTree() {};
		void visitTree(GrammarNode& root);
	private:
		void visit(ItemNode&);
		void visit(LstNode&);
		void visit(AtomNode&);
		void visit(DefNode&);
		void visit(RuleNode&);
		void visit(ExpansionsNode&);
		void visit(TokenNode&);
		void visit(StartNode&);
		void visit(IgnoreNode&);
		void visit(StatementNode&);
		static wstring getRandomLabel();
	};

	void printTree(GrammarNode& g);

	//
	//	收集def以及rule的信息.
	//
	class CollectDefsAndRules : public GrammarNode::IVisitor {
	public:
		CollectDefsAndRules() {};
		void collect(GrammarNode& root);
		void printDefsAndRules();
		friend shared_ptr<LALRParserTable> buildParsingTable(CollectDefsAndRules& coll);
	private:
		void visit(ItemNode&);
		void visit(LstNode&);
		void visit(AtomNode&);
		void visit(DefNode&);
		void visit(RuleNode&);
		void visit(ExpansionsNode&);
		void visit(TokenNode&);
		void visit(StartNode&);
		void visit(IgnoreNode&);
		void visit(StatementNode&);
	private:
		wstring patternOrName_;
		shared_ptr<RuleDef> r_;
		vector<RuleDef> rules_;
		list<TokenDef> tokens_;
		vector<wstring> start_;		// 用于记录开始的Token
		set<wstring> ignore_;		// 一些需要被忽略的Token
		set<wstring> terminal_;		// 终结符
		set<wstring> nonTerminal_;  // 非终结符
	};

	wstring readFile(const wstring& fileName);
	void collectDefsAndRules(GrammarNode& nd);
}
