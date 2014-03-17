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
#include<stdlib.h>
#include<stdio.h>
#include<string.h>


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
	//eval
}