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

namespace tinyYACC {
	class Rule;
	class Grammar;
	class SymbolTable;
	typedef int symbol;
	typedef shared_ptr<Rule> rulePtr;
	typedef shared_ptr<Grammar> grammarPtr;

	extern SymbolTable g_symbolTable;
	//
	// isSubSet 用于判断l是否是r的子集,算法复杂度O(N).
	//
	template<typename T>
	bool isSubSet(const set<T>& l,const set<T>& r) {
		if (r.size() < l.size()) return false;
		auto e1 = r.begin(), e2 = l.begin();
		for (; e2 != l.end() && e1 != r.end(); ) {
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

	//
	// GrammarError用于记录文法的错误. 
	//
	struct GrammarError {
	public:
		size_t line;
		wstring msg;
	public:
		wstring what() const {
			wchar_t message[256];
			swprintf_s(message, 256, L"In line %d : %s\n", line, msg.c_str());
			return message;
		}
	};

	//
	// GeneralError是一个非常简易的错误类,只用于记录出错的信息,对于我们这个简易的YACC来说,足够了.
	//
	struct GeneralError {
	public:
		wstring msg;
	};

}
