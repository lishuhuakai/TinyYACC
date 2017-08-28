#include "LALRParser.h"
#include "Grammar.h"
#include "Rule.h"

namespace tinyYACC {
	LALRParser::LALRParser(Grammar &g) :
		g_(g)
	{
	}

	LALRParser::~LALRParser()
	{
	}

	//
	// computerLookAhead 用于计算前看符号,同时构建状态.
	//
	void LALRParser::computerLookAhead()
	{
		// 首先从文法的start符号出发,构建一个stat
		g_.calculateSets(); // 计算所有符号的first集, folllow集,以及nullable集
		list<Item> its;
		bool exists = false;				// 用于记录状态是否早就存在了
		queue<size_t> notHandled;			// 用于存储那些尚未处理过的状态的标志

		its.push_back(Item(g_.startRule(), 0));
		start_ = expandRule(its, exists);	// 得到第一个状态
		notHandled.push(start_);

		while (!notHandled.empty()) {
			// 取出一个尚未被处理过的stat
			size_t stat = notHandled.front(); notHandled.pop();
			shared_ptr<map<symbol, list<Action>>> lookahead = make_shared<map<symbol, list<Action>>>();
			list<Item> satisfied; // satisfied用于记录stat中可以规约的item

			// 对状态里的Item进行分类,包括可以规约的项,以及通过某个Item中的next对应的symbol都相同的项
			// 根据每个状态的标记,从status_总取出状态(status_[stat])
			auto group = classify(status_[stat], satisfied);

			for (auto it : satisfied) {
				// 可以使用这个来规约
				for (auto sym : g_.follow(it.getRule()->origin())) {
					appendAction(*lookahead, sym, makeAction(Action::reduce, it.getRule()));
				}
			}
			// 接下来对于各种符号进行遍历
			for (auto &pr : *group) {
				int sym = pr.first;
				list<Item>& its = pr.second;

				for (auto &it : its) {
					// 这里可以保证不会越界
					//assert(it.advance() != false);
					it.advance();
				}
				size_t stat = expandRule(its, exists); // 继续扩展状态

				// 从前不存在过,也就是新加入的节点,需要处理
				if (!exists) notHandled.push(stat);

				// appendAction返回action插入的那个链表的长度,如果长度不为1,表示存在两个动作要执行
				// 这显然发生了错误
				if (appendAction(*lookahead, sym, makeAction(Action::shift, stat)) != 1) {
					GeneralError error;
					error.msg = L"移进规约冲突 : " + g_symbolTable[sym] + L"!";
					throw error;
				}
			}

			// 如果不存在问题的话,将表项填写到table中
			table_[stat] = map<int, Action>();
			for (auto &pr : *lookahead) {
				int sym = pr.first;
				table_[stat][sym] = *pr.second.begin();
			}
		}
		// 构造出Parsing table之后,status_没有了任何用处
		status_.clear();
	}

	//
	// expandRule 用于扩展状态,并传回一个状态的标记,如果这个状态之前已经存在过了,exists
	// 设定为true,否则设定为false
	// 
	size_t LALRParser::expandRule(list<Item>& items, bool& exists)
	{
		set<int> visited;				// 用于记录已经访问过的节点
		statusPtr stat = make_shared<Status>();
		int sym;

		for (auto &it : items) {
			stat->insertItem(it);
		}

		// 这里之所以选择list作为容器,因为即使在list的后面添加元素,它的迭代器也不会失效
		for (auto &it : items) {
			if (it.next(sym) && !g_symbolTable.isTerminal(sym)) { // 开始扩展
				if (visited.find(sym) == visited.end()) {
					visited.insert(sym);
					auto rules = g_.findRules(sym);
					for (auto &r : *rules) {
						auto newItem = Item(r, 0);
						stat->insertItem(newItem);
						items.push_back(newItem);
					}
				}
			}
		}
		// tofix: 虽然概率很小很小,但是hash还是有可能会冲突
		// 有没有更好的办法来区分重复的状态呢?
		size_t h = StatusHash{}(*stat);		// 计算hash值
		if (status_.find(h) == status_.end()) { // 记录下新状态
			status_[h] = stat;
			exists = false;
		}
		else exists = true;
		//wcout << *stat << endl;
		// 我们只需要返回一个唯一的hash值即可,它代表了这个stat
		return h;
	}

	//
	// classify 用于将status中的一些可以规约的item放入lst中,对于不可以规约的项
	// 将item中下一个要遇到的字符作为key,将key相同的item们放入一个list中,作为value
	// 构建成一个字典返回回来
	// 
	shared_ptr<map<int, list<Item>>> LALRParser::classify(statusPtr& s, list<Item>& lst)
	{
		shared_ptr<map<symbol, list<Item>>> res = make_shared<map<symbol, list<Item>>>();
		for (auto &it : *s->items) {
			// 可规约项不管
			int next;
			if (it.next(next)) {
				if (res->find(next) != res->end()) {
					(*res)[next].push_back(it); 
				}
				else {
					(*res)[next] = list<Item>();
					(*res)[next].push_back(it);
				}
			}
			else { // it这个item可以规约了(只有出现了类似于E -> abC●这种情况才可能返回false)
				lst.push_back(it);
			}
		}
		return res;
	}

	//
	// printStats 用于输出构建后的Status_表.
	//
	void LALRParser::printStats()
	{
		for (auto &pr : status_) {
			wcout << L"lable :" << pr.first << endl;
			wcout << (*pr.second) << endl;
		}
	}

	LALRParser::Action LALRParser::makeAction(Action::action act, size_t target)
	{
		assert(act == Action::shift);
		return Action(Action::shift, target);
	}

	// 
	// makeAction 使用规则r进行规约
	// 
	LALRParser::Action LALRParser::makeAction(Action::action act, rulePtr& r)
	{
		assert(act == Action::reduce);
		reducePool_.push_back(r);
		return Action(Action::reduce, reducePool_.size() - 1);
	}

	//
	// appendAction 将符号应该执行的操作添加到list中,最终返回list的长度.
	// 
	size_t LALRParser::appendAction(map<int, list<Action>>& lookahead, int sym, Action &act)
	{
		if (lookahead.find(sym) != lookahead.end()) {
			lookahead[sym].push_back(act);
			return lookahead[sym].size(); 
		}
		lookahead[sym] = list<Action>();
		lookahead[sym].push_back(act);
		return 1;
	}
}