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

	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	if( p == NULL){
		//TLS中没有对应的Exception对象
		p = (struct JsException* )JsMalloc(sizeof(struct JsException));
		JsListInit(&p->defenders);
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
//都从这里跳转
void JsThrowException(struct JsValue* e){
	JsAssert(e != NULL);
	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	//设置error对象
	p->err = e;
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
	struct JsValue* e = p->err;
	p->err = NULL;
	return e;
}
void JsSetException(struct JsValue* v){

	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	if(p == NULL)
		return;
	p->err = v;
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