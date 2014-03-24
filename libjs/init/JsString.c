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

#define JS_STRING_FLOOR 1
//prototype == NULL , 表示使用Object.prototype
//str == NULL , 表示sb = ""
static struct JsObject* JsCreateStringObject(struct JsObject* prototype,char* str);

static void JsStringFunctionInit(struct JsObject* str,struct JsObject* str_proto);
static	void JsStringConstCall(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res);
static	void JsStringConstConstruct(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res);


static void JsStringProtoInit(struct JsObject* str,struct JsObject* str_proto);
static	void JsStringProtoToString(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res);
static	void JsStringProtoValueOf(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res);
static	void JsStringProtoCharAt(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res);
static	void JsStringProtoConcat(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res);


static	void JsStringInstGet(struct JsObject *self, char *prop,int* attr, struct JsValue *res);



void JsStringInit(struct JsVm* vm){
	
	struct JsObject* str = JsCreateStandardFunctionObject(NULL,NULL,TRUE);
	struct JsObject* str_proto = JsCreateStringObject(NULL,NULL);
	
	JsStringFunctionInit(str,str_proto);
	JsStringProtoInit(str,str_proto);
	struct JsValue* v = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	v->type = JS_OBJECT;
	v->u.object = str;
	(*vm->Global->Put)(vm->Global,"String",v,JS_OBJECT_ATTR_STRICT);
}

static struct JsObject* JsCreateStringObject(struct JsObject* prototype,char* str){
	struct JsObject* string = JsAllocObject(JS_STRING_FLOOR);
	JsCreateStandardObject(string);
	if(prototype != NULL)
		string->Prototype = prototype;
	string->Class = "String";
	string->Get = &JsStringInstGet;
	if(str == NULL)
		str = "";
	//初始化自己的Sb
	char* p = (char*)JsMalloc(strlen(str)+4);
	strcpy(p,str);
	string->sb[JS_STRING_FLOOR] = p;
	
	
	//length
	struct JsValue* vLength = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	vLength->type = JS_NUMBER;
	vLength->u.number = strlen(str);
	(*string->Put)(string,"length",vLength,JS_OBJECT_ATTR_STRICT);
	return string;
	
}


static void JsStringFunctionInit(struct JsObject* str,struct JsObject* str_proto){
	str->Call = &JsStringConstCall;
	str->Construct = &JsStringConstConstruct;
	struct JsValue* v = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	v->type = JS_OBJECT;
	v->u.object = str_proto;
	(*str->Put)(str,"prototype",v,JS_OBJECT_ATTR_STRICT);
}
static	void JsStringConstCall(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res){
	if(argc <= 0){
		res->type = JS_STRING;
		res->u.string ="";
		return;
	}
	JsToString(argv[0],res);
}
static	void JsStringConstConstruct(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res){
	struct JsValue v;
	struct JsValue v0;
	(*self->Get)(self,"prototype",NULL,&v);
	struct JsObject*  string;
	if(argc<=0){
		string = JsCreateStringObject(v.u.object,NULL);
	}else{
		JsToString(argv[0],&v0);
		string = JsCreateStringObject(v.u.object,v0.u.string);
	}
	res->type = JS_OBJECT;
	res->u.object = string;
}

static void JsStringProtoInit(struct JsObject* str,struct JsObject* str_proto){
	
	struct JsValue* p = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	//String.prototype.constructor
	p= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	p->type = JS_OBJECT;
	p->u.object = str;
	(*str_proto->Put)(str_proto,"constructor",p,JS_OBJECT_ATTR_DONTENUM);
	//toString
	p= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	p->type = JS_OBJECT;
	p->u.object = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	p->u.object->Call = &JsStringProtoToString;
	(*str_proto->Put)(str_proto,"toString",p,JS_OBJECT_ATTR_DONTENUM);
	//valueOf
	p= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	p->type = JS_OBJECT;
	p->u.object = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	p->u.object->Call = &JsStringProtoValueOf;
	(*str_proto->Put)(str_proto,"valueOf",p,JS_OBJECT_ATTR_DONTENUM);
	//CharAt
	p= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	p->type = JS_OBJECT;
	p->u.object = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	p->u.object->Call = &JsStringProtoCharAt;
	(*str_proto->Put)(str_proto,"charAt",p,JS_OBJECT_ATTR_DONTENUM);
	//Concat
	p= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	p->type = JS_OBJECT;
	p->u.object = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	p->u.object->Call = &JsStringProtoConcat;
	(*str_proto->Put)(str_proto,"concat",p,JS_OBJECT_ATTR_DONTENUM);

}
static	void JsStringProtoToString(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res){
	//只有对应的对象类 String 才能使用该函数
	if(strcmp(thisobj->Class,"String") != 0)
		JsThrowString("TypeError");
	res->type =JS_STRING;
	res->u.string = (char*)thisobj->sb[JS_STRING_FLOOR];
}
static	void JsStringProtoValueOf(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res){
	if(strcmp(thisobj->Class,"String") != 0)
		JsThrowString("TypeError");
	res->type =JS_STRING;
	res->u.string = (char*)thisobj->sb[JS_STRING_FLOOR];
}

static	void JsStringProtoCharAt(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res){
	struct JsValue v;
	struct JsValue pos;
	struct JsValue shell;
	
	shell.type = JS_OBJECT;
	shell.u.object = thisobj;
	JsToString(&shell,&v);
	
	if(argv <= 0){
		pos.u.number = 0;
	}else{
		JsToInteger(argv[0],&pos);
	}
	int length  =strlen(v.u.string);
	if(pos.u.number < 0 ||  pos.u.number >= length){
		//return empty
		res->type = JS_STRING;
		res->u.string = "";
	}else{
		res->type =JS_STRING;
		res->u.string = (char*)JsMalloc(4);
		res->u.string[0] = v.u.string[(int)(pos.u.number - 1)];
		res->u.string[1] = '\0';
	}
}
static	void JsStringProtoConcat(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res){
	struct JsValue v;
	struct JsValue r;
	struct JsValue shell;
	shell.type = JS_OBJECT;
	shell.u.object =thisobj;
	JsToString(&shell,&v);
	r.type = JS_STRING;
	int sizeOfMem = strlen(v.u.string)+4;
	r.u.string = (char*)JsMalloc(sizeOfMem);
	strcpy(r.u.string,v.u.string);
	int i;
	
	for(i=0;i< argc;++i){
		JsToString(argv[i],&v);
		sizeOfMem += strlen(v.u.string);
		r.u.string = (char*)JsReAlloc(r.u.string,sizeOfMem);
		strcat(r.u.string,v.u.string);
	}
	*res = r;
}




//String->sb[1] = char*
static	void JsStringInstGet(struct JsObject *self, char *prop,int* attr, struct JsValue *res){
	char* pEnd;
	long index = strtol(prop,&pEnd,10);
	if(*pEnd == '\0'){
		char* p = self->sb[JS_STRING_FLOOR];
		if(index >= strlen(p)){
			res->type = JS_STRING;
			res->u.string = "";
			return;
		}
		res->type = JS_STRING;
		
		res->u.string = (char*)JsMalloc(4);
		res->u.string[0] = p[index];
		res->u.string[1] = '\0';
		*attr = JS_OBJECT_ATTR_DEFAULT;
		return;
	}
	JsStandardGet(self,prop,attr,res);
}