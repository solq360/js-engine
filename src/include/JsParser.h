#ifndef JsParserH
#define JsParserH

#include"JsType.h"

//JsParse.h
#define JS_PARSER_DEBUG_CLOSE 0 //全部关闭,包括语法错误提示
#define JS_PARSER_DEBUG_ERROR 1 //开启语法错误调试
#define JS_PARSER_DEBUG_NEW   2 //开启NEW调试
#define JS_PARSER_DEBUG_PARSE 3 //开启PARSE调试
#define JS_PARSER_DEBUG_ALL   4 //全部开启, 包括lexer

/****************************************************************************
									通用API
*****************************************************************************/
struct JsAstNode;
//filename = NULL的时候, 表示从stdin获取
void JsParseFile(int debug, char* filename, struct JsAstNode** ast);
void JsParseString(int debug, char* string, struct JsAstNode** ast);
#endif