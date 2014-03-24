#include"JsObject.h"
#include"JsContext.h"
#include"JsEngine.h"
#include"JsVm.h"
#include"JsValue.h"
#include"JsList.h"
#include"JsSys.h"
#include"JsDebug.h"
#include"JsAst.h"
#include"JsECMAScript.h"
#include"JsException.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#define JS_BOOLEAN_FLOOR 1


//prototype == NULL , 表示使用Object.prototype
static struct JsObject* JsCreateBooleanObject(struct JsObject* prototype,int boolean);

static void JsBooleanFunctionInit(struct JsObject* boolean,struct JsObject* boolean_proto);
static	void JsBooleanConstCall(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res);
static	void JsBooleanConstConstruct(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res);


static void JsBooleanProtoInit(struct JsObject* boolean,struct JsObject* boolean_proto);
static	void JsBooleanProtoToString(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res);
static	void JsBooleanProtoValueOf(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res);

void JsBooleanInit(struct JsVm* vm){
	
	struct JsObject* boolean = JsCreateStandardFunctionObject(NULL,NULL,TRUE);
	struct JsObject* boolean_proto = JsCreateBooleanObject(NULL,FALSE);
	
	JsBooleanFunctionInit(boolean,boolean_proto);
	JsBooleanProtoInit(boolean,boolean_proto);
	struct JsValue* v = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	v->type = JS_OBJECT;
	v->u.object = boolean;
	(*vm->Global->Put)(vm->Global,"Boolean",v,JS_OBJECT_ATTR_STRICT);
}

static struct JsObject* JsCreateBooleanObject(struct JsObject* prototype,int boolean){
	struct JsObject* b = JsAllocObject(JS_BOOLEAN_FLOOR);
	JsCreateStandardObject(b);
	if(prototype != NULL)
		b->Prototype = prototype;
	b->Class = "Boolean";
	
	//初始化自己的Sb
	int* p = (int*)JsMalloc(sizeof(int));
	*p = boolean;
	b->sb[JS_BOOLEAN_FLOOR] = p;
	return b;
}


static void JsBooleanFunctionInit(struct JsObject* boolean,struct JsObject* boolean_proto){
	boolean->Call = &JsBooleanConstCall;
	boolean->Construct = &JsBooleanConstConstruct;
	struct JsValue* v = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	v->type = JS_OBJECT;
	v->u.object = boolean_proto;
	(*boolean->Put)(boolean,"prototype",v,JS_OBJECT_ATTR_STRICT);
}
static	void JsBooleanConstCall(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res){
	if(argc <= 0){
		res->type = JS_BOOLEAN;
		res->u.boolean = FALSE;
		return;
	}
	JsToBoolean(argv[0],res);
}
static	void JsBooleanConstConstruct(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res){
	struct JsValue v;
	struct JsValue v0;
	(*self->Get)(self,"prototype",NULL,&v);
	struct JsObject*  boolean;
	if(argc<=0){
		boolean = JsCreateBooleanObject(v.u.object,FALSE);
	}else{
		JsToBoolean(argv[0],&v0);
		boolean = JsCreateBooleanObject(v.u.object,v0.u.boolean);
	}
	res->type = JS_OBJECT;
	res->u.object = boolean;
}
static void JsBooleanProtoInit(struct JsObject* boolean,struct JsObject* boolean_proto){
	
	struct JsValue* p = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	//Boolean.prototype.constructor
	p= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	p->type = JS_OBJECT;
	p->u.object = boolean;
	(*boolean_proto->Put)(boolean_proto,"constructor",p,JS_OBJECT_ATTR_DONTENUM);
	//toString
	p= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	p->type = JS_OBJECT;
	p->u.object = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	p->u.object->Call = &JsBooleanProtoToString;
	(*boolean_proto->Put)(boolean_proto,"toString",p,JS_OBJECT_ATTR_DONTENUM);
	//valueOf
	p= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	p->type = JS_OBJECT;
	p->u.object = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	p->u.object->Call = &JsBooleanProtoValueOf;
	(*boolean_proto->Put)(boolean_proto,"valueOf",p,JS_OBJECT_ATTR_DONTENUM);
}
static	void JsBooleanProtoToString(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res){
	if(strcmp(thisobj->Class,"Boolean") != 0)
		JsThrowString("TypeError");
	res->type =JS_BOOLEAN;
	res->u.boolean = *(int*)thisobj->sb[JS_BOOLEAN_FLOOR];
}
static	void JsBooleanProtoValueOf(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res){
	if(strcmp(thisobj->Class,"Boolean") != 0)
		JsThrowString("TypeError");
	res->type =JS_STRING;
	res->u.boolean = *(int*)thisobj->sb[JS_BOOLEAN_FLOOR];
}