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
	char* s  =NULL;
	if(JsGetVm()->debug){
		switch(e->type){
			case JS_UNDEFINED: s= "undefined";break;
			case JS_NULL : s= "null";break;
			case JS_NUMBER : s= "number";break;
			case JS_STRING: s = e->u.string;break;
			case JS_OBJECT: s = "object";break;
			case JS_BOOLEAN: s = "boolean";break;
			default:
				s = "unknow";
		}
		printf("%s\n",s);
	}
}
void JsAssert(int v){
	assert(v);
}