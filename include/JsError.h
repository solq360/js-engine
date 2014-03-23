#ifndef JsErrorH
#define JsErrorH

#include"JsType.h"
/**
	因为该异常机制基于栈结构, 不能多线程处理, 每个线程有且只有一个记录组.
	且每条线程都具有一个根异常处理点.
	使用抛出异常API的Value基本上不为JS_COMPLETION.THROW类型,
	而是具体某个类型如:
		JS_STRING,
		JS_OBJECT,
		...
		当到捕获点的时候, 还原JsContext, 并且释放这段期间的锁, 以及其他非托管
	资源.
*/
struct JsValue;
/*
	#include<setjmp.h>
	
	JS_TRY(0){
		可以放置表达式(函数调用, 赋值,...)
		如果使用return , break 语句则需要在之前调用 JsOmitDefender()
		
	}
	doFinally工作
	struct JsValue* e0 = NULL;
	JS_CATCH(e0){
		1. 处理错误
		2. 继续抛出异常
			JsThrowError(e0);
	
	}
	JS_TRY(1){
		可以放置表达式(函数调用, 赋值,...)
		如果使用return , break 语句则需要在之前调用 JsOmitDefender()
		
	}
	doFinally工作
	struct JsValue* e1 = NULL;
	JS_CATCH(e1){
		1. 处理错误
		2. 继续抛出异常
			JsThrowError(e1);
	
	}
*/
/*一个函数中mark都要不同*/
#define JS_TRY(mark) \
	int done##mark; \
	jmp_buf* jmp_buf##mark = (jmp_buf*)JsMalloc(sizeof(jmp_buf)); \
	for(done##mark = 0; \
	    done##mark == 0 && (setjmp(*jmp_buf##mark) == 0 ? \
			(JsBuildDefender(jmp_buf##mark),1) : (JsOmitDefender(),0));  \
		++done##mark, JsOmitDefender())

/*Catch之后, 异常已经被清除了, 并且e会被赋值 [NULL,Value] */
#define JS_CATCH(e) \
		if((e = JsGetError()))
		
		
/****************************************************************************
									通用API
*****************************************************************************/
//模块初始化API
void JsPrevInitError();

void JsPostInitError();

//保存一个还原点到环境中,p 为jmp_buf*指针
void JsBuildDefender(void* p);

//在环境中删除一个最近的还原点
void JsOmitDefender();

//抛出一个String类型的错误
void JsThrowString(char* msg);

//抛出一个error
void JsThrowError(struct JsValue* e);


//检查当前环境是否存在异常, 当并不清除错误
int JsCheckError();

//获得当前错误, 并且清除当前错误, 如果没有则返回NULL
struct JsValue* JsGetError();

//设置一个错误, NULL表示清除错误
void JsSetError(struct JsValue* v);

#endif