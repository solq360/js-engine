#include"JsDebug.h"
#include"JsEngine.h"
#include"JsContext.h"
#include"JsValue.h"
#include"JsObject.h"
#include"JsVm.h"
#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<assert.h>
void JsPrint(char* fmt,...){
	
	if(JsGetVm()->debug){
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}
}
void JsPrintError(struct JsValue* e){

	if(JsGetVm()->debug == FALSE)
		return;
	switch(e->type){
	case JS_NULL:{
		printf("null");
		break;
	}
	case JS_UNDEFINED:{
		printf("undefined");
		break;
	}
	case JS_BOOLEAN:{
		if(e->u.boolean == TRUE){
			printf("true");
		}else{
			printf("false");
		}
		break;
	}
	case JS_NUMBER:{
		printf("%lf",e->u.number);
		break;
	}
	case JS_STRING:{
		printf("%s",e->u.string);
		break;
	}
	case JS_OBJECT:{
		struct JsValue str;
		JsToString(e,&str);
		printf("%s",str.u.string);
		break;
	}
	default:
		printf("unknow type");
	}
	printf("\n");
}
void JsAssert(int v){
	assert(v);
}