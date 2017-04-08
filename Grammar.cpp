#include "Grammar.h"
#include "Rule.h"
#include "Symbol.h"


Grammar::Grammar(symbolPtr& s) :
	start_(s)
{
}


Grammar::~Grammar()
{
}

/*
 * findRules 用于寻找非终结符l的推导规则.
 */
shared_ptr<vector<rulePtr>> Grammar::findRules(symbolPtr & l)
{
	assert(l->type_ == Symbol::NonTerminal);
	shared_ptr<vector<rulePtr>> result = make_shared<vector<rulePtr>>();
	for (auto rule : rules_) {
		if (rule->originEqualTo(l)) {
			result->push_back(rule);
		}
	}
	return result;
}

/*
 * getAllSymbols 用于获取rule中所有的符号.
 */
void Grammar::getAllSymbols() {
	for (auto r : rules_) {
		auto origin = r->origin();
		symbols_.insert(origin);
		strMapping_[origin->content_] = origin;
		for (size_t i = 0; i < r->expansionLength(); ++i) {
			auto elem = r->findNthElem(i);
			symbols_.insert(elem);
			strMapping_[elem->content_] = elem;
		}
	}
}

/*
 * updateSet 利用s来更新t,如果t改变了,那么返回true,否则返回false
 */
bool Grammar::updateSet(set<symbolPtr>& t, set<symbolPtr>& s) {
	size_t len = t.size();
	t.insert(s.begin(), s.end());
	return t.size() != len;
}

bool Grammar::updateSet(set<symbolPtr>& t, symbolPtr& s) {
	size_t len = t.size();
	t.insert(s);
	return t.size() != len;
}


/*
 * calculateSets 用于计算FIRST集和FOLLOW集
 */
void Grammar::calculateSets()
{
	/* 第一步,获得所有的符号 */
	getAllSymbols();
	/* 接下来做初始化的工作 */
	for (auto sym : symbols_) {
		first_[sym] = make_shared<symbolSet>();
		follow_[sym] = make_shared<symbolSet>();
		if (sym->isTerminal())
			first_[sym]->insert(sym);
	}
	bool changed = true;
	while (changed) {
		changed = false;
		for (auto r : rules_) {
			/* 如果可以r的expansion包含的符号全部可以推导出epsilon,那么r的orgin也可以推导出epsilon */
			if (isSubSet(set<symbolPtr>(r->expansion().begin(), r->expansion().end()), nullable_)) {
				if (updateSet(nullable_, r->origin())) changed = true;
			}

			size_t len = r->expansionLength();
			for (size_t i = 0; i < len; ++i) {
				auto subExpansion = r->subExpansionSymbols(0, i);
				auto sym = r->findNthElem(i);
				auto origin = r->origin();
				/* 如果r的expansion从0到i(不包括i)的元素都可以推导出epsilon, 那么first(r.origin)包括first(r.findNthElem(i)) */
				if (isSubSet(*subExpansion, nullable_)) {
					if (updateSet(*first_[origin], *first_[sym]))
						changed = true;
				}
				 /* 第i个元素不是最后一个元素,并且expansion从第i+1个元素之后的元素全都nullable 
				  * 那么follow[sym]包括follow[r.origin].
				  * 如果第i个元素是最后一个元素,同理有follow[sym]包括follow[sym].
				  */
				if ((i == (len - 1)) || isSubSet(*r->subExpansionSymbols(i + 1, len - i - 1), nullable_)) { 
						if (updateSet(*follow_[sym], *follow_[origin])) changed = true;
				}

				/* 如果从第i+1到第j(不包括j)位置的元素都是nullable的,那么Follow[sym]包括first[r->findNthElem(j)] */
				for (size_t j = i + 1; j < len; j++) {
					/* 第j个元素必须存在于expansion */
					size_t len = j - i - 1;
					subExpansion = r->subExpansionSymbols(i + 1, len);
					if (isSubSet(*subExpansion, nullable_)) {
						if (updateSet(*follow_[sym], *first_[r->findNthElem(j)])) changed = true;
					}
				}
			}
		}
	}
	//wcout << L"================================================" << endl;
	//printSets();
}

wostream & operator<<(wostream & os, Grammar & g)
{
	for (auto rule : g.rules_) {
		os << *rule;
	}
	return os;
}

void Grammar::printSets() {
	wcout << L">>>>>>>>>>first<<<<<<<<<<" << endl;
	for (auto pr : first_) {
		wcout << *(pr.first) << L":" << endl;
		for (auto sym : *pr.second) {
			wcout << *sym << L" ";
		}
		wcout << endl;
	}

	wcout << L">>>>>>>>>>follow<<<<<<<<<<" << endl;
	for (auto pr : follow_) {
		wcout << *(pr.first) << L":" << endl;
		for (auto sym : *pr.second) {
			wcout << *sym << L" ";
		}
		wcout << endl;
	}

	wcout << L">>>>>>>>>>nullable<<<<<<<<<<" << endl;
	for (auto sym : nullable_) {
		wcout << *sym << L" ";
	}
	wcout << endl;
}

/**********************************************************
 构建文法相关的部分.
**********************************************************/

