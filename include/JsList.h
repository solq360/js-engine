#ifndef JsListH
#define JsListH

#include"JsType.h"

//JsList.h
#define JS_LIST_END     -1
#define JS_LIST_FIRST 	0
/****************************************************************************
									通用API
*****************************************************************************/
/*
	数据结构如下：
	[node(first), node, ... ,node,end]
	header: NULL
	注: List类型在系统中, 每个对象(Context, Engine, Vm, Object ...)都要进行复制, 
		防止多线程中数据被破坏
*/
//创建一个JsList类型数据
JsList JsCreateList();
//向尾部退入数据
void JsListPush(JsList l,void* data);
//返回具体指针, NULL代表具体的数据
void* JsListGet(JsList l,int pos);
//返回大小
int JsListSize(JsList l);
//返回是否成功
int JsListRemove(JsList l,int pos);
//返回一个被初始化过的JsList, 并且拷贝head
JsList JsListCopy(JsList dst,JsList src);

#endif