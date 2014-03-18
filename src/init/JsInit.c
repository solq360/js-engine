#include"JsObject.h"
#include"JsContext.h"
#include"JsEngine.h"
#include"JsVm.h"
#include"JsValue.h"
#include"JsList.h"
#include"JsSys.h"
#include"JsDebug.h"
#include"JsError.h"
#include"JsAst.h"
#include"JsParser.h"
#include"JsEval.h"
#include"JsECMAScript.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

static	void JsGlobalEval(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res);

void JsECMAScriptObjectInit(struct JsVm* vm){
	//初始化vm->Global对象
	struct JsObject* Global =  JsCreateStandardObject(NULL);
	vm->Global = Global;	
	
	JsOFInit(vm);	
	JsArrayInit(vm);
	JsStringInit(vm);
	JsBooleanInit(vm);
	JsNumberInit(vm);
	
	
	//配置Global属性
	//NaN
	struct JsValue* v = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	v->type = JS_NUMBER;
	v->u.number = JS_VALUE_NUMBER_NAN;
	(*vm->Global->Put)(vm->Global,"NaN",v,JS_OBJECT_ATTR_STRICT);
	//eval
	v = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	v->type = JS_OBJECT;
	v->u.object = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	v->u.object->Call = &JsGlobalEval;
	(*vm->Global->Put)(vm->Global,"eval",v,JS_OBJECT_ATTR_STRICT);
}
static	void JsGlobalEval(struct JsObject *self, struct JsObject *thisobj, int argc, struct JsValue **argv, struct JsValue *res){

	struct JsObject* saveThisObj;
	int saveVarattr;
	
	if(argc <=0){
		res->type =JS_UNDEFINED;
		return;
	}else  if( argv[0]->type != JS_STRING){
		*res = *argv[0];
		return;
	}
	
	//从线程中获得JsEngine
	struct JsEngine* e = JsGetTlsEngine();
	
	//处理上下文
	//Stack 和 scope 都为当前
	saveThisObj = e->exec->thisObj;
	e->exec->thisObj = (thisobj != NULL ? thisobj : e->jsVm->Global);
	saveVarattr = e->exec->varattr;
	e->exec->varattr = JS_OBJECT_ATTR_DEFAULT;
	
	
	//执行函数
	res->type =JS_UNDEFINED;
	struct JsAstNode * ast;
	JsParseString(e->jsVm->debug ? JS_PARSER_DEBUG_PARSE 
		: JS_PARSER_DEBUG_CLOSE,argv[0]->u.string,&ast);
	if(ast == NULL)
		JsThrowString("SytaxError");
	JsEval(e,ast,res);
	//还原环境
	e->exec->thisObj = saveThisObj;
	e->exec->varattr = saveVarattr;
	
	//结果处理
	if(res->type == JS_COMPLETION){

		if(res->u.completion.type == JS_COMPLETION_RETURN){
			*res = *res->u.completion.value;
		}else if(res->u.completion.type == JS_COMPLETION_THROW){
			//不处理
		}else{
			//JS_COMPLETION_NORMAL, JS_COMPLETION_BREAK, JS_COMPLETION_CONTINUE
			res->type = JS_UNDEFINED;
		}
	}else if(res->type == JS_REFERENCE){
		res->type = JS_UNDEFINED;
	}//res->type = JS_OBJECT, JS_NULL, JS_BOOLEAN ...
	return;

}