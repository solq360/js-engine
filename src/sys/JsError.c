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

struct JsRecord{
	jmp_buf* bp;
	//复制当前的context
	struct JsContext* cp;
	//记录建立在该记录上的锁句柄
	JsList locks;
};
struct JsException{
	JsList records; 
	struct JsValue* err; //错误信息
};

static JsTlsKey eKey = NULL;
static void JsTlsClose(void *data);
static void JsInitTlsKey();

//------------------------------------------------------------------
void JsPrevInitError(){
	JsInitTlsKey();
}
void JsPostInitError(){


}

void JsThrowString(char* msg){
	struct JsValue* e =(struct JsValue*) JsMalloc(sizeof(struct JsValue));
	e->type = JS_STRING;
	if(msg == NULL)
		msg = "EMPTY MSG";
	e->u.string = msg;
	JsThrow(e);
}
//都从这里跳转
void JsThrow(struct JsValue* e){
	JsAssert(e != NULL);
	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	//设置error对象
	p->err = e;
	//获得记录
	struct JsRecord* r = (struct JsRecord*)JsListGet(p->records,JS_LIST_END);
	//先前没有设置还原点
	JsAssert(r != NULL);
	//还原JS环境
	struct JsContext* curContext  = JsGetTlsContext();
	if(curContext != NULL && r->cp != NULL){
		curContext->scope = r->cp->scope;
		curContext->stack = r->cp->stack;
		curContext->varattr = r->cp->varattr;
		curContext->thisObj = r->cp->thisObj;
	}
	//解锁
	int size = JsListSize(r->locks);
	int i ;
	for( i = size - 1 ; i >= 0 ; -- i){
		JsLock l = (JsLock)JsListGet(r->locks,i);
		JsUnlock(l);
	}
	longjmp(*r->bp,1);
}
//创建一个还原点
void JsBuildRecord(void* jmp_buf_p){

	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	if( p == NULL){
		p = (struct JsException* )JsMalloc(sizeof(struct JsException));
		JsListInit(&p->records);
		p->err = NULL;
		JsSetTlsValue(eKey,p);
	}
	struct JsRecord* r = (struct JsRecord*)JsMalloc(sizeof(struct JsRecord));
	//还原点
	r->bp = (jmp_buf*)jmp_buf_p;
	//JS环境记录
	struct JsContext* curC = JsGetTlsContext();
	//处于init阶段, 不需要记录Context
	if(curC == NULL)
		r->cp = NULL;
	else
		r->cp = JsCopyContext(curC);
	//初始化锁记录
	JsListInit(&r->locks);
	
	JsListPush(p->records,r);
}
//每次加锁的时候, 把对应的锁添加到最近还原点的上下文中
void JsPushLockToRecord(JsLock lock){

	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	if(p == NULL)
		return;
	//获得最近的记录
	struct JsRecord* r = (struct JsRecord*)JsListGet(p->records,JS_LIST_END);
	if(r == NULL)
		return;
	//添加到最近都的记录上
	JsListPush(r->locks,lock);
}

//解锁的时候,  把给定的锁从最后面扫描, 剔除
void JsPopLockInRecord(JsLock lock){

	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	if(p == NULL)
		return;
	//获得最近的记录
	struct JsRecord* r = (struct JsRecord*)JsListGet(p->records,JS_LIST_END);
	if(r == NULL)
		return;
	int size = JsListSize(r->locks);
	int i;
	for(i= size -1; i>=0;--i){
		JsLock l = (JsLock)JsListGet(r->locks,i);
		if(l == lock){
			//找到距离顶部最近的该锁
			JsListRemove(r->locks,i);
			break;
		}
	}
	
}
//删除一个最近的还原点
void JsOmitPoint(){
	
	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	if(p == NULL)
		return;
	JsListRemove(p->records,JS_LIST_END);
}
int JsCheckError(){

	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	if(p == NULL)
		return FALSE;
	return p->err != NULL;
}
//获得当前错误
struct JsValue* JsGetError(){

	struct JsException* p  = (struct JsException*)JsGetTlsValue(eKey);
	if(p == NULL)
		return NULL;
	struct JsValue* e = p->err;
	p->err = NULL;
	return e;
}
void JsSetError(struct JsValue* v){

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
		p->records = NULL;
		p->err = NULL;
	}
}