#ifndef JsEvalH
#define JsEvalH

#include"JsType.h"

struct JsEngine;
struct JsAstNode;
struct JsValue;
/****************************************************************************
									运行时API
*****************************************************************************/
/*
	e->exec 为执行的状态
	注意修改Engine的状态, 和 位置追踪 context->stack + context->pc
	如果不想知道res, 则使用NULL
*/
void JsEval(struct JsEngine* e,struct JsAstNode* ast, struct JsValue* res);
#endif