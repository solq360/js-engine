#ifndef JsContextH
#define JsContextH

#include"JsType.h"

struct JsEngine;
struct JsObject;
struct JsFunction;


/*
	上下文, 对执行过程的控制, 每个JsContext只属于一个线程(Engine,NIO)
	所以不会发生多线程占用, 数据要被独占, 不能给其他对象使用
*/
struct JsContext{
	
	struct JsEngine* engine;//指向挂载的Engine
	/*
		struct JsObject 当前Context查询的scope
		每个JsContext都唯一
	*/
	JsList scope;
	/*
		struct JsLocation 函数的堆栈表示
		每个JsContext都唯一, 基本在
			JsEval.c:Call | Construct
		中被配置
	*/
	JsList stack;
	/*
		当前代码语句, 每次遇到新的statement的时候被修改
		JsEval.c : TRACE
	*/
	struct JsLocation* pc;
	/*
		当前this
	*/
	struct JsObject* thisObj;
	/*
		代码区段var的属性
	*/
	int varattr;

	/*
		该线程当前所在的线程信息, 作为Stop Engine的时候停止的参数
		当该Context不占用线程的时候, 设置为NULL, 如加入Engine.waits
		队列中
	*/
	JsThread thread; 
	/*
		当JsContext被JsEngine调用的时候, 会执行如下属性
	*/
	void* data; //和task钩子函数联合使用
	JsContextTaskFn task; //指向Task的函数
	
};
/****************************************************************************
									通用API
*****************************************************************************/
//模块初始化API
void JsPrevInitContext();
void JsPostInitContext();
/*
	创建新的JsContext
	如果 c == NULL, 则配置为全局Context类型
	否则 拷贝c
	*最后把该context注册到JsEngine中
*/
struct JsContext* JsCreateContext(struct JsEngine* e, struct JsContext* c, 
			JsContextTaskFn taskFn, void* data);
			
/*
	创建新的JsContext
	如果 c == NULL,则配置为全局Context
	否则 拷贝c
	*纯粹的拷贝context, 不进行注册到JsEngine中.
*/	
struct JsContext* JsCopyContext(struct JsContext* c);	
/*
	
	这两个个函数是非线程安全的, 如果在查询过程中发生属性被
	删除(其他线程对delete scope.object.prop操作)则会发生未知结果
	1. 返回 JS_UNDEFINED(其实 在scope.prev.object 中存在一个同name的属性)
	2. 返回 已经被删除的属性
	注: 如果c == NULL, 表示从Global中获取对象
*/		
//查询name指定的属性, 并且返回一个Ref
void JsFindValueRef(struct JsContext* c, char* name,struct JsValue* res);
//查询name指定的属性, 返回非Ref
//可能返回JS_UNDEFINED表示未查询到
void JsFindValue(struct JsContext* c, char* name,struct JsValue* res);

/*获得当前线程Context*对象*/
void JsSetTlsContext(struct JsContext* c);
struct JsContext* JsGetTlsContext();
#endif