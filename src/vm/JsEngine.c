#include"JsObject.h"
#include"JsContext.h"
#include"JsEngine.h"
#include"JsVm.h"
#include"JsValue.h"
#include"JsList.h"
#include"JsSys.h"
#include"JsDebug.h"
#include"JsError.h"
#include<setjmp.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
//当前线程的engine对象的key
static JsTlsKey engineKey = NULL;
static void checkEngineKey();
//--------------------------------------------------------
struct JsEngine* JsCreateEngine(){
	struct JsVm* jsVm = JsGetVm();
	struct JsEngine* e = (struct JsEngine*)JsMalloc(sizeof(struct JsEngine));
	e->jsVm = jsVm;
	JsEngine2Vm(e);
	
	e->state = JS_ENGINE_KERNEL;
	
	e->exec = NULL;
	JsListInit(e->waits);
	JsListInit(e->pools);
	
	JsCreateLock(&e->lock);
	return e;
}

void JsDispatch(struct JsContext* c){

//验证
	if(c == NULL)
		return;
	struct JsEngine* e = c->engine;
	if(e == NULL || e->jsVm == NULL 
		|| e->state ==JS_ENGINE_STOPPED || e->jsVm->state == JS_VM_HALT)
		return;
//添加到wait队列中
	JsLockup(e->lock);

	JsListPush(e->waits,c);
	c->thread = NULL;
	c = NULL;
	
	JsUnlock(e->lock);
//循环执行waits队列
	while(TRUE){
		JsLockup(e->lock);

		if(e->exec != NULL){
			JsUnlock(e->lock);
			return;
		}
		//获得waits第一个Context, 并且删除它
		e->exec = (struct JsContext*)JsListGet(e->waits,JS_LIST_FIRST);
		JsListRemove(e->waits,JS_LIST_FIRST);
		if(e->exec == NULL){
			//waits队列中不存在等待的context
			JsUnlock(e->lock);
			return;
		}
		//记录当前线程信息
		e->exec->thread = JsCurThread();
		JsUnlock(e->lock);
		
		
		//配置环境
		struct JsEngine* tlsEngine = JsGetTlsEngine();
		struct JsContext* tlsContext = JsGetTlsContext();
		JsSetTlsEngine(e);
		JsSetTlsContext(e->exec);
		JS_TRY{
			(*e->exec->task)(e);
		}JS_CATCH{
			//简单的清除错误
			JsGetError();
		}
		//还原环境
		JsSetTlsEngine(tlsEngine);
		JsSetTlsContext(tlsContext);
		
		JsLockup(e->lock);
		//从pools中剔除exec指向的context
		int size,i;
		size = JsListSize(e->pools);
		for(i = 0 ; i < size; ++i){
			if(JsListGet(e->pools,i) == e->exec){
				JsListRemove(e->pools,i);
				break;
			}
		}
		e->exec = NULL;
		JsUnlock(e->lock);
	}

}
//注册一个新的context到reg中, lock
void JsContext2Engine(struct JsEngine* e, struct JsContext* c){
	JsAssert(e != NULL && c != NULL);
	JsLockup(e->lock);
	JsListPush(e->pools,c);
	JsUnlock(e->lock);
	
}
void JsStopEngine(struct JsEngine* e){
	
	struct JsContext* c;
	int size ,i;
	if(e ==NULL)
		return;
	JsLockup(e->lock);
	
	if(e->state == JS_ENGINE_STOPPED){
		JsUnlock(e->lock);
		return;
	}
	//Mark
	e->state = JS_ENGINE_STOPPED;
	
	size = JsListSize(e->pools);
	for( i = 0 ; i < size ; ++i){
		c = (struct JsContext*)JsListGet(e->pools,i);
		if(c->thread){
			//停止nio线程
			JsCloseThread(c->thread);
			c->thread = NULL;
		}
	}
	JsUnlock(e->lock);
}
/*获得当前线程Engine*对象*/
void JsSetTlsEngine(struct JsEngine* e){
	checkEngineKey();
	JsSetTlsValue(engineKey,e);
}
struct JsEngine* JsGetTlsEngine(){
	struct JsEngine* e;
	checkEngineKey();
	e = (struct JsEngine*)JsGetTlsValue(engineKey);
	return e;
}
static void checkEngineKey(){
	if(engineKey == NULL){
		JsGLock();
		if(engineKey == NULL){
			//初始化key
			engineKey = JsCreateTlsKey(NULL);
			//不需要配置具体的Value
		}
		JsGUnlock();
	}
}