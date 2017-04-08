#pragma once
#include "Common.h"
/*
 * TokenDef是关于Token的定义.
 */
struct TokenDef
{
	TokenDef(const wstring& name, const wstring& pattern) :
		name(name), pattern(pattern)
	{}
	wstring name;		/* 记录这个Token的名称 */
	wstring pattern;	/* 记录对应的正则表达式 */
};


