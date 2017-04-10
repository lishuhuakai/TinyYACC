#pragma once
#include <iostream>
#include <algorithm>
#include <memory>
#include <vector>
#include <queue>
#include <assert.h>
#include <set>
#include <map>
#include <list>
#include <fstream>
#include <stack>
#include <regex>
using namespace std;

class Rule;
class Symbol;
class Grammar;
typedef shared_ptr<Symbol> symbolPtr;
typedef shared_ptr<Rule> rulePtr;
typedef shared_ptr<Grammar> grammarPtr;


//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
//   about set
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

/*
 * isSubSet 用于判断l是否是r的子集,算法复杂度O(N)
 */
template<typename T>
bool isSubSet(set<T>& l, set<T>& r) {
	if (r.size() < l.size()) return false;
	auto e1 = r.begin(), e2 = l.begin();
	for ( ; e2 != l.end() && e1 != r.end(); ) {
		if (*e1 == *e2) {
			e1++, e2++;
		}
		else
			e1++;
	}
	if (e2 == l.end())
		return true;
	return false;
}

/*
 * 文法的错误 
 */
class GrammarError {
private:
	size_t line_;   
	size_t pos_;
	wstring msg_;
public:
	GrammarError(size_t line, size_t pos,const wstring& msg) :
		line_(line), pos_(pos), msg_(msg)
	{}
	wstring what() {
		wchar_t msg[256];
		swprintf_s(msg, 256, L"In line %d pos %d : %s\n", line_, pos_, msg_.c_str());
		return msg;
	}
};

struct GeneralError {
public:
	wstring msg;
};
