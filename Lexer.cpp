#include "Lexer.h"
#include "TokenDef.h"

Lexer::Lexer(vector<TokenDef>& tokens, vector<wstring>& ignoreTokens) {
	// 这里非常有可能抛出异常, 但是在构造函数里抛出异常是不被允许的,所以在进行Lexer构造之前,请确保传入的tokens是无误的.
	for (auto it = tokens.begin(); it != tokens.end(); ++it) {
		regexs_.push_back(wregex(it->pattern));
		names_.push_back(it->name);
	}
	ignore_.insert(ignoreTokens.begin(), ignoreTokens.end());
}

Lexer::~Lexer()
{
}

/*
 * lex开始对解析文本.
 */
Token Lexer::nextToken()
{
	if (cache_.size() != 0) {
		auto tk = cache_.front();
		cache_.pop_front();
		return tk;
	}

	if (offset_ == stream_.length()) // 所有字符均已经被消耗掉了
		return Token(L"#", line_, column_, L"");

	std::wsmatch mc;
	bool wrong = true;
	/* 使用各种正则表达式去匹配 */
	int	line = line_;
	int	column = column_;
	size_t i = 0;
	for (; i < regexs_.size(); ++i) {
		bool finded = std::regex_search(stream_.cbegin() + offset_, stream_.cend(), mc, regexs_[i]);
		if (finded && mc.position() == 0 && mc.length() != 0) {
			wrong = false;
			offset_ += mc.length();
			column_ += mc.length();
			bool over = offset_ == stream_.length();
			wstring type = names_[i];
			if (type == L"NL") { // new line
				line_ += 1;
				column_ = 1;
			}
			if (ignore_.find(type) != ignore_.end()) { // should be ignored!
				if (!over)
					return nextToken();
				else
					return Token(L"end", line, column, L"");
			}
			break;
		}
	}
	if (wrong) { /* 这么多正则表达式,居然没有一个匹配,也就是所,出现了无法匹配的字符. */
		throw GrammarError();
	}
	return Token(names_[i], line, column, mc.str());
}

/*
 * peekToken 并不消耗这个token,只是偷看一下
 */
Token Lexer::peekToken() {
	if (cache_.size() == 0) {
		cache_.push_back(nextToken());
	}
	 return cache_.front();
}

/*
 * checkTokens用于检查token的定义是否有问题,
 */
void checkTokens(vector<TokenDef>& tokens) {
	set<wstring> count;
	try
	{
		for (auto it = tokens.begin(); it != tokens.end(); ++it) {
			if (count.find(it->name) != count.end()) {
				/* 一个Token出现了多重定义 */
				throw GrammarError();
			}
			else
				count.insert(it->name);
			wregex(it->pattern);
		}
	}
	catch (regex_error& e)
	{
		e.what();
		throw GrammarError();
	}
}

