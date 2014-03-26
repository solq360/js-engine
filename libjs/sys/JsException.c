#include"JsObject.h"
#include"JsContext.h"
#include"JsEngine.h"
#include"JsVm.h"
#include"JsValue.h"
#include"JsList.h"
#include"JsSys.h"
#include"JsInit.h"
#include"JsDebug.h"
#include"JsException.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<setjmp.h>

struct JsException{
	JsList defenders; //jmp_buf* 
	// 抛出异常的定位信息 JsLocation*
	JsList stack;
	struct JsValue* err; //错误信息
};

static JsTlsKey eKey = NULL;
static void JsTlsClose(void *data);
static void JsInitTlsKey();
//------------------------------------------------------------------
void JsPrevInitException(){
	JsInitTlsKey();
}
void JsPostInitException(){

}

//创建一个还原点
void JsBuildDefender(void* jmp_buf_p){
	JsAssert(jmp_buf_p != NULL);
	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	if( p == NULL){
		//TLS中没有对应的Exception对象
		p = (struct JsException* )JsMalloc(sizeof(struct JsException));
		p->defenders = JsCreateList();
		p->stack = JsCreateList();
		p->err = NULL;
		JsSetTlsValue(eKey,p);
	}
	//压入该还原点
	JsListPush(p->defenders,jmp_buf_p);
}

//删除一个最近的还原点
void JsOmitDefender(){
	
	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	if(p == NULL)
		return;
	JsListRemove(p->defenders,JS_LIST_END);
}


void JsThrowString(char* msg){
	struct JsValue* e =(struct JsValue*) JsMalloc(sizeof(struct JsValue));
	e->type = JS_STRING;
	if(msg == NULL)
		msg = "EMPTY MSG";
	e->u.string = msg;
	JsThrowException(e);
}
//抛出异常都调用这个函数
void JsThrowException(struct JsValue* e){
	JsAssert(e != NULL);
	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	JsAssert(p != NULL);
	//设置error对象
	p->err = e;
	//每次都重新配置一个List
	p->stack = JsCreateList();
	//设置stack, 先拷贝当前Context的位置'
	struct JsContext* tc = JsGetTlsContext();
	if(tc != NULL){
		if(tc->stack != NULL)
			JsListCopy(p->stack,tc->stack);
		if(tc->pc != NULL)
			JsListPush(p->stack,tc->pc);
	}	
	//获得记录
	void* r = JsListGet(p->defenders,JS_LIST_END);
	JsAssert(r != NULL);
	longjmp(*(jmp_buf*)r,1);
}
//rethrow函数
void JsReThrowException(struct JsValue* e){
	JsAssert(e != NULL);
	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	JsAssert(p != NULL);
	//设置error对象
	p->err = e;
	//不进行修改stack位置
	
	//获得记录
	void* r = JsListGet(p->defenders,JS_LIST_END);
	JsAssert(r != NULL);
	longjmp(*(jmp_buf*)r,1);

}
int JsCheckException(){

	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	if(p == NULL)
		return FALSE;
	return p->err != NULL;
}
//获得当前错误,  并且消除e
struct JsValue* JsGetException(){

	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	if(p == NULL)
		return NULL;
	return p->err;
}
//获得当前错误, 并且清除当前错误, 如果没有则返回NULL
struct JsValue* JsCatchException(){
	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	if(p == NULL)
		return NULL;
	struct JsValue* e = p->err;
	p->err = NULL;
	return e;
}
JsList JsGetExceptionStack(){
	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	if(p == NULL)
		return NULL;
	return p->stack;

}

static void JsInitTlsKey(){
	eKey = JsCreateTlsKey(JsTlsClose);
}
static void JsTlsClose(void *data){
	struct JsException* p = data;
	if( p != NULL){
		p->defenders = NULL;
		p->err = NULL;
	}
}