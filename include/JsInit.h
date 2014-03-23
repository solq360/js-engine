/*
	初始化ECMAScript对象
*/

#ifndef JsInitH
#define JsInitH
#include"JsType.h"
/****************************************************************************
									通用API
*****************************************************************************/
/*
	内置的几种类型Class都不同
		Object
		Function
		Array
		String
		Boolean
		Number
		Math
		Date
		RegExp
		Error
		Activation[10.1.6]
*/
void JsECMAScriptObjectInit(struct JsVm* vm);

#endif