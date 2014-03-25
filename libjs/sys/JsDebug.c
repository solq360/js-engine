#include"JsDebug.h"
#include"JsEngine.h"
#include"JsContext.h"
#include"JsValue.h"
#include"JsObject.h"
#include"JsVm.h"
#include"JsList.h"
#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<assert.h>
void JsPrintString(char* fmt,...){

	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

}
void JsPrintValue(struct JsValue* v){

	switch(v->type){
	case JS_NULL:{
		printf("null");
		break;
	}
	case JS_UNDEFINED:{
		printf("undefined");
		break;
	}
	case JS_BOOLEAN:{
		if(v->u.boolean == TRUE){
			printf("true");
		}else{
			printf("false");
		}
		break;
	}
	case JS_NUMBER:{
		printf("%lf",v->u.number);
		break;
	}
	case JS_STRING:{
		printf("%s",v->u.string);
		break;
	}
	case JS_OBJECT:{
		struct JsValue str;
		JsToString(v,&str);
		printf("%s",str.u.string);
		break;
	}
	default:
		printf("unknow type");
	}
	printf("\n");
}
void JsPrintStack(JsList stack){
	JsAssert(stack != NULL);
	int size = JsListSize(stack);
	int i;
	printf("*******Stack******\n");
	for(i = size -1  ; i >= 0 ; --i){
		struct JsLocation* l = JsListGet(stack,i);
		printf("[%s : %d] \n",l->filename,l->lineno);
	}
	printf("******************\n");
}
void JsAssert(int v){
	assert(v);
}