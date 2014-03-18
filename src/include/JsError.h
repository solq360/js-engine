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
	当到捕获点的时候, 还原JsContext, 并且释放这段期间的锁
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
			?(JsBuildRecord(jmp_buf_1234567890_abcdefg),1) \
			: (JsOmitRecord(),0));  \
		++done_1234567890_abcdefg, JsOmitRecord()) 
		
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
//保存一个还原点到环境中,p 为jmp_buf*指针

void JsBuildRecord(void* p);

//每次加锁的时候, 把对应的锁添加到最近还原点的上下文中
void JsPushLockToRecord(JsLock lock);
//解锁的时候,  把给定的锁从最后面扫描, 剔除
void JsPopLockInRecord(JsLock lock);

//在环境中删除一个最近的还原点
void JsOmitRecord();

//获得当前错误, 并且清空, 如果没有则返回NULL
struct JsValue* JsGetError();

//设置一个错误, NULL表示清除错误
void JsSetError(struct JsValue* v);
#endif