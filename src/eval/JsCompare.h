#ifndef JsCompareH
#define JsCompareH

struct JsValue;

void JsRelationalExpressionCompare( struct JsValue *x, struct JsValue *y, struct JsValue *res);
void JsEqualityExpressionEqCompare( struct JsValue *x, struct JsValue *y, struct JsValue *res);
void JsEqualityExpressionSeqCompare( struct JsValue *x, struct JsValue *y, struct JsValue *res);

#endif