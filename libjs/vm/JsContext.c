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
static void JsInitContextTlsKey();

//模块初始化API
void JsPrevInitContext(){
	JsInitContextTlsKey();
}
void JsPostInitContext(){

}


struct JsContext* JsCreateContext(struct JsEngine* e, struct JsContext* c){
			
	struct JsContext* context;
	JsAssert(e != NULL);
	context = JsCopyContext(c);
	context->engine = e;
	//注册到JsEngine中
	JsContext2Engine(e,context);
	context->thread = NULL;

	return context;	
}

struct JsContext* JsCopyContext(struct JsContext* c){
	struct JsContext* context;
	
	context = (struct JsContext*)JsMalloc(sizeof(struct JsContext));
	context->scope = JsCreateList();
	context->stack = JsCreateList();
	if(c == NULL){
		context->engine = NULL;
		JsListPush(context->scope,JsGetVm()->Global);
		//stack 仅仅进行初始化
		context->pc = NULL;
		context->thisObj = JsGetVm()->Global;
		/*10.2.1*/
		context->varattr = JS_OBJECT_ATTR_DONTDELETE;
		context->thread = NULL;
	}else{
		context->engine = c->engine;
		JsListCopy(context->scope,c->scope);
		JsListCopy(context->stack,c->stack);
		context->pc = c->pc;
		context->thisObj = c->thisObj;
		context->varattr = c->varattr;
		context->thread = c->thread;
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
		if(obj == NULL){
			//当Global调用CreateStandardObject->FindValue的时候
			//obj = NULL
			res->type = JS_UNDEFINED;
			return;
		}
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
		if(obj == NULL){
			//当Global调用CreateStandardObject->FindValue的时候
			//obj = NULL
			res->type = JS_UNDEFINED;
			return;
		}
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

	JsSetTlsValue(key,c);

}
struct JsContext* JsGetTlsContext(){
	struct JsContext* c;
	c = (struct JsContext*)JsGetTlsValue(key);
	return c;
}
static void JsInitContextTlsKey(){
	key = JsCreateTlsKey(NULL);
}