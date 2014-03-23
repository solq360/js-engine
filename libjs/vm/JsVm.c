#include"JsObject.h"
#include"JsContext.h"
#include"JsEngine.h"
#include"JsVm.h"
#include"JsValue.h"
#include"JsList.h"
#include"JsSys.h"
#include"JsInit.h"
#include"JsError.h"
#include"JsDebug.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

//全局唯一变量
static struct JsVm* g_JsVm = NULL;

//初始化各种优先于VM模块的模块单位
static void JsPrevInitModules();
//加载DLL, SO 类型的模块
static void JsLoadShareModule(struct JsVm* vm);
//Vm初始化完成后, 再初始化的资源模块
static void JsPostInitModules();

//------------------------------------------------
//初始化一个新的Vm
struct JsVm* JsCreateVm(int debug,int mSize, char** mPath,
					JsVmTraceFn traceFn){
	if(g_JsVm)
		return g_JsVm;
	//初始化优先于VM的模块
	JsPrevInitModules();
	g_JsVm = (struct JsVm*)JsMalloc(sizeof(struct JsVm));
	g_JsVm->state = JS_VM_START;
	g_JsVm->debug = debug;
	g_JsVm->trace = traceFn;
	
	JsListInit(&g_JsVm->engines);
	//初始化Ecmascript的对象
	JsECMAScriptObjectInit(g_JsVm);
	JsAssert(g_JsVm->Global != NULL);
	
	g_JsVm->mSize = mSize;
	g_JsVm->mPath = mPath;
	JsLoadShareModule(g_JsVm);
	
	JsCreateLock(&g_JsVm->lock);
	JsPostInitModules();
	
	return g_JsVm;
}
//关闭Vm
void JsHaltVm(){
	JsHalt();
}
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


static void JsPrevInitModules(){
	//.h中声明为 JsPrevInitializeModule 的API
	JsPrevInitSys();
	JsPrevInitError();
	JsPrevInitEngine();
	JsPrevInitContext();
}

static void JsLoadShareModule(struct JsVm* vm){


}

static void JsPostInitModules(){
	//.h中声明为 JsPostInitializeModule 的API
	JsPostInitSys();
	JsPostInitError();
	JsPostInitEngine();
	JsPostInitContext();
}
