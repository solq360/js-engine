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

#define JS_NUMBER_FLOOR 1


//prototype == NULL , 表示使用Object.prototype

static struct JsObject* JsCreateNumberObject(struct JsObject* prototype,double number);

static void JsNumberFunctionInit(struct JsObject* number,struct JsObject* number_proto);
static	void JsNumberConstCall(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res);
static	void JsNumberConstConstruct(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res);


static void JsNumberProtoInit(struct JsObject* number,struct JsObject* number_proto);
static	void JsNumberProtoToString(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res);
static	void JsNumberProtoValueOf(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res);

void JsNumberInit(struct JsVm* vm){
	
	struct JsObject* number = JsCreateStandardFunctionObject(NULL,NULL,TRUE);
	struct JsObject* number_proto = JsCreateNumberObject(NULL,0);
	
	JsNumberFunctionInit(number,number_proto);
	JsNumberProtoInit(number,number_proto);
	struct JsValue* v = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	v->type = JS_OBJECT;
	v->u.object = number;
	(*vm->Global->Put)(vm->Global,"Number",v,JS_OBJECT_ATTR_STRICT);
}

static struct JsObject* JsCreateNumberObject(struct JsObject* prototype,double number){
	struct JsObject* b = JsAllocObject(JS_NUMBER_FLOOR);
	JsCreateStandardObject(b);
	if(prototype != NULL)
		b->Prototype = prototype;
	b->Class = "Number";
	
	//初始化自己的Sb
	double* p = (double*)JsMalloc(sizeof(double));
	*p = number;
	b->sb[JS_NUMBER_FLOOR] = p;
	return b;
}


static void JsNumberFunctionInit(struct JsObject* number,struct JsObject* number_proto){
	number->Call = &JsNumberConstCall;
	number->Construct = &JsNumberConstConstruct;
	//prototype
	struct JsValue* v = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	v->type = JS_OBJECT;
	v->u.object = number_proto;
	(*number->Put)(number,"prototype",v,JS_OBJECT_ATTR_STRICT);
	//NaN
	v = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	v->type = JS_NUMBER;
	v->u.number = JS_VALUE_NUMBER_NAN;
	(*number->Put)(number,"NaN",v,JS_OBJECT_ATTR_STRICT);
}
static	void JsNumberConstCall(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res){
	if(argc <= 0){
		res->type = JS_NUMBER;
		res->u.number = 0.0;
		return;
	}
	JsToNumber(argv[0],res);
}
static	void JsNumberConstConstruct(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res){
	struct JsValue v;
	struct JsValue v0;
	(*self->Get)(self,"prototype",NULL,&v);
	struct JsObject*  number;
	if(argc<=0){
		number = JsCreateNumberObject(v.u.object,0);
	}else{
		JsToNumber(argv[0],&v0);
		number = JsCreateNumberObject(v.u.object,v0.u.number);
	}
	res->type = JS_OBJECT;
	res->u.object = number;
}
static void JsNumberProtoInit(struct JsObject* number,struct JsObject* number_proto){
	
	struct JsValue* p = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	//Number.prototype.constructor
	p= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	p->type = JS_OBJECT;
	p->u.object = number;
	(*number_proto->Put)(number_proto,"constructor",p,JS_OBJECT_ATTR_DONTENUM);
	//toString
	p= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	p->type = JS_OBJECT;
	p->u.object = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	p->u.object->Call = &JsNumberProtoToString;
	(*number_proto->Put)(number_proto,"toString",p,JS_OBJECT_ATTR_DONTENUM);
	//valueOf
	p= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	p->type = JS_OBJECT;
	p->u.object = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	p->u.object->Call = &JsNumberProtoValueOf;
	(*number_proto->Put)(number_proto,"valueOf",p,JS_OBJECT_ATTR_DONTENUM);
}
static	void JsNumberProtoToString(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res){
	if(strcmp(thisobj->Class,"Number") != 0)
		JsThrowString("TypeError");
	res->type =JS_NUMBER;
	res->u.number = *(double*)thisobj->sb[JS_NUMBER_FLOOR];
}
static	void JsNumberProtoValueOf(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res){
	if(strcmp(thisobj->Class,"Number") != 0)
		JsThrowString("TypeError");
	res->type =JS_NUMBER;
	res->u.number = *(double*)thisobj->sb[JS_NUMBER_FLOOR];
}