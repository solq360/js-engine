#include"JsObject.h"
#include"JsContext.h"
#include"JsEngine.h"
#include"JsVm.h"
#include"JsList.h"
#include"JsValue.h"
#include"JsSys.h"
#include"JsDebug.h"
#include"JsException.h"
#include"JsAst.h"
#include"JsParser.h"
#include"JsEval.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<setjmp.h>


static void JsContextTask(struct JsEngine* e){
	struct JsAstNode* ast = NULL;
	struct JsValue v;
	ast = JsParseFile(JS_PARSER_DEBUG_ERROR,"../file.js");
	JsAssert(ast != NULL);
	JsEval(e,ast,&v);
}
//一个参数value
static void JsPrintFn(struct JsEngine* e,void* data,struct JsValue* res){
	struct JsValue v;
	struct JsContext* c = JsGetTlsContext();
	if(c ==  NULL)
		return;
	JsFindValue(c,"value",&v);
	switch(v.type){
	case JS_NULL:{
		printf("null");
		break;
	}
	case JS_UNDEFINED:{
		printf("undefined");
		break;
	}
	case JS_BOOLEAN:{
		if(v.u.boolean == TRUE){
			printf("true");
		}else{
			printf("false");
		}
		break;
	}
	case JS_NUMBER:{
		printf("%lf",v.u.number);
		break;
	}
	case JS_STRING:{
		printf("%s",v.u.string);
		break;
	}
	case JS_OBJECT:{
		struct JsValue str;
		JsToString(&v,&str);
		printf("%s",str.u.string);
		break;
	}
	default:
		printf("unknow type");
	}
	printf("\n");
}
static void CreatePrintFn(){
	char** argv = JsMalloc(sizeof(char*) * 1);
	argv[0] = "value";
	//创建PrintFunction
	struct JsObject* print = JsCreateStandardSpecFunction(NULL,NULL,0,1, 
		argv,NULL,&JsPrintFn,"print",FALSE);
	struct JsValue* vPrint = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	vPrint->type = JS_OBJECT;
	vPrint->u.object = print;
	(*JsGetVm()->Global->Put)(JsGetVm()->Global,"print",vPrint,JS_OBJECT_ATTR_STRICT);
}
int main(){
	JsCreateVm(TRUE,0,NULL, NULL);
	struct JsEngine* e = JsCreateEngine();
	struct JsContext* c = JsCreateContext(e, NULL, &JsContextTask, NULL);
	CreatePrintFn();
	JsDispatch(c);
	return 0;
}