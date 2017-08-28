#pragma once
#include "Common.h"
#include <functional>

namespace tinyYACC {
	//
	// Rule类主要用于记录语法,或者说文法.
	//
	class Rule
	{
	public:
		Rule(symbol start);
		~Rule();
	public:
		bool isEpsRule() const {				 // 用于判断这条规则是否是ε规则
			return expansion_.size() == 0;
		}

		bool originEqualTo(symbol s) const {		// 判断rule的左边是否是s
			return s == origin_;
		}

		int findNthElem(size_t n) const {		// 寻找rule推导式右侧的第N个元素
			assert(n < expansion_.size());
			return expansion_[n];
		}

		const vector<int>& expansion() const {
			return expansion_;
		}

		vector<symbol> subExpansion(size_t from, size_t len) const {
			auto start = expansion_.begin();
			return vector<symbol>(start + from, start + from + len);
		}

		shared_ptr<set<symbol>> subExpansionSymbols(size_t from, size_t len) const {
			auto start = expansion_.begin();
			return make_shared<set<symbol>>(start + from, start + from + len);
		}

		size_t expansionLength() const {  // 获取规则右侧式子的长度
			return expansion_.size();
		}

		int origin() const {		// 获取规则左侧的非终结符
			return origin_;
		}


		void expansionAppend(int s) {
			expansion_.push_back(s);
		}

	public:
		bool operator==(const Rule& r);
		friend wostream& operator<<(wostream& os, const Rule& r);
	private:
		int origin_;					// 左边部分
		vector<int> expansion_;			// 展开部分
	};


	//
	// Item 用于记录一些状态,说实话,Item已经非常类似于一个指针,它只是记录了rule的地址,然后附带了一个pos,所以复制的开销也并不大.
	//
	class Item
	{
	public:
		Item(const rulePtr&, int);
		~Item();

	private:
		rulePtr rule_;		// 用于记录推导规则
		int pos_;			// 用于记录Item现在已经解析到了rule的expansions的哪一个符号上了

	public:
		friend wostream& operator<<(wostream& os, const Item &it);

		bool operator==(const Item& r) const {		// 用于比较两个Item是否相等
			return (rule_ == r.rule_) && (pos_ == r.pos_);
		}

		int getPos() const {
			return pos_;
		}

		//
		// 每个item都有一个hash值,如果item相等的话,他们的hash值也相等.
		//
		size_t hash() const {
			size_t h1 = std::hash<shared_ptr<Rule>>{}(rule_);
			size_t h2 = std::hash<int>{}(pos_);
			return h1 ^ (h2 << 1);
		}

		rulePtr getRule() {
			return rule_;
		}

		//
		// isSatisfied 用于判断pos是否到了rule的最后一个位置,也就是判断能否规约了.
		//
		bool isSatisfied() const {
			return pos_ == rule_->expansionLength();
		}

		//
		// next获取rule_的Expansion中圆点后面的符号,存入s中,成功返回true,否则返回false
		// 举个例子: E -> ab●C ,将C对应的标记记入s中.
		//
		bool next(int& s) const {
			if (!isSatisfied()) {
				s = rule_->findNthElem(pos_);
				return true;
			}
			else
				return false;
		}

		//
		// advance将pos_前进一步,无法前进时,返回false,否则返回true
		//
		bool advance() {
			if (!isSatisfied()) {
				pos_++;
				return true;
			}
			else
				return false;
		}

		bool operator<(const Item& r) const {	 // 这个东西可能用不到,但是还是放在这里吧
			if (rule_ != r.rule_)
				return rule_ < r.rule_;
			else
				return pos_ < r.pos_;
		}

		Item(const Item& r) {
			this->rule_ = r.rule_;
			this->pos_ = r.pos_;
		}

	};
	typedef shared_ptr<Item> itemPtr;
}

