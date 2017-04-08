#pragma once
#include "Common.h"
#include "Grammar.h"
#include "Lexer.h"
class Lexer;
class LALRParser;
class GrammarNode;
class StartNode;
class RuleNode;
class ListNode;
class TokenNode;
class AtomNode;
class ExpansionsNode;
class DefineNode;
class ItemNode;
typedef shared_ptr<GrammarNode> grammarNodePtr;

/*
 * GrammarLoader主要用来加载文法.
 */
class GrammarLoader {
public:
	GrammarLoader();
	~GrammarLoader();
private:
	shared_ptr<Grammar>  g_;
public:
	shared_ptr<Lexer> lexer_;
	shared_ptr<LALRParser> parser_;
	friend grammarNodePtr parse(GrammarLoader&, const wstring&);
};

/*
 * 接下来是关于一些Grammar节点的定义.
 */
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
		virtual void visit(ListNode&)=0;
		virtual void visit(StartNode&)=0;
		virtual void visit(RuleNode&)=0;
		virtual void visit(ExpansionsNode&)=0;
		virtual void visit(TokenNode&)=0;
		virtual void visit(DefineNode&)=0;
		virtual void visit(AtomNode&)=0;
		virtual void visit(ItemNode&)=0;
	};
	wstring type_;
};

class StartNode : public GrammarNode {
public:
	StartNode(grammarNodePtr& lst) :
		GrammarNode(L"start"), list_(lst)
	{}
	grammarNodePtr list_;
};

class ListNode : public GrammarNode {
public:
	ListNode(grammarNodePtr& lst, grammarNodePtr& it) :
	GrammarNode(L"list"), list_(lst), item_(it)
	{}
	ListNode() :
	GrammarNode(L"list")
	{}
	
	grammarNodePtr list_;
	grammarNodePtr item_;
};

class RuleNode : public GrammarNode {
public:
	RuleNode(Token& tk, grammarNodePtr& exp) :
		GrammarNode(L"rule"), origin_(tk.content), expansion_(exp)
	{}
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
	grammarNodePtr atom_;
	grammarNodePtr expansions_;
};

class TokenNode : public GrammarNode {
public:
	TokenNode(Token& tk, grammarNodePtr& def) :
		GrammarNode(L"token"), name_(tk.content), def_(def)
	{}
	wstring name_;
	grammarNodePtr def_;
};

class DefineNode : public GrammarNode {
public:
	DefineNode(Token& tk) :
	GrammarNode(L"define")
	{
		if (tk.kind == L"STRING")
			name = tk.content.substr(1, tk.content.size() - 2); // 去掉两侧的""
		else if (tk.kind == L"REGEXP") {
			name = tk.content.substr(2, tk.content.size() - 4); // 去掉正则标记/  /
		}
		else
			assert(0);
	}
	wstring name;
};

class AtomNode : public GrammarNode {
public:
	AtomNode(Token& tk) :
		GrammarNode(L"atom")
	{
		if (tk.kind == L"STRING")
			pattern_ = tk.content.substr(1, tk.content.size() - 2); // 去掉两侧的""
		else if (tk.kind == L"NAME") {
			pattern_ = tk.content;
		}
		else
			assert(0);
	}
	wstring pattern_;
};

class ItemNode : public GrammarNode {
public:
	ItemNode(grammarNodePtr& g):
		GrammarNode(L"item"), sub_(g)
	{}
	ItemNode() :
		GrammarNode(L"item")
	{}
	grammarNodePtr sub_;
};

