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


struct JsStack{
	struct JsLocation* loc;
	struct JsFunction* function; //NULL为Global
};

/****************************************************************************
									通用API
*****************************************************************************/
void JsPrintf(char* fmt,...);
void JsAssert(int v);
#endif