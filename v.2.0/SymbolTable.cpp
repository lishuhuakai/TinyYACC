#include "SymbolTable.h"

namespace tinyYACC {
	SymbolTable::SymbolTable()
	{
		// 添加开始符号和结束符号
		table_[L"#start"] = -2;
		table_[L"#end"] = -1;
		reverseTable_[-2] = { L"#start", false };
		reverseTable_[-1] = { L"#end", true };  // #end是终结符
	}


	SymbolTable::~SymbolTable()
	{
	}

	//
	// appendType用于添加新类型,如果类型已经存在,返回该类型的标记,否则添加该类型.
	//
	int SymbolTable::appendType(const wstring & tp, bool terminal)
	{
		if (table_.find(tp) != table_.end()) {
			return table_[tp];
		}
		int label = assignLabel();
		table_[tp] = label;
		reverseTable_[label] = { tp, terminal};
		return label;
	}

	wstring SymbolTable::queryType(int mark)
	{
		assert(reverseTable_.find(mark) != reverseTable_.end());
		return reverseTable_[mark].name;
	}

	int SymbolTable::queryMark(const wstring & tp)
	{
		assert(table_.find(tp) != table_.end());
		return table_[tp];
	}

	int SymbolTable::assignLabel()
	{
		static int count = 0;
		return ++count;
	}
}