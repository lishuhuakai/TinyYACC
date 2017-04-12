#pragma once
#include "Common.h"

namespace tinyYACC {
	//
	// TypeMapping 用于实现wstring类型的[类型名]到int类型的[类型标记]的映射.反之也同样成立.
	//
	class SymbolTable
	{
	public:
		const int startMark = -2;
		const int endMark = -1;
		SymbolTable();
		~SymbolTable();
		int appendType(const wstring& tp, bool terminal);
		wstring queryType(int mark);
		int queryMark(const wstring& tp);

		bool isTerminal(int mark) {
			assert(reverseTable_.find(mark) != reverseTable_.end());
			return reverseTable_[mark].terminal == true;
		}

		int operator[](const wstring& tp) {
			assert(table_.find(tp) != table_.end());
			return table_[tp];
		}

		wstring operator[](int mark) {
			assert(reverseTable_.find(mark) != reverseTable_.end());
			return reverseTable_[mark].name;
		}

		void clear() {
			table_.clear();
			reverseTable_.clear();
			table_[L"#start"] = -2;
			table_[L"#end"] = -1;
			reverseTable_[-2] = { L"#start", false };
			reverseTable_[-1] = { L"#end", true };
		}
	private:
		struct Symbol {
			wstring name;
			bool terminal;
		};
	private:
		static int assignLabel();
	private:
		map<wstring, int> table_;
		map<int, Symbol> reverseTable_;
	};
}

