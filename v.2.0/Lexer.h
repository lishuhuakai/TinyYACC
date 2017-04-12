#pragma once
#include "Common.h"
#include <regex>
#include "SymbolTable.h"

namespace tinyYACC {
	struct TokenDef;

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
		friend wostream& operator<<(wostream& os, Token& tk);
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
		vector<int> marks_;
		set<int> ignore_;
		wstring stream_;
		size_t offset_;					// 用于记录已经解析到了的位置
		size_t line_;					// 记录文本所在的行,列也非常重要
		list<Token> cache_;
	public:
		void setStream(wstring& stream) {
			offset_ = 0;
			line_ = 1;
			stream_ = stream;
		}
		Token next();
		Token peek();
	private: 
		int countNL(wstring&);
	};

	void checkTokens(vector<TokenDef>& tokens);
}