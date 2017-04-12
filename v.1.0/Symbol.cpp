#include "Symbol.h"

Symbol::Symbol(symbolType t, const wstring& str) :
	type_(t), mark_(str)
{
}

Symbol::~Symbol()
{
}

wostream& operator<<(wostream& os, Symbol& s) {
	os << s.mark_.c_str();
	return os;
}
