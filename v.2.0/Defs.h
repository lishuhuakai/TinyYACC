#pragma once
#include "Common.h"

namespace tinyYACC {
	//
	// TokenDef是关于Token的定义.
	//
	struct TokenDef
	{
		TokenDef(const wstring& name, const wstring& pattern) :
			name(name), pattern(pattern)
		{}
		TokenDef() {}
		friend wostream& operator<<(wostream& os, const TokenDef& tk) {
			os << tk.name << L"	:" << tk.pattern;
			return os;
		}
		wstring name;		// 记录这个Token的名称
		wstring pattern;	// 记录对应的正则表达式
	};

	//
	// RuleDef是关于规则的定义.
	//
	struct RuleDef {
		wstring origin;				// 推导式左侧的非终结符
		vector<wstring> expansions; // 推导式右侧的终结符或非终结符
		friend wostream& operator<<(wostream& os, const RuleDef& r) {
			os << r.origin << L"	:";
			for (auto exp : r.expansions) {
				os << L" " << exp;
			}
			return os;
		}
	};
}


