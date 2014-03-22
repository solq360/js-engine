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
#include<pthread.h>

pthread_mutex_t globalLock = PTHREAD_MUTEX_INITIALIZER;
//配置一个程序内部使用, 可以计数的锁的属性
static pthread_mutexattr_t* lock_attr = NULL;;
static void JsInitLockAttr();


void JsPrevInitSys(){
	JsInitLockAttr();	
}
void JsPostInitSys(){

}

void JsGLock(){
	pthread_mutex_lock(&globalLock);
}
void JsGUnlock(){
	pthread_mutex_unlock(&globalLock);
}

void JsCreateLock(JsLock* lock){
	pthread_mutex_t* a = (pthread_mutex_t*) JsMalloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(a,lock_attr);
	*lock = (JsLock)a;
}
void JsLockup(JsLock lock){
	JsAssert(lock != NULL);
	pthread_mutex_lock((pthread_mutex_t*)lock);
}
void JsUnlock(JsLock lock){
	JsAssert(lock != NULL);
	pthread_mutex_unlock((pthread_mutex_t*)lock);  
}
void JsDestroyLock(JsLock* lock){
	if(lock == NULL || *lock == NULL)
		return;
	pthread_mutex_t** p = (pthread_mutex_t**)lock;
	pthread_mutex_destroy(*p);
	*lock = NULL;
}


//返回线程相关信息
JsThread JsCurThread(){
	pthread_t* p = (pthread_t *)JsMalloc(sizeof(pthread_t));
	*p = pthread_self();
	return p;
}
JsThread JsStartThread(JsThreadFn fn,void* data){
	pthread_t* p = (pthread_t *)JsMalloc(sizeof(pthread_t));
	int err = pthread_create(p,NULL,fn,data); 
	if(err !=0){
		return NULL;
	}
	return p;
}
void JsCloseSelf(){
	pthread_exit(NULL);
}
void JsCloseThread(JsThread thread){
	if(thread == NULL)
		return ;
	pthread_t* p = (pthread_t*)thread;
	pthread_cancel(*p);
}

void JsHalt(){
	exit(0);
}
void* JsMalloc(int size){
	if( size <= 0 )  
		return NULL;
	void * p = malloc(size);
	JsAssert(p != NULL);
	memset(p,0,size);
	return p;
}
void* JsReAlloc(void* mem,int newSize){
	void* p = realloc(mem,newSize);
	JsAssert(p != NULL);
	return p;
}

/*把数据存储在当前TLS中*/
JsTlsKey JsCreateTlsKey(JsTlsFn fn){
	pthread_key_t * key = (pthread_key_t*)JsMalloc(sizeof(pthread_key_t));
	pthread_key_create( key, fn);
	return key;
}
void JsSetTlsValue(JsTlsKey key, void* value){
	JsAssert(key != NULL);
	pthread_setspecific( *(pthread_key_t*)key,value);
}
void* JsGetTlsValue(JsTlsKey key){
	JsAssert(key != NULL);
	return pthread_getspecific( *(pthread_key_t*)key);
}

/****************************************************************/
static void JsInitLockAttr(){
	lock_attr = (pthread_mutexattr_t*) JsMalloc(sizeof(pthread_mutexattr_t));
	pthread_mutexattr_init(lock_attr);
	pthread_mutexattr_setpshared(lock_attr, PTHREAD_PROCESS_PRIVATE);
	pthread_mutexattr_settype(lock_attr ,PTHREAD_MUTEX_RECURSIVE);
}
