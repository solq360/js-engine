#ifndef JsEvalH
#define JsEvalH

#include"JsType.h"

struct JsEngine;
struct JsAst;
struct JsValue;
/****************************************************************************
									运行时API
*****************************************************************************/
/*
	e->exec 为执行的状态
	注意修改Engine的状态
*/
void JsEval(struct JsEngine* e,struct JsAst* ast,
			struct JsValue** res);
#endif