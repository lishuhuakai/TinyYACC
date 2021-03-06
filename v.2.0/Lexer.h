#pragma once
#include "Common.h"
#include <regex>
#include "SymbolTable.h"

namespace tinyYACC {
	struct TokenDef;
	//
	// Token 词法分析器每次会吐出一个词法单元,即token
	// 
	struct Token {
		int mark;
		size_t line;
		wstring content;
		Token(int mark, size_t line, const wstring& content) :
			mark(mark), line(line) , content(content)
		{}
		// 下面这个构造函数在某些时候也是用得到的!
		Token(int mark) :
			mark(mark), line(0)
		{}
		friend wostream& operator<<(wostream& os, const Token& tk);
	};

	//
	// Lexer是一个词法分析器.
	//
	class Lexer
	{
	public:
		Lexer(const vector<TokenDef>&, const vector<wstring>&);
		~Lexer();
	private:
		vector<wregex> regexs_;			// 记录所有词法的正则表达式
		vector<int> marks_;
		set<int> ignore_;				// 记录需要被忽略掉的词法单元
		wstring stream_;
		size_t offset_;					// 用于记录已经解析到了的位置
		size_t line_;					// 记录文本所在的行,列也非常重要
		list<Token> cache_;
	public:
		void setStream(const wstring& stream) {
			offset_ = 0;
			line_ = 1;				// 从第1行开始
			stream_ = stream;
		}
		Token next();
		Token peek();
	private: 
		int countNL(const wstring&);
	};

	void checkTokens(const vector<TokenDef>& tokens);
}