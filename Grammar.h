#pragma once
#include "Common.h"
#include "Rule.h"
#include "Symbol.h"

class Grammar
{
	typedef set<symbolPtr> symbolSet;
	typedef shared_ptr<symbolSet> symbolSetPtr;
public:
	class iterator
	{
	public:
		friend class Grammar;
		iterator() {}

		/* 迭代器之间进行比较 */
		bool operator==(const iterator& rhs) const {
			return it_ == rhs.it_;
		}
		bool operator!=(const iterator& rhs) const {
			return it_ != rhs.it_;
		}
		rulePtr& operator*() {
			/* 这里有必要说明一下,it_是迭代器类型, *it_是rulePtr类型, **it_是Rule类型 */
			return *it_;
		}
		iterator& operator++ () {
			it_++;
			return *this;
		}
		iterator& operator--() {
			--it_;
			return *this;
		}
	private:
		vector<rulePtr>::iterator it_;
		iterator(vector<rulePtr>::iterator& it) :
			it_(it)
		{}
	};
public:
	Grammar(symbolPtr&);
	~Grammar();
private:
	Grammar() {}
public:
	symbolPtr start_;								/* 开始符号 */
private:
	set<symbolPtr> symbols_;							/* 用于记录所有的符号,包括终结符和非终结符 */
	map<symbolPtr, symbolSetPtr> follow_;				/* 用于记录所有的follow集合 */
	map<symbolPtr, symbolSetPtr> first_;				/* 用于记录所有的first集合 */
	set<symbolPtr> nullable_;							/* 用于记录非终结符是否能够产生空串 */
	vector<rulePtr> rules_;								/* 用于记录规则 */
	map<wstring, symbolPtr> strMapping_;				/* 记录字符串到symbol的映射 */
public:
	shared_ptr<vector<rulePtr>> findRules(symbolPtr& l);	/* 通过l,寻找以l为开头的rule */
	friend wostream& operator<< (wostream& os, Grammar& g);
	void appendRule(rulePtr& r) {
		rules_.push_back(r);
	}
	iterator begin() {
		return iterator(rules_.begin());
	}
	iterator end() {
		return iterator(rules_.end());
	}

	vector<rulePtr> getAllRules() {  /* 虽然我也不想拷贝一份,但是没有办法,我不能将内部的数据暴露出去. */
		return rules_;
	}
	/* startRule返回增广文法的起始规则 */
	rulePtr startRule() {
		return (*findRules(start_))[0];
	}
	void calculateSets();
	void printSets();
	set<symbolPtr> follow(symbolPtr& s) {
		return *follow_[s];
	}
	set<symbolPtr> first(symbolPtr& s) {
		return *first_[s];
	}
	symbolPtr strToSymbol(wstring& s) {
		return strMapping_[s];
	}
private:
	void getAllSymbols();
	static bool updateSet(set<symbolPtr>& t, set<symbolPtr>& s);
	static bool updateSet(set<symbolPtr>& t, symbolPtr& s);
};

