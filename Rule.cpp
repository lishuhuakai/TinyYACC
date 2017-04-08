#include "Rule.h"

Rule::Rule(symbolPtr& l) :
	origin_(l)
{
}

Rule::~Rule()
{
}

/*
 * 拷贝构造函数
 */
Rule::Rule(const Rule & r)
{
	this->origin_ = r.origin_;
	this->expansion_ = r.expansion_;
}

bool Rule::operator==(Rule & r)
{
	if (*(this->origin_) == *(r.origin_)) {
		if (this->expansion_.size() != r.expansion_.size()) {
			return false;
		}
		for (size_t i = 0; i < expansion_.size(); ++i) {
			if (!(*(expansion_[i]) == *(r.expansion_[i])))
				return false;
		}
	}
	return true;
}

wostream & operator<<(wostream & os, Rule & r)
{
	os << *(r.origin_) << L" --> ";
	if (r.isEpsRule()) {
		os << L"eps";
	}
	else {
		for (auto elem : r.expansion_) {
			os << *elem << L" ";
		}
	}
	os << endl;
	return os;
}

/*********************************************************************************
Items
**********************************************************************************/

Item::Item(rulePtr& r, int pos) :
	rule_(r), pos_(pos)
{
}


Item::~Item()
{
}

/*
* Item的operator<<函数纯粹是为了调试方便.
*/
wostream & operator<<(wostream & os, Item & it)
{
	os << *(it.rule_->origin()) << L" --> ";
	size_t i = 0;
	size_t size = it.rule_->expansionLength();
	for (; i < size; ++i) {
		if (i == it.pos_)
			os << L"●";
		os << *(it.rule_->findNthElem(i)) << L" ";
	}
	if (i == it.pos_)
		os << L"●";
	return os;
}

