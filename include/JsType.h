#ifndef JsTypeH
#define JsTypeH
/*
	类型定义使用的模型为typedef 或者 #define
*/

struct JsEngine;
struct JsObject;
struct JsValue;
enum JsTraceEvent;
struct JsLocation;
//Sys
#ifndef NULL
	#define NULL  ((void *)0)
#endif

#define FALSE 0
#define TRUE  1


/* JsList 类型*/
typedef void* JsList;

/* Js的迭代器类型 */
typedef void* JsIter;
/*
	JsTlsKey : Tls 的key
	JsTlsFn  : 在线程结束的时候调用的析构函数
	Example	 : JsEngine.c : JsSetTlsEngine 
*/
typedef void* JsTlsKey;
typedef void (*JsTlsFn)(void *data);

/*线程信息*/
typedef void* JsThread;
/* Js锁*/
typedef void* JsLock;
/* 线程函数 */
typedef void* (*JsThreadFn)(void* data);

/* 调试函数指针 */
typedef void (*JsVmTraceFn)(struct JsEngine* e,struct JsLocation* l, enum JsTraceEvent event);

/*上下文任务*/
typedef void (*JsContextTaskFn)(struct JsEngine* e,void* data);

/*
	被标准函数所使用的函数指针 
	e 		: JsEngine*
	data	: 来自JsCreateStandardFunction(...)传入的data数据
	res		: [JS_UNDEFINED, JS_NULL, JS_BOOLEAN, JS_STRING,JS_NUMBER,JS_OBJECT,JS_REFERENCE,JS_COMPLETION]
*/
typedef void (*JsFunctionFn)(struct JsEngine* e,void* data,struct JsValue* res);

/*NaN 数据类型*/
#define JS_VALUE_NUMBER_NAN 		((double)(0.0/0.0))

/******************************************************
	如果返回或者输入的类型不在[...]中, 则抛出异常
********************************************************/
/* 
	Object.Get 函数
	self 	: 调用该函数的对象
	prop	: 要查询的prop
	attr	: 如果查到prop,则设置为prop的属性 如果设置为NULL, 则不使用
	res		: [JS_UNDEFINED, JS_NULL, JS_BOOLEAN, JS_STRING,JS_NUMBER,JS_OBJECT]
*/
typedef	void (*JsObjectGetFn)(struct JsObject *self, char *prop,int* attr, struct JsValue *res);

/* 
	Object.Put 函数
	self 	: 调用该函数的对象
	prop	: 设置属性的名字
	value	: [JS_UNDEFINED, JS_NULL, JS_BOOLEAN, JS_STRING,JS_NUMBER,JS_OBJECT]
	flags	: prop的属性
*/
typedef void (*JsObjectPutFn)(struct JsObject *self,char *prop, struct JsValue *value, int flags);
/* 
	Object Boolean类型函数
	self 	: 调用该函数的对象
	prop	: 要处理额prop
	res		: [JS_BOOLEAN]
*/
typedef void (*JsObjectBooleanFn)(struct JsObject *self,char *prop,struct JsValue* res);

/* 
	Object.HasInstance 函数
	self 	: 调用该函数的对象
	instance: 具体的实例对象[JS_UNDEFINED, JS_NULL, JS_BOOLEAN, JS_STRING,JS_NUMBER,JS_OBJECT]
	res		: [JS_BOOLEAN]

*/
typedef void (*JsObjectHasInstanceFn)(struct JsObject *self, struct JsValue *instance, struct JsValue* res);
			
/* 
	Object.Call 和 Object.Construct 函数
	self 	: 调用该函数的对象
	thisobj : this指针, 一个JsObject* 的空间, 不同于self, self 指向该函数所在的对象
	argc	: 参数的个数
	argv	: 具体的参数
	res		: [JS_UNDEFINED, JS_NULL, JS_BOOLEAN, JS_STRING,JS_NUMBER,JS_OBJECT,JS_REFERENCE,JS_COMPLETION]
*/
typedef	void (*JsObjectCallFn)(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res);


/* 
	Object.DefaultValue 函数
	self 	: 调用该函数的对象
	type 	: JS_OBJECT_HIT_TYPE_*
	res		: [JS_UNDEFINED, JS_NULL, JS_BOOLEAN, JS_STRING,JS_NUMBER]
*/
typedef void (*JsObjectDefaultValueFn)(struct JsObject *self,int type, struct JsValue *res);

/* 
	返回:
		正在迭代的对象
		如果返回, NULL 表示迭代完成, 或者出现异常
	
	self 		: 初始化该对象的迭代器
	iter		: 迭代器指针
	initialized : 该迭代器指针是否已经初始化
	res			: [JS_UNDEFINED, JS_NULL, JS_BOOLEAN, JS_STRING,JS_NUMBER, JS_OBJECT]
	
	example:
		JsIter iter;
		//必须有新指针
		struct JsObject* next = &Object;
		char* prop = (*next->NextValue)(&next,&iter,FALSE);
		while(prop != NULL && next != NULL){
			//do something
			prop = (*next->NextValue)(&next,&iter,TRUE);
		}
*/
typedef char* (*JsObjectNextValueFn)(struct JsObject** next, JsIter* iter,int initialized);

#endif