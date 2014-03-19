#include"JsObject.h"
#include"JsContext.h"
#include"JsEngine.h"
#include"JsVm.h"
#include"JsValue.h"
#include"JsList.h"
#include"JsSys.h"
#include"JsAst.h"
#include"JsDebug.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

//当前线程的context对象的key
static JsTlsKey key = NULL;
static void checkKey();

struct JsContext* JsCreateContext(struct JsEngine* e, struct JsContext* c, 
			JsContextTaskFn task, void* data){
	struct JsContext* context;
	JsAssert(e != NULL);
	JsAssert(task != NULL);
	context = JsCopyContext(c);
	context->engine = e;
	//注册到JsEngine中
	JsContext2Engine(e,context);
	context->thread = NULL;
	context->data = data;
	context->task = task;

	return context;	
}

struct JsContext* JsCopyContext(struct JsContext* c){
	struct JsContext* context;
	
	context = (struct JsContext*)JsMalloc(sizeof(struct JsContext));
	JsListInit(context->scope);
	JsListInit(context->stack);
	if(c == NULL){
		context->engine = NULL;
		JsListPush(context->scope,JsGetVm()->Global);
		struct JsStack* stack = (struct JsStack*)JsMalloc(sizeof(struct JsStack));
		stack->loc = NULL;
		stack->function = NULL;//表示Global
		JsListPush(context->stack,stack);
		context->thisObj = JsGetVm()->Global;
		/*10.2.1*/
		context->varattr = JS_OBJECT_ATTR_DONTDELETE;
		context->thread = NULL;
		context->data = NULL;
		context->task = NULL;
	}else{
		context->engine = c->engine;
		JsListCopy(context->scope,c->scope);
		JsListCopy(context->stack,c->stack);
		context->thisObj = c->thisObj;
		context->varattr = c->varattr;
		context->thread = c->thread;
		context->data = c->data;
		context->task = c->task;
		
	}
	return context;	
}
//通过scope, 查询name指定的属性, 并且返回一个Ref
void JsFindValueRef(struct JsContext* c, char* name,struct JsValue* res){

	struct JsObject* obj = NULL;
	struct JsObject* base = NULL;
	int size,i;
	struct JsValue v;
	
	if(c == NULL){
		obj = JsGetVm()->Global;
		(*obj->HasProperty)(obj,name,&v);
		if(v.type == JS_BOOLEAN && v.u.boolean == TRUE){
			base = obj;
		}
	}else{
		size = JsListSize(c->scope);
		for( i= size-1; i>=0;--i){
			obj = JsListGet(c->scope,i);
			(*obj->HasProperty)(obj,name,&v);
			if(v.type == JS_BOOLEAN && v.u.boolean == TRUE){
				base = obj;
				break;
			}
		}
	}
	//Mark
	res->type = JS_REFERENCE;
	res->u.reference.base = base;
	res->u.reference.name = name;
	return;
}
//通过scope, 查询name指定的属性, 返回非Ref
void JsFindValue(struct JsContext* c, char* name,struct JsValue* res){
				
	struct JsObject* obj = NULL;
	struct JsObject* base = NULL;
	int size,i;
	struct JsValue v;
	if(c == NULL){
		obj = JsGetVm()->Global;
		(*obj->HasProperty)(obj,name,&v);
		if(v.type == JS_BOOLEAN && v.u.boolean == TRUE){
			base = obj;
		}
	}else{
		size = JsListSize(c->scope);
		for( i= size-1; i>=0;--i){
			obj = JsListGet(c->scope,i);
			(*obj->HasProperty)(obj,name,&v);
			if(v.type == JS_BOOLEAN && v.u.boolean == TRUE){
				base = obj;
				break;
			}
		}
	}
	if(base == NULL){
		res->type = JS_UNDEFINED;
	}else{
		(*base->Get)(base,name,NULL,res);
	}
}


/*获得当前线程Context*对象*/
void JsSetTlsContext(struct JsContext* c){
	checkKey();
	JsSetTlsValue(key,c);

}
struct JsContext* JsGetTlsContext(){
	struct JsContext* c;
	checkKey();
	c = (struct JsContext*)JsGetTlsValue(key);
	return c;
}
static void checkKey(){
	if(key == NULL){
		JsGLock();
		if(key == NULL){
			//初始化key
			key = JsCreateTlsKey(NULL);
			//不需要配置具体的Value
		}
		JsGUnlock();
	}
}