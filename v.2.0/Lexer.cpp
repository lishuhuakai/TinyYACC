#include "Lexer.h"
#include "Defs.h"
#include "SymbolTable.h"

namespace tinyYACC {

	//
	// printToken 用于输出Token的值.
	//
	wostream& operator<<(wostream& os, const Token& tk)
	{
		os << tk.content << L"[" << g_symbolTable[tk.mark] << L"]" << L"(" << tk.line << L")" << endl;
		return os;
	}

	Lexer::Lexer(const vector<TokenDef>& tokens, const vector<wstring>& ignoreTokens)
	{
		// 这里非常有可能抛出异常, 但是在构造函数里抛出异常通常是不被允许的,
		// 所以在进行Lexer构造之前,请确保传入的tokens是无误的.
		for (auto it = tokens.begin(); it != tokens.end(); ++it) {
			regexs_.push_back(wregex(it->pattern));
			marks_.push_back(g_symbolTable[it->name]);  // 将词法的name转化为int
		}
		for (auto sym : ignoreTokens)
		ignore_.insert(g_symbolTable[sym]);
	}

	Lexer::~Lexer()
	{
	}

	//
	// next函数用于获取下一个元素,并且消耗掉这个元素.
	//
	Token Lexer::next()
	{
		if (cache_.size() != 0) {
			auto tk = cache_.front();
			cache_.pop_front();
			return tk;
		}

		if (offset_ == stream_.length()) // 所有字符均已经被消耗掉了
			return Token(g_symbolTable.endMark, line_, L"");

		std::wsmatch mc;
		bool wrong = true;
		// 使用各种正则表达式去匹配
		size_t	line = line_;
		size_t i = 0;
		for (; i < regexs_.size(); ++i) {
			bool finded = std::regex_search(stream_.cbegin() + offset_, stream_.cend(), mc, regexs_[i]);
			if (finded && mc.position() == 0 && mc.length() != 0) {
				wrong = false;
				offset_ += mc.length();
				bool over = offset_ == stream_.length();
				int type = marks_[i];
				wstring matchStr = mc.str();
				if (matchStr.find(L'\n') != matchStr.npos) { // new line
					line_ += countNL(matchStr); 
				}
				if (ignore_.find(type) != ignore_.end()) { // should be ignored!
					if (!over)
						return next();
					else
						return Token(g_symbolTable.endMark, line,  L"");
				}
				break;
			}
		}
		if (wrong) { // 这么多正则表达式,居然没有一个匹配,也就是所,出现了无法匹配的字符.
			GrammarError error = { line, L"无法识别的字符" };
			throw error;
		}
		return Token(marks_[i], line, mc.str());
	}

	int Lexer::countNL(const wstring& str) {
		int res = 0;
		for (auto s : str) {
			if (L'\n' == s)
				res++;
		}
		return res;
	}
	//
	// peekToken 并不消耗这个token,只是偷看一下.
	//
	Token Lexer::peek() {
		if (cache_.size() == 0) {
			cache_.push_back(next());
		}
		return cache_.front();
	}


	//
	// checkTokens用于检查token的定义是否有问题.
	//
	void checkTokens(const vector<TokenDef>& tokens) {
		set<wstring> count;
		for (auto it = tokens.begin(); it != tokens.end(); ++it) {
			if (count.find(it->name) != count.end()) {
				// 一个Token出现了多重定义
				GeneralError error;
				error.msg = it->name + L"出现了多重定义!";
				throw error;
			}
			else
				count.insert(it->name);
			try
			{
				wregex(it->pattern);
			}
			catch (...) {
				GeneralError error;
				error.msg = it->name + L"的定义\"" + it->pattern + L"\"出现了错误!";
				throw error;
			}
		}
	}
}
