#ifndef JsErrorH
#define JsErrorH

#include"JsType.h"
/**

	使用抛出异常API的Value基本上不为JS_COMPLETION.THROW类型,
	而是具体某个类型如:
		JS_STRING,
		JS_OBJECT,
		...
*/
struct JsValue;
/*
	*因为命名的原因, 一个函数只能使用一个TRY
	
	基本使用方法
	#include<setjmp.h>
	
	JS_TRY{
		可以放置表达式(函数调用, 赋值,...)
		如果使用return , break 语句则需要在之前调用JsOmitPoint()
		
	}
	doFinally工作
	JS_CATCH{
		JsValue* e = JsGetError();
		1. 处理错误, 最后执行	
		2. 继续抛出异常
			JsThrow(e);
	
	}
*/
#define JS_TRY \
	int done_1234567890_abcdefg; \
	jmp_buf* jmp_buf_1234567890_abcdefg = (jmp_buf*)JsMalloc(sizeof(jmp_buf)); \
	for(done_1234567890_abcdefg = 0;  \
		done_1234567890_abcdefg == 0 \
		&& (setjmp(*jmp_buf_1234567890_abcdefg) == 0 \
			?(JsBuildPoint(jmp_buf_1234567890_abcdefg),1) \
			: (JsOmitPoint(),0));  \
		++done_1234567890_abcdefg, JsOmitPoint()) 
		
#define JS_CATCH \
		if(JsGetError())
/****************************************************************************
									通用API
*****************************************************************************/
/*
	抛出一个String类型的错误
*/
void JsThrowString(char* msg);
/*
	抛出一个error
*/
void JsThrow(struct JsValue* e);
//保存一个还原点到环境中
void JsBuildPoint(void* p);
//在环境中删除一个最近的还原点
void JsOmitPoint();
//获得当前错误, 并且清空, 如果没有则返回NULL
struct JsValue* JsGetError();
//设置一个错误, NULL表示清除错误
void JsSetError(struct JsValue* v);
#endif