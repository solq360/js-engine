#include"JsObject.h"
#include"JsContext.h"
#include"JsEngine.h"
#include"JsVm.h"
#include"JsValue.h"
#include"JsList.h"
#include"JsSys.h"
#include"JsInit.h"
#include"JsDebug.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

//全局唯一变量
static struct JsVm* g_JsVm = NULL;

//-----------------------------------------------
static void jsVmTrace(struct JsEngine* jsEngine,struct JsLocation* l,enum JsTraceEvent jsTraceEvent);
static void jsLoadModule(struct JsVm* vm);

//------------------------------------------------
//初始化一个新的Vm
struct JsVm* JsCreateVm(int debug,int mSize, char** mPath,
					JsVmTraceFn traceFn){
	if(g_JsVm)
		return g_JsVm;
	
	g_JsVm = (struct JsVm*)JsMalloc(sizeof(struct JsVm));
	
	g_JsVm->state = JS_VM_START;
	
	g_JsVm->debug = debug;
	if(traceFn == NULL)
		g_JsVm->trace = jsVmTrace;
	else
		g_JsVm->trace = traceFn;
	
	JsListInit(g_JsVm->engines);
	//初始化Ecmascript的对象
	JsECMAScriptObjectInit(g_JsVm);
	JsAssert(g_JsVm->Global != NULL);
	
	g_JsVm->mSize = mSize;
	g_JsVm->mPath = mPath;
	jsLoadModule(g_JsVm);
	
	JsCreateLock(&g_JsVm->lock);
	
	return g_JsVm;
}
//关闭Vm, 锁住lock
void JsHaltVm(){
	JsHalt();
}
//addEngine, 锁住lock
void JsEngine2Vm(struct JsEngine* e){
	JsAssert(g_JsVm != NULL && e != NULL);
	JsLockup(g_JsVm->lock);
	JsListPush(g_JsVm->engines,e);
	JsUnlock(g_JsVm->lock);
}
struct JsVm* JsGetVm(){
	JsAssert( g_JsVm != NULL);
	return g_JsVm;
}
//-------------------------
static void jsVmTrace(struct JsEngine* jsEngine,struct JsLocation* l,enum JsTraceEvent event){
	
}
static void jsLoadModule(struct JsVm* vm){


}