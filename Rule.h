#pragma once
#include "Common.h"
#include "Symbol.h"
#include <functional>

/*
 * Rule类主要用于记录语法,或者说文法.
 */
class Rule
{
public:
	Rule(symbolPtr&);
	~Rule();
public:
	bool isEpsRule() {						/* 用于判断这条规则是否是ε规则 */
		return expansion_.size() == 0;
	}

	bool originEqualTo(symbolPtr& s) {		/* 判断rule的左边是否是s */
		return s == origin_;
	}

	symbolPtr findNthElem(size_t n) {		/* 寻找rule推导式右侧的第N个元素 */
		assert(n < expansion_.size());
		return expansion_[n];
	}
	
	vector<symbolPtr>& expansion() {
		return expansion_;
	}

	vector<symbolPtr> subExpansion(size_t from, size_t len) {
		auto start = expansion_.begin();
		return vector<symbolPtr>(start + from, start + from + len);
	}

	shared_ptr<set<symbolPtr>> subExpansionSymbols(size_t from, size_t len) {
		auto start = expansion_.begin();
		return make_shared<set<symbolPtr>>(start + from, start + from + len);
	}

	size_t expansionLength() {
		return expansion_.size();
	}

	symbolPtr origin() {		/* 获取规则左侧的非终结符 */
		return origin_;
	}

	Rule(const Rule& r);

	void expansionAppend(symbolPtr& s) {
		expansion_.push_back(s);
	}

public:
	friend wostream& operator<<(wostream& os, Rule& r);
	bool operator==(Rule& r);

private:
	symbolPtr origin_;					/* 左边部分 */
	vector<symbolPtr> expansion_;		/* 展开部分 */
};


/*
* Item 用于记录一些状态,说实话,Item已经非常类似于一个指针,它只是记录了rule的地址,然后附带了一个pos,所以复制的开销也并不大.
*/
class Item
{
public:
	Item(rulePtr&, int);
	~Item();

private:
	rulePtr rule_;		/* 用于记录推导规则 */
	int pos_;			/* 用于记录Item现在已经解析到了rule的right hand的哪一个符号上了. */

public:
	friend wostream& operator<<(wostream& os, Item& it);
	
	bool operator==(const Item& r) const {		/* 用于比较两个Item是否相等 */
		return (rule_ == r.rule_) && (pos_ == r.pos_);
	}

	int getPos() {
		return pos_;
	}

	/* 每个item都有一个hash值 
	 * 如果item相等的话,他们的hash值也相等.
	 */
	size_t hash() {
		size_t h1 = std::hash<shared_ptr<Rule>>{}(rule_);
		size_t h2 = std::hash<int>{}(pos_);
		return h1 ^ (h2 << 1);
	}

	rulePtr getRule() {
		return rule_;
	}

	/*
	 * isSatisfied 用于判断pos是否到了rule的最后一个位置,也就是判断能否规约了.
	 */
	bool isSatisfied() {
		return pos_ == rule_->expansionLength();
	}

	/*
	 * next获取rule_的Expansion中第pos_个符号,存入s中,成功返回true,否则返回false
	 */
	bool next(symbolPtr& s) {
		if (!isSatisfied()) {
			s = rule_->findNthElem(pos_);
			return true;
		}
		else
			return false;
	}

	/*
	 * advance将pos_前进一步,无法前进时,返回false,否则返回true
	 */
	bool advance() {
		if (!isSatisfied()) {
			pos_++;
			return true;
		}
		else
			return false;
	}

	bool operator<(const Item& r) const {	 /* 这个东西可能用不到,但是还是放在这里吧. */
		if (rule_ != r.rule_)
			return rule_ < r.rule_;
		else 
			return pos_ < r.pos_;
	}

	Item(const Item& r) {		/* 拷贝构造函数 */
		this->rule_ = r.rule_;
		this->pos_ = r.pos_;
	}

};
typedef shared_ptr<Item> itemPtr;

