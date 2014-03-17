#ifndef JsValueH
#define JsValueH

#include"JsType.h"

struct JsReference;
struct JsCompletion;
struct JsValue;

struct JsReference {
	struct JsObject *base;
	char *name;
};

struct JsCompletion {
	struct JsValue *value;	/* Return value */
	enum JsCompletionEnum{ 
			JS_COMPLETION_NORMAL,
			JS_COMPLETION_BREAK, 
			JS_COMPLETION_CONTINUE, 
			JS_COMPLETION_RETURN,
			//因为使用Exception机制, 所以该类型基本不被使用
			JS_COMPLETION_THROW 
	} type;
};

struct JsValue{
	enum JsValueEnum{
		JS_UNDEFINED,
		JS_NULL,
		JS_BOOLEAN,
		JS_NUMBER,
		JS_STRING,
		JS_OBJECT,
		JS_REFERENCE,			/* internal type (8.7) */
		JS_COMPLETION			/* internal type (8.9) */
	} type;
	union {
		double	  			number;
		int	      			boolean;
		struct JsObject* 	object;
		char*				string;
		/* The following members are not part of the public API */
		struct JsReference  reference;
		struct JsCompletion completion;
	} u;

};
/****************************************************************************
									通用API
*****************************************************************************/
/*
	如果转换过程发生错误, 则res->type = JS_COMPLETION && JS_COMPLETION.type = throw;
*/
//type 为JsObject.h中的Hit
void JsToPrimitive(struct JsValue *val, int type, struct JsValue *res);
void JsToBoolean(struct JsValue *val, struct JsValue *res);
void JsToNumber(struct JsValue *val,struct JsValue *res);
void JsToInteger(struct JsValue *val, struct JsValue *res);
void JsToString(struct JsValue *val, struct JsValue *res);
void JsToObject(struct JsValue *val, struct JsValue *res);

/* Integer converters */

int  JsToInt32(struct JsValue *val);
unsigned int JsToUint32(struct JsValue *val);
unsigned short JsToUint16(struct JsValue *val);


/* JsReference */
void JsGetValue(struct JsValue* v,struct JsValue* res);
/*res->u.boolean = FALSE 表示失败*/
void JsPutValue(struct JsValue* v,struct JsValue* w,struct JsValue* res);

#endif