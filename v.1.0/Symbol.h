#pragma once
#include "Common.h"

//
// Symbol类用来表示终结符和非终结符.
//
class Symbol
{
public:
	enum symbolType {
		Terminal, NonTerminal
	};
public:
	Symbol(symbolType,const wstring&);
	~Symbol();
public:
	bool isTerminal() {
		return type_ == Terminal;
	}
	bool isNonTerminal() {
		return type_ == NonTerminal;
	}
	bool isEps() {
		return mark_.size() == 0 && type_ == Terminal;
	}
	friend wostream& operator<<(wostream&, Symbol&);
	wstring uniqueMark() {
		return mark_;
	}
	bool operator==(Symbol& r) {
		return this->mark_ == r.mark_;
	}
public:
	symbolType type_;	// 用来区分符号的类型
	wstring mark_;		// 用于记录符号的标记
};


