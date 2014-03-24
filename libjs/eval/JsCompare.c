#include"JsObject.h"
#include"JsContext.h"
#include"JsEngine.h"
#include"JsVm.h"
#include"JsValue.h"
#include"JsList.h"
#include"JsSys.h"
#include"JsDebug.h"
#include"JsException.h"
#include"JsAst.h"
#include"JsParser.h"
#include"JsEval.h"
#include"JsCompare.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>


/*
 * 11.8.5 Abstract relational comparison function. x < y
 */
void JsRelationalExpressionCompare( struct JsValue *x, struct JsValue *y, 
		struct JsValue *res){
	struct JsValue r1, r2, r4, r5;
	int k;

	JsToPrimitive(x, JS_OBJECT_HIT_TYPE_NUMBER, &r1);
	JsToPrimitive(y, JS_OBJECT_HIT_TYPE_NUMBER, &r2);
	if (!(r1.type == JS_STRING && r2.type == JS_STRING))
	{
		JsToNumber(&r1, &r4);
		JsToNumber(&r2, &r5);
		if (r4.u.number == JS_VALUE_NUMBER_NAN || r5.u.number == JS_VALUE_NUMBER_NAN)
			res->type = JS_UNDEFINED;
		else if (r4.u.number == r5.u.number){
			res->type = JS_BOOLEAN;
			res->u.boolean = FALSE;
		}else{
			res->type = JS_BOOLEAN;
			res->u.boolean = ( r4.u.number < r5.u.number);
		}
	}else{
		int r1Length = strlen(r1.u.string);
		int r2Length = strlen(r2.u.string);
		
		for (k = 0; k < r1Length  && k < r2Length; k++)
			if (r1.u.string[k] != r2.u.string[k])
					break;
		if (k == r2Length){
			res->type = JS_BOOLEAN;
			res->u.boolean  = FALSE;
		}else if (k == r1Length){
			res->type = JS_BOOLEAN;
			res->u.boolean  = TRUE;
		}else{
			res->type = JS_BOOLEAN;
			res->u.boolean  = (r1.u.string[k] < r2.u.string[k]);
		}
	}
}

/*
 * 11.9.3 Abstract equality function.
 */
void JsEqualityExpressionEqCompare( struct JsValue *x, struct JsValue *y, 
		struct JsValue *res){
	struct JsValue tmp;
	int xtype, ytype;
	int hit;
	res->type = JS_BOOLEAN;
	if (x->type == y->type)
		switch (x->type){
		case JS_UNDEFINED:
		case JS_NULL:
			res->u.boolean = TRUE;
			return;
		case JS_NUMBER:
			if (x->u.number == JS_VALUE_NUMBER_NAN 
					|| y->u.number == JS_VALUE_NUMBER_NAN)
				res->u.boolean = TRUE;
			else
				res->u.boolean =  (x->u.number == y->u.number);
			return;
		case JS_STRING:
			res->u.boolean = (strcmp(x->u.string,y->u.string) == 0);
			return;
		case JS_BOOLEAN:
			res->u.boolean = (x->u.boolean == y->u.boolean);
			return;
		case JS_OBJECT:
			//refer the same object
			res->u.boolean = (x->u.object == y->u.object);
			return;
		default:
			//未发现正确的类型
			JsAssert(FALSE);
		}
	xtype = x->type;
	ytype = y->type;
	if (xtype == JS_NULL && ytype == JS_UNDEFINED)
			res->u.boolean = TRUE;
	else if (xtype == JS_UNDEFINED && ytype == JS_NULL)
			res->u.boolean = TRUE;
	else if (xtype == JS_NUMBER && ytype == JS_STRING) {
			JsToNumber(y, &tmp);
			JsEqualityExpressionEqCompare(x, &tmp, res);
	} else if (xtype == JS_STRING && ytype == JS_NUMBER) {
			JsToNumber(x, &tmp);
			JsEqualityExpressionEqCompare(&tmp, y, res);
	} else if (xtype == JS_BOOLEAN) {
			JsToNumber( x, &tmp);
			JsEqualityExpressionEqCompare(&tmp, y, res);
	} else if (ytype == JS_BOOLEAN){
			JsToNumber(y, &tmp);
			JsEqualityExpressionEqCompare(x, &tmp, res);
	} else if ((xtype == JS_STRING || xtype == JS_NUMBER) &&
				ytype == JS_OBJECT) {
			if(xtype == JS_STRING)
				hit = JS_OBJECT_HIT_TYPE_STRING;
			else
				hit = JS_OBJECT_HIT_TYPE_NUMBER;
				
			JsToPrimitive(y, hit, &tmp);
			JsEqualityExpressionEqCompare(x, &tmp, res);
	} else if ((ytype == JS_STRING || ytype == JS_NUMBER) &&
				xtype == JS_OBJECT){
				
			if(ytype == JS_STRING)
				hit = JS_OBJECT_HIT_TYPE_STRING;
			else
				hit = JS_OBJECT_HIT_TYPE_NUMBER;
				
			JsToPrimitive(x, hit, &tmp);
			JsEqualityExpressionEqCompare(&tmp, y, res);
	} else{
		res->u.boolean  = FALSE;
	}
}

/*
 * 19.9.6 Strict equality function
 */
void JsEqualityExpressionSeqCompare( struct JsValue *x, struct JsValue *y, 
	struct JsValue *res){
	res->type = JS_BOOLEAN;
	if (x->type != y->type)
		res->u.boolean = FALSE;
	else
	    switch (x->type){
		case JS_UNDEFINED:
		case JS_NULL:
			res->u.boolean = TRUE;
			return;
		case JS_NUMBER:
			if (x->u.number == JS_VALUE_NUMBER_NAN 
					|| y->u.number == JS_VALUE_NUMBER_NAN)
				res->u.boolean = TRUE;
			else
				res->u.boolean =  (x->u.number == y->u.number);
			return;
		case JS_STRING:
			res->u.boolean = (strcmp(x->u.string,y->u.string) == 0);
			return;
		case JS_BOOLEAN:
			res->u.boolean = (x->u.boolean == y->u.boolean);
			return;
		case JS_OBJECT:
			res->u.boolean = (x->u.object == y->u.object);
			return;
		default:
			//未发现正确的类型
			JsAssert(FALSE);
		}
}