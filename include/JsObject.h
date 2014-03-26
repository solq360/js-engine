#ifndef JsObjectH
#define JsObjectH

/*
	继承结构可以参考
	http://www.cnblogs.com/tickobject/p/3605365.html
*/

#include"JsType.h"

struct JsEngine;
struct JsValue;
struct JsObject;

//JsObject.defaultValue.hit or ToObject.hit
#define JS_OBJECT_HIT_TYPE_STRING 	0
#define JS_OBJECT_HIT_TYPE_NUMBER 	1

//JsObject.flag
//!ATTR_READONLY !ATTR_DONTENUM !ATTR_DONTDELETE
#define JS_OBJECT_ATTR_DEFAULT 	    0x0
#define JS_OBJECT_ATTR_READONLY 	0x1
#define JS_OBJECT_ATTR_DONTENUM 	0x2
#define JS_OBJECT_ATTR_DONTDELETE 	0x4
#define JS_OBJECT_ATTR_STRICT		0x7

/*
	对象中的属性都为只读属性, 当初始化完成后, 就不能再进行修改了
	且其中任何属性都要为非NULL, 如果对象无该内置属性,则使用Dark*函数
	设置
*/
struct JsObject{
	
	/*****************************数据属性******************/
	struct JsObject* 		Prototype; 		
	const char *			Class;
	//synchronized 锁, 不同于函数的sync锁
	JsLock 					SyncLock;
	/*函数特有scope, 且都唯一*/
	JsList 					Scope;
	/*****************************函数属性******************/
	/*通用操作属性*/
	JsObjectGetFn			Get;
	JsObjectPutFn			Put;
	JsObjectBooleanFn		CanPut;
	JsObjectBooleanFn		HasProperty;
	JsObjectBooleanFn		Delete;
	JsObjectDefaultValueFn  DefaultValue;
	/*额外的通用操作属性*/
	JsObjectBooleanFn		HasOwnProperty;
	JsObjectNextValueFn     NextValue;
	
	/*函数特有属性函数*/
	JsObjectCallFn			Construct;
	JsObjectCallFn			Call;
	JsObjectHasInstanceFn	HasInstance;
	
	/************************用于继承的数据结构*************/
	void** pb; //公共数据块数组, 第一个为都为JsObjectPb
	void** sb; //私有数据块,相当于[[Value]]
};

/****************************************************************************
									通用API
*****************************************************************************/
//JsStandardObject 所在的层 
#define JS_STANDARD_OBJECT_FLOOR	0

//构造一个JsObject的空间, floor 声明了pb 和 sb 数组的长度
// RootObject为 0 即JsStandardObject所在的层数
struct JsObject* JsAllocObject(int floor);

//如果Vm没有Object或者Function, prototype 配置为NULL
//o = NULL, 构造新空间
struct JsObject* JsCreateStandardObject(struct JsObject* o);

//如果Scope = NULL, 则使用Global代替
//如果inst = TRUE, 则使用标准HasInstance, 且需要对象中属性中
//添加prototype属性(和Constructor属性配合使用)
//o = NULL, 构造新空间
struct JsObject* JsCreateStandardFunctionObject(struct JsObject* o,JsList scope, int inst);

/*
	创建一个特化(使用JsFunction指针)的函数对象, 被使用在系统接口或者JsEval中被调用
	o:
		o = NULL, 构造新空间
	scope:
		scope == NULL , scope = VM->Global
	argc:	
		预定义的函数参数
	argv:
		参数的名字
	data:
		和fn配合使用的数据, 在Object->cls->Call的时候传递给fn
	name:
		该函数的名字, 可以为NULL
	sync:
		函数是否需要同步
	最终的到的objcect添加了一般的函数属性(通过obj->cls->Get获取)如:
		length,
		prototype.
	*整个函数类似JS中标准的函数, 具有自己的Scope
		
*/
struct JsObject* JsCreateStandardSpecFunction(struct JsObject* o,JsList scope,int argc, 
		char** argv,JsSpecFunctionFn fn,void* data,char* name,int sync);
		
/*标准对象内置的函数*/
void JsStandardGet( struct JsObject *self,  char *prop,int* flag, struct JsValue *res);
void JsStandardPut( struct JsObject *self, char *prop, struct JsValue *val, int flags);
void JsStandardCanPut( struct JsObject *self, char *prop,struct JsValue* res);
void JsStandardHasProperty( struct JsObject *self, char *prop,struct JsValue* res);	
void JsStandardDelete( struct JsObject *self,  char *prop,struct JsValue* res);
void JsStandardDefaultValue( struct JsObject *self, int hint, struct JsValue *res);
void JsStandardHasOwnProperty( struct JsObject *self, char *prop,struct JsValue* res);
char* JsStandardNextValue(struct JsObject** next, JsIter* iter,int initialized);

void JsStandardConstruct(struct JsObject *self, struct JsObject *thisobj,int argc, struct JsValue **argv, struct JsValue *res);
void JsStandardCall(struct JsObject *self, struct JsObject *thisobj,int argc, struct JsValue **argv, struct JsValue *res);
void JsStandardHasInstance(struct JsObject *self, struct JsValue *instance,struct JsValue* res);

/*非标准内置函数, 直接抛出异常*/
void JsDarkGet(struct JsObject *self, char *prop,int* flag, struct JsValue *res);
void JsDarkPut(struct JsObject *self,char *prop, struct JsValue *value, int flags);
void JsDarkBoolean(struct JsObject *self,char *prop,struct JsValue* res);
void JsDarkDefaultValueFn(struct JsObject *self,int type, struct JsValue *res);
char* JsDarkNextValueFn(struct JsObject** next, JsIter* iter,int initialized);
void JsDarkCall(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res);
void JsDarkHasInstance(struct JsObject *self, struct JsValue *instance, struct JsValue* res);


#endif