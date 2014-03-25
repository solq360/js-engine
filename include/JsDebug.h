#ifndef JsDebugH
#define JsDebugH

#include"JsType.h"

struct JsEngine;
struct JsFunction;
//事件
enum JsTraceEvent {
	JS_TRACE_CALL,
	JS_TRACE_RETURN,
	JS_TRACE_STATEMENT,
	JS_TRACE_THROW
};
struct JsLocation{
	char *filename;		/* source location */
	int lineno;
};

/****************************************************************************
									通用API
*****************************************************************************/
//Print函数组
void JsPrintString(char* fmt,...);
void JsPrintValue(struct JsValue* v);
//struct JsLocation .
void JsPrintStack(JsList stack);

//Assert
void JsAssert(int v);
#endif