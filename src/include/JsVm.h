#ifndef JsVmH
#define JsVmH

#include"JsType.h"

enum JsTraceEvent;
struct JsObject;
struct JsEngine;
struct JsVm;

//VM包含一组engine所在的公共资源
//且全局唯一JsVm
struct JsVm{
	enum {
		JS_VM_START,
		JS_VM_HALT
	}state;
	
	int debug; //是否开启调试
	JsVmTraceFn trace;//调试使用的函数钩子
	
	/*struct JsEngine 临界资源*/
	JsList engines;
	
	
	/*ECMAScript Native Object*/
	struct JsObject* Global;
	int mSize; //加载模块的长度
	char** mPath; //模块位置
	
	JsLock lock; //配置该对象中属性的时候, 需要锁住对象
};
/****************************************************************************
									通用API
*****************************************************************************/
/*
	一个进程只能有一个VM, 且该API在main线程中调用
	创建一个新的VM, 初始化Global对象, 以及加载输入的模块(DLL, SO)
	还有两个init函数, 用于初始化PrevVm 和 PostVm模块的初始化.
*/
struct JsVm* JsCreateVm(int debug,int mSize, char** mPath, JsVmTraceFn traceFn);
/*
	获得本进程的JsVm
*/
struct JsVm* JsGetVm();
/* 
	直接结束进程
*/
void JsHaltVm();
/*
	创建新的JsEngine的时候, 需要把它关联到它所在的VM
*/
void JsEngine2Vm(struct JsEngine* e);


#endif