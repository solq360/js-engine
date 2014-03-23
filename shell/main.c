#include"JsObject.h"
#include"JsContext.h"
#include"JsEngine.h"
#include"JsVm.h"
#include"JsList.h"
#include"JsValue.h"
#include"JsSys.h"
#include"JsDebug.h"
#include"JsError.h"
#include"JsAst.h"
#include"JsParser.h"
#include"JsEval.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<setjmp.h>
static char* source = "function a(){ var i =1;}; a(); this.a = 1; this['a']; var c  = [1,2,3,4];";
static void JsContextTask(struct JsEngine* e){
	struct JsAstNode* ast = NULL;
	struct JsValue v;
	JsParseString(JS_PARSER_DEBUG_PARSE,source,&ast);
	JsEval(e,ast,&v);
	printf("hello");
}
int main(){
	JsCreateVm(TRUE,0,NULL, NULL);
	struct JsEngine* e = JsCreateEngine();
	struct JsContext* c = JsCreateContext(e, NULL, &JsContextTask, NULL);
	JsDispatch(c);
	return 0;
}