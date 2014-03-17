#include"JsObject.h"
#include"JsContext.h"
#include"JsEngine.h"
#include"JsVm.h"
#include"JsValue.h"
#include"JsList.h"
#include"JsSys.h"
#include"JsInit.h"
#include"JsDebug.h"
#include"JsError.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<setjmp.h>

struct JsException{
	JsList jmpbufs; //jmp_buf
	struct JsValue* err; //错误信息
};

static JsTlsKey eKey = NULL;
static void JsTlsClose(void *data);
static void checkInitTls();

//------------------------------------------------------------------

void JsThrowString(char* msg){
	struct JsValue* e =(struct JsValue*) JsMalloc(sizeof(struct JsValue));
	e->type = JS_STRING;
	e->u.string = msg;
	JsThrow(e);
}
void JsThrow(struct JsValue* e){
	JsAssert(e != NULL);
	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	//设置error对象
	p->err = e;
	//跳转目标, 最后一个目标jmp_buf
	jmp_buf* target = (jmp_buf*)JsListGet(p->jmpbufs,JS_LIST_END);
	JsAssert(target != NULL);
	longjmp(*target,1);
}
//创建一个还原点
void JsBuildPoint(void* jmp_buf_p){
	checkInitTls();
	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	JsListPush(p->jmpbufs,jmp_buf_p);
}
//删除一个最近的还原点
void JsOmitPoint(){
	checkInitTls();
	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	JsListRemove(p->jmpbufs,JS_LIST_END);
}
//获得当前错误
struct JsValue* JsGetError(){
	checkInitTls();
	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	struct JsValue* e = p->err;
	p->err = NULL;
	return e;

}
void JsSetError(struct JsValue* v){
	checkInitTls();
	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	p->err = v;
}

static void checkInitTls(){
	if(eKey == NULL){
		JsGLock();
		if(eKey == NULL){
			eKey = JsCreateTlsKey(JsTlsClose);
			struct JsException* p = (struct JsException* )JsMalloc(sizeof(struct JsException));
			JsSetTlsValue(eKey,p);
		}
		JsGLock();
	}
}
static void JsTlsClose(void *data){
	struct JsException* p = data;
	p->jmpbufs = NULL;
	p->err = NULL;
}