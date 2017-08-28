#pragma once
class Grammar;
#include "Status.h"
#include "Rule.h"
#include "SymbolTable.h"


namespace tinyYACC {
	//
	// LALRParser和之前的LR(1)有所不同,因为它有更加简洁高效的实现方法.
	//
	class LALRParser
	{
	public:
		LALRParser(Grammar&);
		~LALRParser();
	public:
		struct Action {
			enum action { shift, reduce };  // 一共两种动作,一种是移进 shift,一种是规约reduce
			action act;
			size_t index;					// 用于存储状态status
			
			Action(const action act, size_t index) :
				act(act), index(index)
			{}

			Action() {}

			Action(const Action& a) {
				if (&a == this) return;
				act = a.act;
				index = a.index;
			}
		};
	private:
		Grammar& g_;
		map<size_t, statusPtr> status_;
		// 使用vector来存储移进时使用的规则
		vector<rulePtr> reducePool_;
		map<size_t, map<int, Action>> table_; // size_t标记状态, int标记符号, Action标记动作
	public:
		size_t start_;		// 用于记录开始状态的下标
	public:
		void printStats();
		void computerLookAhead();
		Action queryAction(size_t state, int sym) {
			assert(table_.find(state) != table_.end());
			if (table_[state].find(sym) == table_[state].end()) {
				GeneralError error;
				error.msg = L"无法为" + g_symbolTable.queryType(sym) + L"查询到Action!";
				throw error;
			}
			return table_[state][sym];
		}

		rulePtr queryRule(size_t idx) {
			assert(idx < reducePool_.size());
			return reducePool_[idx];
		}

	private:
		Action makeAction(Action::action, size_t);
		Action makeAction(Action::action, rulePtr&);
		static size_t appendAction(map<int, list<Action>>&, int sym, Action&);
		size_t expandRule(list<Item>&, bool&);
		shared_ptr<map<int, list<Item>>> classify(statusPtr& s, list<Item>&);
	};
}

