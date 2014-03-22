#ifndef JsEngineH
#define JsEngineH

#include"JsType.h"

struct JsVm;
struct JsContext;


/*
	具有调度功能, 每一个Engine只有一个正在执行的上下文
	其他的上下文处于等待状态(queue)
	一个Engin代表一个执行单元
*/
struct JsEngine{
	struct JsVm* vm;
	enum{
		JS_ENGINE_RUNNING,//运行Source中
		JS_ENGINE_STOPPED,//被关闭后, 不会再启动
		JS_ENGINE_KERNEL
	}state;
	
	/*临界资源 正在执行的JsContext*/
	struct JsContext* exec;
	/*struct JsContext  临界资源 等待Engine调度*/
	JsList waits; 
	/*struct JsContext 临界资源 和该Engine关联的JsContext*/
	JsList pools;
	
	JsLock lock; //jsEngine 对对象锁, 防止多个Context对jsEngine抢占
};

/****************************************************************************
									通用API
*****************************************************************************/
//模块初始化API
void JsPrevInitEngine();
void JsPostInitEngine();
/*
	创建一个Engine, 初始化各个属性, state = JS_ENGINE_KERNEL
*/
struct JsEngine* JsCreateEngine();
/*
	把c加入到wait队列中, 然后调度执行
	执行完的Context从pools中剔除
*/
void JsDispatch(struct JsContext* c);
/*
	在创建Context需要关联新建的Context到它所属的Engine
*/
void JsContext2Engine(struct JsEngine* e, struct JsContext* c);

/*
	不能由与该Engine相关线程关闭.
	关闭所有于该Engine关联的线程, 通过Context->thread来完成,
	所以需要注意context->thread 的赋值
*/
void JsStopEngine(struct JsEngine* e);

/*获得当前线程Engine*对象*/
void JsSetTlsEngine(struct JsEngine* e);
struct JsEngine* JsGetTlsEngine();

#endif