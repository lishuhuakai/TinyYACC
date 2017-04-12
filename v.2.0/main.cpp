#include "Common.h"
#include "Grammar.h"
#include "Rule.h"
#include "GrammarLoader.h"
#include "Lexer.h"
#include "LALRParserTable.h"
#include "GrammarBuilder.h"
#include <regex>
using namespace std;
using namespace tinyYACC;


int main() {
	locale loc("chs");
	wcout.imbue(loc);
	GrammarLoader loader;
	// nd是根据文法1.df生成的ast.如果生成成功,nd不为空,否则会打印出错误原因,nd为空.
	grammarNodePtr nd = parseGrammar(loader, L"1.df");
	if (nd) {
		// printTree将生成的ast保存成graphviz脚本,可以用graphviz查看.
		printTree(*nd);
		// collectDefsAndules从nd中搜集Rule,token的定义,利用这些信息,我们可以生成这个文法的Parsing Table.
		// 至于Parser,估计要自己写,不在本程序的功能范围之内.
		collectDefsAndRules(*nd);
	}
	getchar();
	return 0;
}