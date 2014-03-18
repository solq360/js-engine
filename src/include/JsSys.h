#ifndef JsSysH
#define JsSysH
#include"JsType.h"
struct JsVm;
struct JsEngine;
struct JsContext;

/****************************************************************************
									通用API
*****************************************************************************/
/*加锁解锁会注册该锁到还原点上下文*/
/*锁 API*/
void JsCreateLock(JsLock* lock);
void JsLockup(JsLock lock);
void JsUnlock(JsLock lock);
void JsDestroyLock(JsLock* lock);
//全局锁
void JsGLock();
void JsGUnlock();


/*线程 API*/
//返回线程相关信息
JsThread JsCurThread();
//启动线程
JsThread JsStartThread(JsThreadFn fn,void* data);
//自身线程安全退出
void JsCloseSelf();
//终止非当前线程
void JsCloseThread(JsThread thread);
//马上退出
void JsHalt();


/*内存 API*/
//清空内存为NULL, 并且加入heap管理中
void* JsMalloc(int size);
//类似realloc
void* JsReAlloc(void* mem,int newSize);

/*把数据存储在当前TLS中*/
JsTlsKey JsCreateTlsKey(JsTlsFn fn);
void JsSetTlsValue(JsTlsKey key, void* value);
void* JsGetTlsValue(JsTlsKey key);
#endif