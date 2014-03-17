#include"JsDebug.h"
#include"JsEngine.h"
#include"JsContext.h"
#include"JsVm.h"
#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<assert.h>
void JsPrintf(char* fmt,...){
	
	if(JsGetVm()->debug){
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}
}
void JsAssert(int v){
	assert(v);
}