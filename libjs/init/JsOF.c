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

static void JsObjectInit(struct JsObject* obj,struct JsObject* obj_proto,
		struct JsObject* fun,struct JsObject* fun_proto);	
static void JsObjectProtoInit(struct JsObject* obj,struct JsObject* obj_proto,
		struct JsObject* fun,struct JsObject* fun_proto);	
/*15.2.2.1*/
static void JsObjectCall(struct JsObject *obj, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res);
/*15.2.4.2*/
static void JsObjectProtoToStringCall(struct JsObject *obj, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res);
/*15.2.4.4*/			
static void JsObjectProtoValueOfCall(struct JsObject *obj, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res);			
/*15.2.4.5*/			
static void JsObjectProtoHasOwnPropertyCall(struct JsObject *obj, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res);	
/*15.2.4.6*/
static void JsObjectProtoIsPrototypeOfCall(struct JsObject *obj, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res);	
/*15.2.4.7*/
static void JsObjectProtoPropertyIsEnumerableCall(struct JsObject *obj, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res);

////////////////////////////////////////////////////////////////////////////////
static void JsFunctionInit(struct JsObject* obj,struct JsObject* obj_proto,
		struct JsObject* fun,struct JsObject* fun_proto);
static void JsFunctionProtoInit(struct JsObject* obj,struct JsObject* obj_proto,
		struct JsObject* fun,struct JsObject* fun_proto);	
		
/*15.3.4*/
static void JsFunctionProtoCall(struct JsObject *obj, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res);			
/*15.3.4.2*/
static void JsFunctionProtoToStringCall(struct JsObject *obj, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res);
/*15.3.4.3*/
static void JsFunctionProtoApplyCall(struct JsObject *obj, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res);
/*15.3.4.4*/
static void JsFunctionProtoCallCall(struct JsObject *obj, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res);

/////////////////////////////////////////////////////////////////////////////
void JsOFInit(struct JsVm* vm){
	struct JsValue* vPrototype;
	struct JsObject* obj = JsCreateStandardFunctionObject(NULL,NULL,TRUE);
	struct JsObject* obj_proto = JsCreateStandardObject(NULL);
	
	struct JsObject* fun = JsCreateStandardFunctionObject(NULL,NULL,TRUE);
	//不存在construct和prototype属性
	struct JsObject* fun_proto = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	
	//预先配置他们的prototype属性, 给CreateObject类型函数使用
	vPrototype = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	vPrototype->type = JS_OBJECT;
	vPrototype->u.object = obj_proto;
	(*obj->Put)(obj,"prototype",vPrototype,JS_OBJECT_ATTR_STRICT);
	
	vPrototype = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	vPrototype->type = JS_OBJECT;
	vPrototype->u.object = fun_proto;
	(*fun->Put)(fun,"prototype",vPrototype,JS_OBJECT_ATTR_STRICT);	
	
	//向Global添加Object和Function
	struct JsValue* vObject = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	vObject->type = JS_OBJECT;
	vObject->u.object = obj;
	struct JsValue* vFunction = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	vFunction->type = JS_OBJECT;
	vFunction->u.object = fun;
	(*vm->Global->Put)(vm->Global,"Object",vObject,JS_OBJECT_ATTR_STRICT);
	(*vm->Global->Put)(vm->Global,"Function",vFunction,JS_OBJECT_ATTR_STRICT);
	
	//到现在已经可以正常的使用CreateObject 类型函数, Prototype属性不会缺失.
	//注意 上述四个对象的Prototype属性依旧为NULL, 且obj_proto->Prototype = NULL;
	JsObjectInit(obj,obj_proto,fun,fun_proto);
	JsObjectProtoInit(obj,obj_proto,fun,fun_proto);
	JsFunctionInit(obj,obj_proto,fun,fun_proto);
	JsFunctionProtoInit(obj,obj_proto,fun,fun_proto);
}


//---------------------------------
static void JsObjectInit(struct JsObject* obj,struct JsObject* obj_proto,
		struct JsObject* fun,struct JsObject* fun_proto){
	
	obj->Call = &JsObjectCall;
	obj->Construct = &JsObjectCall;
	
	//Object->Prototype = Function.Prototype;
	obj->Prototype = fun_proto;
	struct JsValue* vLength = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	vLength->type = JS_NUMBER;
	vLength->u.number = 1;
	(*obj->Put)(obj,"length",vLength,JS_OBJECT_ATTR_STRICT);
}

static void JsObjectProtoInit(struct JsObject* obj,struct JsObject* obj_proto,
		struct JsObject* fun,struct JsObject* fun_proto){
	struct JsValue* vProperty ;
	//constructor
	vProperty= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	vProperty->type = JS_OBJECT;
	vProperty->u.object = obj;
	(*obj_proto->Put)(obj_proto,"constructor",vProperty,JS_OBJECT_ATTR_DONTENUM);
	//toString
	vProperty= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	vProperty->type = JS_OBJECT;
	vProperty->u.object = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	vProperty->u.object->Call = &JsObjectProtoToStringCall;
	(*obj_proto->Put)(obj_proto,"toString",vProperty,JS_OBJECT_ATTR_DONTENUM);
	//valueOf
	vProperty= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	vProperty->type = JS_OBJECT;
	vProperty->u.object = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	vProperty->u.object->Call = &JsObjectProtoValueOfCall;
	(*obj_proto->Put)(obj_proto,"valueOf",vProperty,JS_OBJECT_ATTR_DONTENUM);
	//hasOwnProperty
	vProperty= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	vProperty->type = JS_OBJECT;
	vProperty->u.object = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	vProperty->u.object->Call = &JsObjectProtoHasOwnPropertyCall;
	(*obj_proto->Put)(obj_proto,"hasOwnProperty",vProperty,JS_OBJECT_ATTR_DONTENUM);
	//isPrototypeOf
	vProperty= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	vProperty->type = JS_OBJECT;
	vProperty->u.object = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	vProperty->u.object->Call = &JsObjectProtoIsPrototypeOfCall;
	(*obj_proto->Put)(obj_proto,"isPrototypeOf",vProperty,JS_OBJECT_ATTR_DONTENUM);
	//propertyIsEnumerable
	vProperty= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	vProperty->type = JS_OBJECT;
	vProperty->u.object = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	vProperty->u.object->Call = &JsObjectProtoPropertyIsEnumerableCall;
	(*obj_proto->Put)(obj_proto,"propertyIsEnumerable",vProperty,JS_OBJECT_ATTR_DONTENUM);
}
//------------------------------Object.prototype------------------------			
/*15.2.2.1*/
static void JsObjectCall(struct JsObject *obj, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res){
	if(argc > 0){
		if(argv[0]->type == JS_OBJECT){
			//TODO rule4
			*res = *argv[0];
			return;
		}
		if(argv[0]->type == JS_STRING ||argv[0]->type == JS_BOOLEAN 
			|| argv[0]->type == JS_NUMBER){
			JsToObject(argv[0],res);
			return;
		}
	}
	struct JsObject* t = JsCreateStandardObject(NULL);
	res->type  = JS_OBJECT;
	res->u.object = t;
	return;
}
/*15.2.4.2*/
static void JsObjectProtoToStringCall(struct JsObject *obj, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res){
	const char* Class = thisobj->Class;
	int size = strlen(Class);
	char* buf =(char*)JsMalloc(size+12);
	sprintf(buf,"[object %s]",Class);
	res->type = JS_STRING;
	res->u.string = buf;
}
/*15.2.4.4*/
static void JsObjectProtoValueOfCall(struct JsObject *obj, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res){
	res->type = JS_OBJECT;
	res->u.object = thisobj == NULL?  JsGetVm()->Global : thisobj;
}

/*15.2.4.5*/			
static void JsObjectProtoHasOwnPropertyCall(struct JsObject *obj, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res){
	struct JsValue v;
	char* prop = "undefined";
	
	if(argc >0 ){
		JsToString(argv[0],&v);
		(*thisobj->HasOwnProperty)(thisobj,v.u.string,res);
		prop = v.u.string;
	}
	(*thisobj->HasOwnProperty)(thisobj,prop,res);
}
/*15.2.4.6*/
static void JsObjectProtoIsPrototypeOfCall(struct JsObject *obj, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res){
	struct JsObject* prototype;
	
	res->type = JS_BOOLEAN;
	if(argc <= 0){
		res->u.boolean = FALSE;
	}
	if(argv[0]->type != JS_OBJECT){
		res->type = JS_BOOLEAN;
		res->u.boolean = FALSE;
		return ;
	}
	prototype = argv[0]->u.object->Prototype;
	//循环查询
	while(prototype != NULL){
		if(prototype == thisobj){
			res->type = JS_BOOLEAN;
			res->u.boolean = TRUE;
		}
		//下一个对象的原形
		prototype = prototype->Prototype;
	}
	//没有查询到
	res->type = JS_BOOLEAN;
	res->u.boolean = FALSE;
}
/*15.2.4.7*/
static void JsObjectProtoPropertyIsEnumerableCall(struct JsObject *obj, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res){

	res->type = JS_BOOLEAN;
	char* prop = "undefined";
	int flag;
	struct JsValue v;
	//prop查询
	if(argc >= 0){
		JsToString(argv[0],&v);
		prop = v.u.string;
	}
	
	(*thisobj->HasOwnProperty)(thisobj,prop,&v);
	if(v.u.boolean == TRUE){
		//存在属性
		(*thisobj->Get)(thisobj,prop,&flag,&v);
		if(v.type == JS_UNDEFINED)
			res->u.boolean = FALSE;
		else
			res->u.boolean = ((flag & JS_OBJECT_ATTR_DONTENUM) == 0);
	}
	//该对象不存在该属性
	res->u.boolean = FALSE;
}



static void JsFunctionInit(struct JsObject* obj,struct JsObject* obj_proto,
		struct JsObject* fun,struct JsObject* fun_proto){
	//Call 和 Constructor 没有写
	fun->Prototype = fun_proto;
	struct JsValue* vLength = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	vLength->type = JS_NUMBER;
	vLength->u.number = 0; //default 1
	(*fun->Put)(fun,"length",vLength,JS_OBJECT_ATTR_STRICT);
}
static void JsFunctionProtoInit(struct JsObject* obj,struct JsObject* obj_proto,
		struct JsObject* fun,struct JsObject* fun_proto){
	struct JsValue* vProperty ;
	fun_proto->Prototype = obj_proto;	
	fun_proto->Call = &JsFunctionProtoCall;
	//constructor
	vProperty= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	vProperty->type = JS_OBJECT;
	vProperty->u.object = fun;
	(*fun_proto->Put)(fun_proto,"constructor",vProperty,JS_OBJECT_ATTR_DONTENUM);
	//toString
	vProperty= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	vProperty->type = JS_OBJECT;
	vProperty->u.object = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	vProperty->u.object->Call = &JsFunctionProtoToStringCall;
	(*fun_proto->Put)(fun_proto,"toString",vProperty,JS_OBJECT_ATTR_DONTENUM);
	//apply
	vProperty= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	vProperty->type = JS_OBJECT;
	vProperty->u.object = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	vProperty->u.object->Call = &JsFunctionProtoApplyCall;
	(*fun_proto->Put)(fun_proto,"apply",vProperty,JS_OBJECT_ATTR_DONTENUM);
	//call
	vProperty= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	vProperty->type = JS_OBJECT;
	vProperty->u.object = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	vProperty->u.object->Call = &JsFunctionProtoCallCall;
	(*fun_proto->Put)(fun_proto,"call",vProperty,JS_OBJECT_ATTR_DONTENUM);
}



//-----------------------------------Function.prototype----------------------

/*15.3.4*/
static void JsFunctionProtoCall(struct JsObject *obj, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res){
	//empty function
	res->type =JS_UNDEFINED;
}
/*15.3.4.2*/
static void JsFunctionProtoToStringCall(struct JsObject *obj, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res){
	//根据this不同类型,  返回不同的String内容, 这里简单的放回一个空String
	res->type =JS_STRING;
	res->u.string = "function Empty(){}";
}
/*15.3.4.3*/
static void JsFunctionProtoApplyCall(struct JsObject *obj, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res){
	struct JsObject* this0;
	int argc0;
	struct JsValue** argv0;
	if(thisobj == NULL){
		JsThrowString("null point this");
	}
	if(thisobj->Call == NULL){
		JsThrowString("TypeError");
		return;
	}
	if(argc == 0){
		this0 = JsGetVm()->Global;
		argc0 = 0;
		argv0 = NULL;
	}else if(argc == 1){
		if(argv[0]->type == JS_NULL || argv[0]->type == JS_UNDEFINED){
			this0 = JsGetVm()->Global;
		}else{
			struct JsValue o;
			JsToObject(argv[0],&o);
			this0 = o.u.object;
		}
		argc0 = 0;
		argv0 = NULL;
	}else if(argc >= 2){
		//this
		if(argv[0]->type == JS_NULL || argv[0]->type == JS_UNDEFINED){
			this0 = JsGetVm()->Global;
		}else{
			struct JsValue o;
			JsToObject(argv[0],&o);
			this0 = o.u.object;
		}
		//argc argv
		if(argv[1]->type == JS_NULL || argv[1]->type == JS_UNDEFINED){
			argc0 = 0;
			argv0 = NULL;
		}else{
			//测试argv[1]是否为OBJECT并且为Array类型
			if(argv[1]->type != JS_OBJECT || strcmp(argv[1]->u.object->Class,"Array") !=0 ){
				JsThrowString("TypeError");
				return;
			}
			struct JsValue v ;
			(*argv[1]->u.object->Get)(argv[1]->u.object,"length",NULL,&v);
			if(v.type != JS_NUMBER){
				JsThrowString("TypeError");
				return;
			}
			argc0 = v.u.number;
			argv0 = (struct JsValue**)JsMalloc(sizeof(struct JsValue*)* argc0);
			//计算最大位数
			int number = argc0;
			int size = 0;
			int i = 0;
			while(number){
				number /=10;
				size++;
			}
			char* buf = (char*)JsMalloc(size + 4);
			//组建Argv0
			for(i =0 ; i < argc0;++i){
				sprintf(buf,"%d",i);
				(*argv[1]->u.object->Get)(argv[1]->u.object,buf,NULL,&v);
				argv0[i] =  (struct JsValue*)JsMalloc(sizeof(struct JsValue));
				*argv0[i] = v;
			}
		}
	}else{
		//argc大小错误
		JsAssert(FALSE);
	}
	(*thisobj->Call)(thisobj,this0,argc0,argv0,res);
}
			
/*15.3.4.4*/
static void JsFunctionProtoCallCall(struct JsObject *obj, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res){
	struct JsObject* this0;
	int argc0;
	struct JsValue** argv0;
	if(thisobj == NULL){
		JsThrowString("null point this");
		return;
	}
	if(thisobj->Call == NULL){
		JsThrowString("TypeError");
		return;
	}
	if(argc == 0){
		this0 = JsGetVm()->Global;
		argc0 = 0;
		argv0 = NULL;
	}else if(argc == 1){
		if(argv[0]->type == JS_NULL || argv[0]->type == JS_UNDEFINED){
			this0 = JsGetVm()->Global;
		}else{
			struct JsValue o;
			JsToObject(argv[0],&o);
			this0 = o.u.object;
		}
		argc0 = 0;
		argv0 = NULL;
	}else if(argc >= 2){
		//this
		if(argv[0]->type != JS_NULL && argv[0]->type != JS_UNDEFINED){
			this0 = JsGetVm()->Global;
		}else{
			struct JsValue o;
			JsToObject(argv[0],&o);
			this0 = o.u.object;
		}
		argc0 = argc - 1;
		argv0 = &argv[1];
	}else{
		//argc大小错误
		JsAssert(FALSE);
	}
	(*thisobj->Call)(thisobj,this0,argc0,argv0,res);	
}