#include "Rule.h"
#include "SymbolTable.h"

namespace tinyYACC {
	Rule::Rule(symbol start) :
		origin_(start)
	{
	}

	Rule::~Rule()
	{
	}

	bool Rule::operator==(const Rule & r)
	{
		if (this->origin_ == r.origin_) {
			if (this->expansion_.size() != r.expansion_.size()) {
				return false;
			}
			for (size_t i = 0; i < expansion_.size(); ++i) {
				if (!(expansion_[i] == r.expansion_[i]))
					return false;
			}
		}
		return true;
	}

	wostream& operator<<(wostream& os, const Rule& r) {
		os << g_symbolTable[r.origin_] << L" --> ";
		if (r.isEpsRule()) {
			os << L"";
		}
		else {
			for (auto elem : r.expansion_) {
				os << g_symbolTable[elem] << L" ";
			}
		}
		os << endl;
		return os;
	}


	//
	// Items
	//
	Item::Item(const rulePtr& r, int pos) :
		rule_(r), pos_(pos)
	{
	}


	Item::~Item()
	{
	}

	//
	// Item的operator<<函数纯粹是为了调试方便.
	//
	wostream& operator<<(wostream& os, const Item& it)
	{
		wcout << g_symbolTable[it.rule_->origin()] << L" --> ";
		size_t i = 0;
		size_t size = it.rule_->expansionLength();
		for (; i < size; ++i) {
			if (i == it.pos_)
				os << L"●";
			os << g_symbolTable[it.rule_->findNthElem(i)] << L" ";
		}
		if (i == it.pos_)
			os << L"●";
		return os;
	}
}
