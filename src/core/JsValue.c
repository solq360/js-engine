#include"JsValue.h"
#include"JsContext.h"
#include"JsEngine.h"
#include"JsVm.h"
#include"JsObject.h"
#include"JsDebug.h"
#include"JsError.h"
#include"JsSys.h"
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>



//------------------------
void JsToPrimitive(struct JsValue *val, int type, struct JsValue *res){
					
	if (val->type == JS_OBJECT)
		(*val->u.object->DefaultValue)(val->u.object,type,res);
	else
		*res = *val;
	JsAssert(res->type != JS_OBJECT && res->type != JS_REFERENCE && res->type  != JS_COMPLETION);
}
void JsToBoolean(struct JsValue *val, struct JsValue *res){
	
	res->type  = JS_BOOLEAN;
	switch (val->type) {
	case JS_UNDEFINED:
	case JS_NULL:
		res->u.boolean = FALSE;
		break;
	case JS_BOOLEAN:
		*res  = * val;
		break;
	case JS_NUMBER:
		if (val->u.number == 0 || val->u.number == JS_VALUE_NUMBER_NAN)
			res->u.boolean = FALSE;
		else
			res->u.boolean = TRUE;
		break;
	case JS_STRING:
		if (val->u.string == NULL || strlen(val->u.string) == 0)
			res->u.boolean = FALSE;
		else
			res->u.boolean = TRUE;
		break;
	case JS_OBJECT:
		res->u.boolean = TRUE;
		break;
	default:
		JsPrint("JsValue.c : ToBoolean ERROR\n");
		JsThrowString("JsValue.c : ToBoolean ERROR");
	}
}
void JsToNumber(struct JsValue *val, struct JsValue *res){
	
	res->type = JS_NUMBER;
	switch (val->type) {
	case JS_UNDEFINED:
		res->u.number =  JS_VALUE_NUMBER_NAN;
		break;
	case JS_NULL:
		res->u.number = 0.0;
		break;
	case JS_BOOLEAN:
		res->u.number =  (val->u.boolean ? 1.0 : 0.0);
		break;
	case JS_NUMBER:
		*res = *val;
		break;
	case JS_STRING:{
			char * szOrbits, *pEnd;
			double d1;
			szOrbits = val->u.string;
			d1 = strtod (szOrbits,&pEnd);

			if(*pEnd == '\0'){
				res->u.number = d1;
			}else{
				//如果不是字符串末尾, 则表示没有解析完该字符串
				res->u.number = JS_VALUE_NUMBER_NAN;
			}
			break;
	    }
	case JS_OBJECT:{
			struct JsValue  primitive;
			JsToPrimitive(val,JS_OBJECT_HIT_TYPE_NUMBER,&primitive);
			JsToNumber(&primitive, res);
			break;
	    }
	default:
		JsPrint("JsValue.c : ToNumber ERROR\n");
		JsThrowString("JsValue.c : ToNumber ERROR");
	}

}
void JsToInteger(struct JsValue *val, struct JsValue *res){
	JsToNumber(val, res);
	if(res->type != JS_NUMBER);
		return;
	if (res->u.number == JS_VALUE_NUMBER_NAN)
		res->u.number = 0.0;
	else
		res->u.number = (int)res->u.number;

}

int  JsToInt32(struct JsValue *val){
	return (int)JsToUint32(val);
}
unsigned int JsToUint32(struct JsValue *val){
	
	struct JsValue i;
	JsToInteger(val, &i);
	if (val->type != JS_NUMBER || i.u.number == JS_VALUE_NUMBER_NAN 
		|| i.u.number == 0.0)
		return 0;
	else {
		i.u.number = fmod(i.u.number ,4294967296.0); /* 2^32 */
		if (i.u.number < 0)
			i.u.number += 4294967296;
		return (unsigned int)i.u.number;
	}

}
unsigned short JsToUint16(struct JsValue *val){
	
	struct JsValue i;
	JsToInteger(val, &i);
	if (val->type != JS_NUMBER || i.u.number == JS_VALUE_NUMBER_NAN
		|| i.u.number == 0.0)
		return 0;
	else {
		i.u.number = fmod(i.u.number, 65536.0);	/* 2^16 */
		if (i.u.number < 0)
			i.u.number  +=  65536.0;
		return (unsigned short)i.u.number;
	}

}
void JsToString(struct JsValue *val, struct JsValue *res){
	
	res->type = JS_STRING;
	switch (val->type) {
	case JS_UNDEFINED:
		res->u.string = "undefined";
		break;
	case JS_NULL:
		res->u.string = "null";
		break;
	case JS_BOOLEAN:{
		if(val->u.boolean)
			res->u.string ="false";
		else
			res->u.string = "true";
		break;
	}
	case JS_NUMBER:{					 /* 9.8.1 */
		char* buf;
		double value ;
		value = val->u.number;
		//最大长度为128位
		buf = (char*)JsMalloc(128);
		if(value - (int)value == 0){
			//整数
			sprintf(buf,"%d",(int)value);
		}else{
			sprintf(buf,"%f",value);
		}
		res->u.string = buf;
		break;
	}
	case JS_STRING:
		*res = *val;
		break;
	case JS_OBJECT:{
		struct JsValue prim;
		JsToPrimitive(val,JS_OBJECT_HIT_TYPE_STRING, &prim);
		JsToString(&prim, res);
		break;
	}
	default:
		JsPrint("JsValue.c : ToString ERROR\n");
		JsThrowString("JsValue.c : ToString ERROR");
		
	}

}

void JsToObject(struct JsValue *val, struct JsValue *res){

	struct JsValue *argv[1];
	struct JsValue con0;
	struct JsValue *con;
	argv[0] = val;
	
	res->type = JS_OBJECT;
	switch (val->type) {
	case JS_UNDEFINED:
	case JS_NULL:
		JsThrowString("TypeError");
		break;
	case JS_OBJECT:
		*res = *val;
		break;
	case JS_BOOLEAN:
		JsFindValue(NULL,"Boolean",&con0);
		break;
	case JS_NUMBER:
		JsFindValue(NULL,"Number",&con0);
		break;
	case JS_STRING:
		JsFindValue(NULL,"String",&con0);
		break;
	default:
		JsThrowString("JsValue.c : unknow type");
	}
	//变换
	con = &con0;
	if( con->type != JS_OBJECT || con->u.object == NULL 
		|| con->u.object->Construct == NULL){
			JsPrint("TypeError");
			//JsThrowString("TypeError");
			//直接中断程序
			JsAssert(FALSE);
			return;
	}
	//构造
	(*con->u.object->Construct)(con->u.object,JsGetVm()->Global, 1, argv, res);
}




/* JsReference */
void JsGetValue(struct JsValue* v, struct JsValue* res){

	if (v->type != JS_REFERENCE) {
		*res = *v;
		return;
	}
	if (v->u.reference.base == NULL){
		JsPrint("ReferenceError\n");
		JsThrowString("ReferenceError");
		return;
	}
	//do get
	(*v->u.reference.base->Get)(v->u.reference.base,v->u.reference.name,NULL,res);


}
void JsPutValue(struct JsValue* v,struct JsValue* w,struct JsValue* res){
	struct JsObject *target;
	res->type = JS_BOOLEAN;
	if (v->type != JS_REFERENCE){
		JsPrint("ReferenceError\n");
		JsThrowString("ReferenceError");
	}
	target = v->u.reference.base;
	if (target == NULL)
		target = JsGetVm()->Global;
	(*target->Put)(target, v->u.reference.name, w, JS_OBJECT_ATTR_DEFAULT);
	res->u.boolean = TRUE;
	return;
}
