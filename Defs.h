#pragma once
#include "Common.h"

//
// TokenDef是关于Token的定义.
//
struct TokenDef
{
	TokenDef(const wstring& name, const wstring& pattern) :
		mark(name), pattern(pattern)
	{}
	TokenDef() {}
	friend wostream& operator<<(wostream& os, TokenDef& tk) {
		os << tk.mark << L"	:" << tk.pattern;
		return os;
	}
	wstring mark;		// 记录这个Token的名称
	wstring pattern;	// 记录对应的正则表达式
};

//
// RuleDef是关于规则的定义.
//
struct RuleDef {
	wstring origin;
	vector<wstring> expansions;
	friend wostream& operator<<(wostream& os, RuleDef& r) {
		os << r.origin << L"	:";
		for (auto exp : r.expansions) {
			os << L" " << exp;
		}
		return os;
	}
};


