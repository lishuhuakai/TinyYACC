#pragma once
#include "Common.h"
#include "Rule.h"

namespace tinyYACC {
	class Grammar
	{
		typedef set<symbol> symbolSet;
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
		Grammar(symbol);
		~Grammar();
	private:
		Grammar() {}
	public:
		int start_;										// 开始符号 
	private:
		set<symbol> symbols_;							// 用于记录所有的符号,包括终结符和非终结符
		map<symbol, symbolSet> follow_;					// 用于记录所有的follow集合
		map<symbol, symbolSet> first_;					// 用于记录所有的first集合
		set<symbol> nullable_;							// 用于记录非终结符是否能够产生空串
		vector<rulePtr> rules_;							// 用于记录规则
	public:
		shared_ptr<vector<rulePtr>> findRules(int l);	// 通过l,寻找以l为开头的rule
		friend wostream& operator<< (wostream& os, const Grammar& g);
		void appendRule(const rulePtr& r) {
			rules_.push_back(r);
		}
		iterator begin() {
			return iterator(rules_.begin());
		}
		iterator end() {
			return iterator(rules_.end());
		}

		const vector<rulePtr>& getAllRules() {
			return rules_;
		}

		void calculateSets();
		void printSets() const;

		shared_ptr<Rule> startRule() {
			return (*findRules(start_))[0];
		}

		const set<int>& follow(symbol s) {
			assert(follow_.find(s) != follow_.end());
			return follow_[s];
		}

		const set<int>& first(symbol s) {
			assert(first_.find(s) != first_.end());
			return first_[s];
		}

	private:
		void getAllSymbols();
		static bool updateSet(set<symbol>& t, const set<symbol>& s);
		static bool updateSet(set<symbol>& t, symbol s);
	};
}

