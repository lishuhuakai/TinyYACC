#pragma once
#include "Common.h"
#include <regex>
struct TokenDef;

struct Token {
	wstring kind;
	size_t line;
	size_t column;
	wstring content;
	Token(const wstring& kind, size_t line, size_t column, const wstring& content) :
	kind(kind), line(line), column(column), content(content)
	{}
	// 下面这个构造函数在某些时候也是用得到的!
	Token(const wstring& kind) :
		kind(kind), line(0), column(0)
	{}
	friend wostream& operator<<(wostream& os, Token& tk) {
		os << tk.content << L"[" << tk.kind << L"]" << L"(" << tk.line << L"," << tk.column << L")";
		return os;
	}
};

//
// Lexer是一个词法分析器.
//
class Lexer
{
public:
	Lexer(vector<TokenDef>&, vector<wstring>&);
	~Lexer();
private:
	vector<wregex> regexs_;
	vector<wstring> names_;
	set<wstring> ignore_;
	wstring stream_;
	size_t offset_;					// 用于记录已经解析到了的位置
	size_t line_;					// 记录文本所在的行,列也非常重要
	size_t column_;
	list<Token> cache_;
public:
	void setStream(wstring& stream) {
		offset_ = 0;
		line_ = 1;
		column_ = 1;
		stream_ = stream;
	}
	Token next();
	Token peek();
};

void checkTokens(vector<TokenDef>& tokens);
