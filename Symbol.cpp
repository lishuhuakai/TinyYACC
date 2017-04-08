#include "Symbol.h"

Symbol::Symbol(symbolType t, const wstring& str) :
	type_(t), content_(str)
{
}

Symbol::~Symbol()
{
}

wostream& operator<<(wostream& os, Symbol& s) { /* Êä³ö·ûºÅ */
	os << s.content_.c_str();
	return os;
}
