#include<stdlib.h>
#include<string.h>
#include<stdio.h>


#include"JsToken.h"
#include"JsValue.h"
#include"JsDebug.h"
#include"JsAst.h"
#include"JsParser.h"
#include"JsLexer.h"
#include"JsSys.h"

struct JsParser{
	yyscan_t lexer; //lexer分析器, 能被一个int yylex(YYSTYPE*, yyscan_t)的接口使用获得Token
	char* filename; //文件名
	int debug; //调试开关 0关闭  包括(语法) 1 开启节点声明调试 2 开启PARSE 调试 3 开启lex调试
	int error; //出现异常, 则返回到最上层
	int noin;//针对NoIn表达式
	int is_lhs;//是否是LeftHandSideExpression
	int funcdepth;//是否在函数中
	
	//Token
	int token ;//Lookahead(0) 的token
	YYSTYPE value;//Lookahead(0) 的Value
	int hasLineToken;//检测时,发现为tLINETERMINATOR的时候标志它, 并读取下一个非tLINETERMINATOR,
					// 且在SKIP的时候消除原先标记;
};
//参数辅助结构
struct var {
	char *name;
	struct var *next;		/* linked list of vars */
};

//-----------------------辅助函数---------------------------------------
	
static struct JsAstNode *new_node(struct JsParser *parser, int sz, 
        enum JsAstClassEnum nc, const char *dbg_nc);
//返回查看过程中是否发现换行
static void yylex0(YYSTYPE* v,int* t,int* has, yyscan_t lexer);


//-----------LL分析------------------------------------------------------
static struct JsAstNode *Literal_parse(struct JsParser *parser);
static struct JsAstNode *NumericLiteral_parse(struct JsParser *parser);
static struct JsAstNode *StringLiteral_parse(struct JsParser *parser);
static struct JsAstNode *PrimaryExpression_parse(struct JsParser *parser);
static struct JsAstNode *ArrayLiteral_parse(struct JsParser *parser);
static struct JsAstNode *ObjectLiteral_parse(struct JsParser *parser);
static struct JsAstArgumentsNode *Arguments_parse(struct JsParser *parser);
static struct JsAstNode *MemberExpression_parse(struct JsParser *parser);
static struct JsAstNode *LeftHandSideExpression_parse(struct JsParser *parser);
static struct JsAstNode *PostfixExpression_parse(struct JsParser *parser);
static struct JsAstNode *UnaryExpression_parse(struct JsParser *parser);
static struct JsAstNode *MultiplicativeExpression_parse(struct JsParser *parser);
static struct JsAstNode *AdditiveExpression_parse(struct JsParser *parser);
static struct JsAstNode *ShiftExpression_parse(struct JsParser *parser);
static struct JsAstNode *RelationalExpression_parse(struct JsParser *parser);
static struct JsAstNode *EqualityExpression_parse(struct JsParser *parser);
static struct JsAstNode *BitwiseANDExpression_parse(struct JsParser *parser);
static struct JsAstNode *BitwiseXORExpression_parse(struct JsParser *parser);
static struct JsAstNode *BitwiseORExpression_parse(struct JsParser *parser);
static struct JsAstNode *LogicalANDExpression_parse(struct JsParser *parser);
static struct JsAstNode *LogicalORExpression_parse(struct JsParser *parser);
static struct JsAstNode *ConditionalExpression_parse(struct JsParser *parser);
static struct JsAstNode *AssignmentExpression_parse(struct JsParser *parser);
static struct JsAstNode *Expression_parse(struct JsParser *parser);
static struct JsAstNode *Statement_parse(struct JsParser *parser);
static struct JsAstNode *Block_parse(struct JsParser *parser);
static struct JsAstNode *StatementList_parse(struct JsParser *parser);
static struct JsAstNode *VariableStatement_parse(struct JsParser *parser);
static struct JsAstNode *VariableDeclarationList_parse(struct JsParser *parser);
static struct JsAstNode *VariableDeclaration_parse(struct JsParser *parser);
static struct JsAstNode *EmptyStatement_parse(struct JsParser *parser);
static struct JsAstNode *ExpressionStatement_parse(struct JsParser *parser);
static struct JsAstNode *IfStatement_parse(struct JsParser *parser);
static struct JsAstNode *IterationStatement_parse(struct JsParser *parser);
static struct JsAstNode *ContinueStatement_parse(struct JsParser *parser);
static struct JsAstNode *BreakStatement_parse(struct JsParser *parser);
static struct JsAstNode *ReturnStatement_parse(struct JsParser *parser);
static struct JsAstNode *SwitchStatement_parse(struct JsParser *parser);
static struct JsAstNode *ThrowStatement_parse(struct JsParser *parser);
static struct JsAstNode *TryStatement_parse(struct JsParser *parser);
static struct JsAstNode *FunctionExpression_parse(struct JsParser *parser);
static struct var *FormalParameterList_parse(struct JsParser *parser);
static struct JsAstNode *FunctionBody_parse(struct JsParser *parser);
static struct JsAstNode *Program_parse(struct JsParser *parser);
static struct JsAstNode *SourceElements_parse(struct JsParser *parser);
/*------------------------------------------------------------
 * macros
 */
 
//转载下一个TOKEN, 且是唯一方式
#define SKIP \
		do{ \
			if(parser->token != tEND){ \
				 yylex0(&parser->value, \
					&parser->token,&parser->hasLineToken,parser->lexer);\
			}\
		}while(0)
		
//超前0个的Token	
#define NEXT \
		(parser->token)
		
//超前0个的value	
#define NEXT_VALUE \
		(&parser->value)

			
//解析PARSE		
#define PARSE(prod,res)					\
			do{ \
				if(parser->debug >= JS_PARSER_DEBUG_PARSE){\
					printf("Parse: %s next = %s\n", #prod, \
						JsTokenName(NEXT)); \
				}\
				res = prod##_parse(parser); \
				if(parser->error != 0)\
					return NULL;\
			}while(0)
			
//解析PARSE, 当时不进行分析, 出错也不会return		
#define PARSE_NO_CHECK(prod,res)					\
			do{ \
				if(parser->debug >= JS_PARSER_DEBUG_PARSE){\
					printf("Parse: %s next = %s\n", #prod, \
						JsTokenName(NEXT)); \
				}\
				res = prod##_parse(parser); \
			}while(0)	

			
//预测第0个字符, 并且SKIP
#define EXPECT(c) EXPECTX(c, JsTokenName(c))

#define EXPECTX(c, tokstr) \
    do { 						\
		EXPECTX_NOSKIP(c, tokstr);			\
		SKIP;						\
    } while (0)
	
#define EXPECT_NOSKIP(c) EXPECTX_NOSKIP(c, JsTokenName(c))

#define EXPECTX_NOSKIP(c, tokstr)			\
    do { 						\
		if (NEXT != (c)) 				\
			EXPECTED(tokstr);				\
    } while (0)

#define EXPECTED(tokstr)				\
    do { 						\
		if(parser->debug >= JS_PARSER_DEBUG_ERROR){ \
			printf(	 \
				"Parse: expected %s but got %c \n in filename: %s : %d \n",	\
				tokstr,			\
				NEXT, \
				parser->filename,\
				yyget_lineno(parser->lexer));			\
		} \
		parser->error = 1; \
		return NULL; \
    } while (0)
	
/* Generates a specific parse error */
#define ERRORm(m)					\
		do{ \
			if(parser->debug >= JS_PARSER_DEBUG_ERROR){ \
				printf(	 \
					"Parse: Error Message %s in filename: %s : %d \n",	\
					m,			\
					parser->filename,\
					yyget_lineno(parser->lexer));			\
			}\
			parser->error = 1; \
			return NULL;\
		}while(0)	

/* 
 * Automatic semicolon insertion macros.
 *
 * Using these instead of NEXT/SKIP allows synthesis of
 * semicolons where they are permitted by the standard.
 */

#define NEXT_FOLLOWS_NL (parser->hasLineToken)

#define NEXT_IS_SEMICOLON				\
	(NEXT == ';' || NEXT == '}' || NEXT == tEND || NEXT_FOLLOWS_NL )
	
#define EXPECT_SEMICOLON				\
    do {						\
		if (NEXT == ';')				\
			SKIP;					\
		else if (NEXT == '}' || NEXT == tEND ||NEXT_FOLLOWS_NL ) {	\
			\
		} else						\
			EXPECTX(';', "';', '}' or newline");	\
    } while (0)

	
#define NEW_NODE(t, nc)					\
	((t *)new_node(parser, sizeof (t), nc, #nc))
	
//---------------------------------------------------------------------

static struct JsAstNode *
new_node(parser, sz, nc,dbg_nc)
	struct JsParser *parser;
	int sz;
	enum JsAstClassEnum nc;
	const char *dbg_nc;
{
	struct JsAstNode *n;

	n = (struct JsAstNode *)JsMalloc(sz);
	n->astClass = nc;
	n->location = (struct JsLocation*) JsMalloc(sizeof(struct JsLocation));
	n->location->filename = parser->filename;
	//可能出现获得lookahead的lineno
	n->location->lineno =  yyget_lineno(parser->lexer);
	if(parser->debug >= JS_PARSER_DEBUG_NEW && dbg_nc != NULL){
		printf("Parse: NEW_NODE %p %s (next=%s)\n", 
			n, dbg_nc, JsTokenName(NEXT));
	}
	return n;
}
static void yylex0(YYSTYPE* v,int* t,int* has, yyscan_t lexer){
	//每次开始默认为没有lineToken
	int hasLineToken; 
	int token ;
	hasLineToken= 0;
	token = tEND;
	while(1){
		token = yylex(v,lexer);
		if(token != tLINETERMINATOR)
			break;
		hasLineToken = 1;
	}
	//非tLINETERMINATOR类型的Token
	*t = token;
	*has = hasLineToken;
	return;
}
//---------------------------------------------------------------------

/*
 *	-- 14
 *
 *	Program
 *	:	SourceElements
 *	;
 *
 *
 *	SourceElements
 *	:	SourceElement
 *	|	SourceElements SourceElement
 *	;
 *
 *
 *	SourceElement
 *	:	Statement
 *	|	*FunctionDeclaration去除该语法
 *	;
 */
static struct JsAstNode *
Program_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *body;
	
	PARSE(SourceElements,body);
	if (NEXT == '}')
		ERRORm("unmatched '}'");
	if (NEXT == ')')
		ERRORm("unmatched ')'");
	if (NEXT == ']')
		ERRORm("unmatched ']'");
	if (NEXT != tEND)
		ERRORm("unexpected token");
	return body;
}



static struct JsAstNode *
SourceElements_parse(parser)
	struct JsParser *parser;
{
	struct JsAstSourceElementsNode *se;
	struct JsAstSourceElement **s;

	se = NEW_NODE(struct JsAstSourceElementsNode, NODECLASS_SourceElements); 
	s = &se->statements;

	for (;;){ 
	    switch (NEXT) {
	    case tFUNCTION:
	    case tTHIS: case tIDENT: case tSTRING: case tNUMBER:
	    case tNULL: case tTRUE: case tFALSE:
	    case '(': case '[': case '{':
	    case tNEW: case tDELETE: case tVOID: case tTYPEOF:
	    case tPLUSPLUS: case tMINUSMINUS:
	    case '+': case '-': case '~': case '!': case ';':
	    case tVAR: case tIF: case tDO: case tWHILE: case tFOR:
	    case tCONTINUE: case tBREAK: case tRETURN:
	    case tWITH: case tSWITCH: case tTHROW: case tTRY:
	    case tDIV: case tDIVEQ: /* in lieu of tREGEX */
			*s = (struct JsAstSourceElement*)JsMalloc(sizeof(struct JsAstSourceElement));
			PARSE(Statement,(*s)->node);
			s = &(*s)->next;
			if (parser->debug >= JS_PARSER_DEBUG_NEW)
				printf("Parse: SourceElements_parse: got statement\n");
			break;
	    case tEND:
	    default:
			if (parser->debug >= JS_PARSER_DEBUG_NEW)
				printf("Parse: SourceElements_parse: got EOF/other (%s)\n", 
					JsTokenName(NEXT));
			*s = NULL;
			return (struct JsAstNode *)se;
		}
	}
}
/*
 *
 * -- 12
 *
 *	Statement
 *	:	Block
 *	|	VariableStatement
 *	|	EmptyStatement
 *	|	ExpressionStatement
 *	|	IfStatement
 *	|	IterationStatement
 *	|	ContinueStatement
 *	|	BreakStatement
 *	|	ReturnStatement
 *	|	*WithStatement消除
 *	|	*LabelledStatement消除
 *	|	SwitchStatement
 *	|	ThrowStatement
 *	|	TryStatement
 *	;
 *
 */

static struct JsAstNode *
Statement_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;


	switch (NEXT) {
	case '{':
		PARSE(Block,n);
		return n;
	case tVAR:
		PARSE(VariableStatement,n);
		return n;
	case ';':
		PARSE(EmptyStatement,n);
		return n;
	case tIF:
		PARSE(IfStatement,n);
		return n;
	case tDO:
	case tWHILE:
	case tFOR:
		PARSE(IterationStatement,n);
		return n;
	case tCONTINUE:
		PARSE(ContinueStatement,n);
		return n;
	case tBREAK:
		PARSE(BreakStatement,n);
		return n;
	case tRETURN:
		PARSE(ReturnStatement,n);
		return n;
	case tSWITCH:
		PARSE(SwitchStatement,n);
		return n;
	case tTHROW:
		PARSE(ThrowStatement,n);
		return n;
	case tTRY:
		PARSE(TryStatement,n);
		return n;
	default:
		PARSE(ExpressionStatement,n);
		return n;
	}
}

/*
 *	-- 12.1
 *
 *	Block
 *	:	'{' '}'					-- 12.1
 *	|	'{' StatementList '}'			-- 12.1
 *	;
 */
static struct JsAstNode *
Block_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;

	EXPECT('{');
	if (NEXT == '}')
		n = NEW_NODE(struct JsAstNode, NODECLASS_Block_empty);
	else
		PARSE(StatementList,n);
	EXPECT('}');
	return n;
}

/*
 *	StatementList
 *	:	Statement				-- 12.1
 *	|	StatementList Statement			-- 12.1
 *	;
 */
static struct JsAstNode *
StatementList_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;
	struct JsAstBinaryNode *ln;

	PARSE(Statement,n);
	switch (NEXT) {
	case '}':
	case tEND:
	case tCASE:
	case tDEFAULT:
		return n;
	}
	ln = NEW_NODE(struct JsAstBinaryNode, NODECLASS_StatementList);
	ln->a = n;
	PARSE(StatementList,ln->b);
	return (struct JsAstNode *)ln;
}

/*
 *	-- 12.2
 *
 *	VariableStatement
 *	:	tVAR VariableDeclarationList ';'
 *	;
 *
 *	VariableDeclarationList
 *	:	VariableDeclaration
 *	|	VariableDeclarationList ',' VariableDeclaration
 *	;
 *
 *	VariableDeclarationListNoIn
 *	:	VariableDeclarationNoIn
 *	|	VariableDeclarationListNoIn ',' VariableDeclarationNoIn
 *	;
 *
 *	VariableDeclaration
 *	:	tIDENT
 *	|	tIDENT Initialiser
 *	;
 *
 *	VariableDeclarationNoIn
 *	:	tIDENT
 *	|	tIDENT InitialiserNoIn
 *	;
 *
 *	Initialiser
 *	:	'=' AssignmentExpression
 *	;
 *
 *	InitialiserNoIn
 *	:	'=' AssignmentExpressionNoIn
 *	;
 */
static struct JsAstNode *
VariableStatement_parse(parser)
	struct JsParser *parser;
{
	struct JsAstUnaryNode *n;

	n = NEW_NODE(struct JsAstUnaryNode, NODECLASS_VariableStatement);
	EXPECT(tVAR);
	PARSE(VariableDeclarationList,n->a);
	EXPECT_SEMICOLON;
	return (struct JsAstNode *)n;
}

static struct JsAstNode *
VariableDeclarationList_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;
	struct JsAstBinaryNode *ln;

	PARSE(VariableDeclaration,n);
	if (NEXT != ',') 
		return n;
	ln = NEW_NODE(struct JsAstBinaryNode, NODECLASS_VariableDeclarationList);
	SKIP;
	/* NB: IterationStatement_parse() also constructs a VarDeclList */
	ln->a = n;
	PARSE(VariableDeclarationList,ln->b );
	return (struct JsAstNode *)ln;
}

static struct JsAstNode *
VariableDeclaration_parse(parser)
	struct JsParser *parser;
{
	struct JsAstVariableDeclarationNode *v;

	v = NEW_NODE(struct JsAstVariableDeclarationNode, 
		NODECLASS_VariableDeclaration);
	if (NEXT == tIDENT)
		v->var = NEXT_VALUE->string;
	EXPECT(tIDENT);
	if (NEXT == '=') {
		SKIP;
		PARSE(AssignmentExpression,v->init);
	} else
		v->init = NULL;
		
	return (struct JsAstNode *)v;
}

/*
 *	-- 12.3
 *
 *	EmptyStatement
 *	:	';'
 *	;
 */
static struct JsAstNode *
EmptyStatement_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;

	n = NEW_NODE(struct JsAstNode, NODECLASS_EmptyStatement);
	EXPECT_SEMICOLON;
	return n;
}

/*
 *	-- 12.4
 *
 *	ExpressionStatement
 *	:	Expression ';'		-- lookahead != '{' or tFUNCTION
 *	;
 */

static struct JsAstNode *
ExpressionStatement_parse(parser)
	struct JsParser *parser;
{
    struct JsAstUnaryNode *n;

	n = NEW_NODE(struct JsAstUnaryNode, NODECLASS_ExpressionStatement);
	PARSE(Expression,n->a);
	EXPECT_SEMICOLON;
	return (struct JsAstNode *)n;
}

/*
 *	-- 12.5
 *
 *	IfStatement
 *	:	tIF '(' Expression ')' Statement tELSE Statement
 *	|	tIF '(' Expression ')' Statement
 *	;
 */
static struct JsAstNode *
IfStatement_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *cond, *btrue, *bfalse;
	struct JsAstIfStatementNode *n;

	n = NEW_NODE(struct JsAstIfStatementNode, NODECLASS_IfStatement);
	EXPECT(tIF);
	EXPECT('(');
	PARSE(Expression,cond);
	EXPECT(')');
	PARSE(Statement,btrue);
	if (NEXT != tELSE)
		bfalse = NULL;
	else {
		SKIP; /* 'else' */
		PARSE(Statement,bfalse);
	}
	n->cond = cond;
	n->btrue = btrue;
	n->bfalse = bfalse;
	return (struct JsAstNode *)n;
}

/*
 *	-- 12.6
 *	IterationStatement
 *	:	tDO Statement tWHILE '(' Expression ')' ';'	-- 12.6.1
 *	|	tWHILE '(' Expression ')' Statement		-- 12.6.2
 *	|	tFOR '(' ';' ';' ')' Statement
 *	|	tFOR '(' ExpressionNoIn ';' ';' ')' Statement
 *	|	tFOR '(' ';' Expression ';' ')' Statement
 *	|	tFOR '(' ExpressionNoIn ';' Expression ';' ')' Statement
 *	|	tFOR '(' ';' ';' Expression ')' Statement
 *	|	tFOR '(' ExpressionNoIn ';' ';' Expression ')' Statement
 *	|	tFOR '(' ';' Expression ';' Expression ')' Statement
 *	|	tFOR '(' ExpressionNoIn ';' Expression ';' Expression ')'
 *			Statement
 *	|	tFOR '(' tVAR VariableDeclarationListNoIn ';' ';' ')' Statement
 *	|	tFOR '(' tVAR VariableDeclarationListNoIn ';'  
 *			Expression ';' ')' Statement
 *	|	tFOR '(' tVAR VariableDeclarationListNoIn ';' ';' 
 *			Expression ')' Statement
 *	|	tFOR '(' tVAR VariableDeclarationListNoIn ';' Expression ';' 
 *			Expression ')' Statement
 *	|	tFOR '(' LeftHandSideExpression tIN Expression ')' Statement
 *	|	tFOR '(' tVAR VariableDeclarationNoIn tIN Expression ')' 
 *			Statement
 *	;
 */

/* Note: the VarDecls of n->init are exposed through parser->vars */

static struct JsAstNode *
IterationStatement_parse(parser)
	struct JsParser *parser;
{
	struct JsAstIterationStatementWhileNode *w;
	struct JsAstIterationStatementForNode *fn;
	struct JsAstIterationStatementForinNode *fin;
	struct JsAstNode *n;
	
	switch (NEXT) {
	case tDO:
		w = NEW_NODE(struct JsAstIterationStatementWhileNode,
			NODECLASS_IterationStatement_dowhile);
		SKIP;
		PARSE(Statement,w->body);
		EXPECT(tWHILE);
		EXPECT('(');
		PARSE(Expression,w->cond);
		EXPECT(')');
		EXPECT_SEMICOLON;
		return (struct JsAstNode *)w;
	case tWHILE:
		w = NEW_NODE(struct JsAstIterationStatementWhileNode,
			NODECLASS_IterationStatement_while);
		SKIP;
		EXPECT('(');
		PARSE(Expression,w->cond);
		EXPECT(')');
		PARSE(Statement,w->body );
		return (struct JsAstNode *)w;
	case tFOR:
		break;
	default:
		ERRORm("unexpected token");
	}

	SKIP;		/* tFOR */
	EXPECT('(');

	if (NEXT == tVAR) {			 /* "for ( var" */
	    SKIP;	/* tVAR */
	    parser->noin = 1;
	    PARSE(VariableDeclarationList,n);	/* NB adds to parser->vars */
	    parser->noin = 0;
	    if (NEXT == tIN && 
		  n->astClass == NODECLASS_VariableDeclaration){				
			/* "for ( var VarDecl in" */
			fin = NEW_NODE(struct JsAstIterationStatementForinNode,
				NODECLASS_IterationStatement_forvarin);
			fin->lhs = n;
			SKIP;	/* tIN */
			PARSE(Expression,fin->list);
			EXPECT(')');
			PARSE(Statement,fin->body);
			return (struct JsAstNode *)fin;
	    }
	    /* Accurately describe possible tokens at this stage */
	    EXPECTX(';', 
		(n->astClass == NODECLASS_VariableDeclaration 
			? "';' or 'in'"
			: "';'"));
					    /* "for ( var VarDeclList ;" */
	    fn = NEW_NODE(struct JsAstIterationStatementForNode,
		NODECLASS_IterationStatement_forvar);

	    fn->init = n;
	    if (NEXT != ';')
			PARSE(Expression,fn->cond);
	    else
			fn->cond = NULL;
	    EXPECT(';');
	    if (NEXT != ')')
			PARSE(Expression,fn->incr);
	    else
			fn->incr = NULL;
	    EXPECT(')');
	    PARSE(Statement,fn->body);
	    return (struct JsAstNode *)fn;
	}

	if (NEXT != ';') {
	    parser->noin = 1;
	    PARSE(Expression,n);
	    parser->noin = 0;
	    if (NEXT == tIN && parser->is_lhs) {   /* "for ( lhs in" */
			fin = NEW_NODE(struct JsAstIterationStatementForinNode,
				NODECLASS_IterationStatement_forin);
			fin->lhs = n;
			SKIP;		/* tIN */
			PARSE(Expression,fin->list);
			EXPECT(')');
			PARSE(Statement,fin->body);
			return (struct JsAstNode *)fin;
	    }
	} else
	    n = NULL;				/* "for ( ;" */

	fn = NEW_NODE(struct JsAstIterationStatementForNode,
	    NODECLASS_IterationStatement_for);
	fn->init = n;
	EXPECT(';');
	if (NEXT != ';')
	    PARSE(Expression,fn->cond);
	else
	    fn->cond = NULL;
	EXPECT(';');
	if (NEXT != ')')
	    PARSE(Expression,fn->incr);
	else
	    fn->incr = NULL;
	EXPECT(')');
	PARSE(Statement,fn->body);
	return (struct JsAstNode *)fn;
}


/*
 *	-- 12.7
 *
 *	ContinueStatement
 *	:	tCONTINUE ';'
 *	|	*tCONTINUE tIDENT ';' 去除
 *	;
 */
static struct JsAstNode *
ContinueStatement_parse(parser)
	struct JsParser *parser;
{
	struct JsAstContinueStatementNode *cn;

	cn = NEW_NODE(struct JsAstContinueStatementNode,
		NODECLASS_ContinueStatement);
	EXPECT(tCONTINUE);
	EXPECT_SEMICOLON;
	return (struct JsAstNode *)cn;
}

/*
 *	-- 12.8
 *
 *	BreakStatement
 *	:	tBREAK ';'
 *	|	tBREAK tIDENT ';'
 *	;
 */
static struct JsAstNode *
BreakStatement_parse(parser)
	struct JsParser *parser;
{
	struct JsAstBreakStatementNode *cn;

	cn = NEW_NODE(struct JsAstBreakStatementNode,
		NODECLASS_BreakStatement);
	EXPECT(tBREAK);
	EXPECT_SEMICOLON;
	return (struct JsAstNode *)cn;
}

/*
 *	-- 12.9
 *
 *	ReturnStatement
 *	:	tRETURN ';'
 *	|	tRETURN Expression ';'
 *	;
 */
static struct JsAstNode *
ReturnStatement_parse(parser)
	struct JsParser *parser;
{
	struct JsAstReturnStatementNode *rn;

	EXPECT(tRETURN);
	if (!parser->funcdepth)
		ERRORm("'return' not within a function");
	if (!NEXT_IS_SEMICOLON) {
            rn = NEW_NODE(struct JsAstReturnStatementNode,
                        NODECLASS_ReturnStatement);
	    PARSE(Expression,rn->expr);
	} else
        rn = NEW_NODE(struct JsAstReturnStatementNode,
                        NODECLASS_ReturnStatement_undef);
	EXPECT_SEMICOLON;
	return (struct JsAstNode *)rn;
}


/*
 *	-- 12.11
 *
 *	SwitchStatement
 *	:	tSWITCH '(' Expression ')' CaseBlock
 *	;
 *
 *	CaseBlock
 *	:	'{' '}'
 *	|	'{' CaseClauses '}'
 *	|	'{' DefaultClause '}'
 *	|	'{' CaseClauses DefaultClause '}'
 *	|	'{' DefaultClause '}'
 *	|	'{' CaseClauses DefaultClause CaseClauses '}'
 *	;
 *
 *	CaseClauses
 *	:	CaseClause
 *	|	CaseClauses CaseClause
 *	;
 *
 *	CaseClause
 *	:	tCASE Expression ':'
 *	|	tCASE Expression ':' StatementList
 *	;
 *
 *	DefaultClause
 *	:	tDEFAULT ':'
 *	|	tDEFAULT ':' StatementList
 *	;
 */
static struct JsAstNode *
SwitchStatement_parse(parser)
	struct JsParser *parser;
{
	struct JsAstSwitchStatementNode *n;
	struct JsAstCaseList **cp, *c;
	int next;

	n = NEW_NODE(struct JsAstSwitchStatementNode,
		NODECLASS_SwitchStatement);

	EXPECT(tSWITCH);
	EXPECT('(');
	PARSE(Expression,n->cond);
	EXPECT(')');
	EXPECT('{');
	cp = &n->cases;
	n->defcase = NULL;
	while (NEXT != '}') {
	    c = (struct JsAstCaseList*)JsMalloc(sizeof(struct JsAstCaseList));
	    *cp = c;
	    cp = &c->next;
	    switch (NEXT) {
	    case tCASE:
			SKIP;
			PARSE(Expression,c->expr);
			break;
	    case tDEFAULT:
			SKIP;
			c->expr = NULL;
			if (n->defcase)
				ERRORm("duplicate 'default' clause");
			n->defcase = c;
			break;
	    default:
			EXPECTED("'}', 'case' or 'default'");
	    }
	    EXPECT(':');
	    next = NEXT;
	    if (next != '}' && next != tDEFAULT && next != tCASE)
			PARSE(StatementList,c->body);
	    else
			c->body = NULL;
	}
	*cp = NULL;
	EXPECT('}');
	return (struct JsAstNode *)n;
}

/*
 *	-- 12.13
 *
 *	ThrowStatement
 *	:	tTHROW Expression ';'
 *	;
 */
static struct JsAstNode *
ThrowStatement_parse(parser)
	struct JsParser *parser;
{
	struct JsAstUnaryNode *n;

	n = NEW_NODE(struct JsAstUnaryNode, NODECLASS_ThrowStatement);
	EXPECT(tTHROW);
	if (NEXT_FOLLOWS_NL)
		ERRORm("newline not allowed after 'throw'");
	PARSE(Expression,n->a);
	EXPECT_SEMICOLON;
	return (struct JsAstNode *)n;
}


/*
 *	-- 12.14
 *
 *	TryStatement
 *	:	tTRY Block Catch
 *	|	tTRY Block Finally
 *	|	tTRY Block Catch Finally
 *	;
 *
 *	Catch
 *	:	tCATCH '(' tIDENT ')' Block
 *	;
 *
 *	Finally
 *	:	tFINALLY Block
 *	;
 */
static struct JsAstNode *
TryStatement_parse(parser)
	struct JsParser *parser;
{
	struct JsAstTryStatementNode *n;
	enum JsAstClassEnum nc;
    struct JsAstNode *block, *bcatch, *bfinally;
	char *ident = NULL;

	EXPECT(tTRY);
	PARSE(Block,block);
	if (NEXT == tCATCH) {
	    SKIP;
	    EXPECT('(');
	    if (NEXT == tIDENT)
		    ident = NEXT_VALUE->string;
	    EXPECT(tIDENT);
	    EXPECT(')');
	    PARSE(Block,bcatch);
	} else
	    bcatch = NULL;

	if (NEXT == tFINALLY) {
	    SKIP;
	    PARSE(Block,bfinally);
	} else
	    bfinally = NULL;

	if (bcatch && bfinally)
		nc = NODECLASS_TryStatement_catchfinally;
	else if (bcatch)
		nc = NODECLASS_TryStatement_catch;
	else if (bfinally)
		nc = NODECLASS_TryStatement_finally;
	else
		ERRORm("expected 'catch' or 'finally'");

	n = NEW_NODE(struct JsAstTryStatementNode, nc);
	n->block = block;
	n->bcatch = bcatch;
	n->bfinally = bfinally;
	n->ident = ident;

	return (struct JsAstNode *)n;
}




/*
 *	-- 11.14
 *
 *	Expression
 *	:	AssignmentExpression
 *	|	Expression ',' AssignmentExpression
 *	;
 *
 *	ExpressionNoIn
 *	:	AssignmentExpressionNoIn
 *	|	ExpressionNoIn ',' AssignmentExpressionNoIn
 *	;
 *
 *
 */
static struct JsAstNode *
Expression_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;
	struct JsAstBinaryNode *cn;

	PARSE(AssignmentExpression,n);
	if (NEXT != ',')
		return n;
	cn = NEW_NODE(struct JsAstBinaryNode, NODECLASS_Expression_comma);
	SKIP;
	cn->a = n;
	PARSE(Expression,cn->b);
	parser->is_lhs = 0;
	return (struct JsAstNode *)cn;
}

/*
 *	-- 11.13
 *
 *	AssignmentExpression
 *	:	ConditionalExpression
 *	|	LeftHandSideExpression AssignmentOperator AssignmentExpression
 *	;
 *
 *	AssignmentExpressionNoIn
 *	:	ConditionalExpressionNoIn
 *	|	LeftHandSideExpression AssignmentOperator 
 *						AssignmentExpressionNoIn
 *	;
 *
 *	AssignmentOperator
 *	:	'='				-- 11.13.1
 *	|	tSTAREQ				-- 11.13.2
 *	|	tDIVEQ
 *	|	tMODEQ
 *	|	tPLUSEQ
 *	|	tMINUSEQ
 *	|	tLSHIFTEQ
 *	|	tRSHIFTEQ
 *	|	tURSHIFTEQ
 *	|	tANDEQ
 *	|	tXOREQ
 *	|	tOREQ
 *	;
 */
static struct JsAstNode *
AssignmentExpression_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;
	enum JsAstClassEnum nc;
	struct JsAstAssignmentExpressionNode *an;

	/*
	 * If, while recursing we parse LeftHandSideExpression,
	 * then is_lhs will be set to 1. Otherwise, it is just a 
	 * ConditionalExpression, and we cannot derive the second
	 * production in this rule. So we just return.
	 */
	PARSE(ConditionalExpression,n);
	if (!parser->is_lhs)
		return n;

	switch (NEXT) {
	case '=':
		nc = NODECLASS_AssignmentExpression_simple;
		break;
	case tSTAREQ:
		nc = NODECLASS_AssignmentExpression_muleq;
		break;
	case tDIVEQ:
		nc = NODECLASS_AssignmentExpression_diveq;
		break;
	case tMODEQ:
		nc = NODECLASS_AssignmentExpression_modeq;
		break;
	case tPLUSEQ:
		nc = NODECLASS_AssignmentExpression_addeq;
		break;
	case tMINUSEQ:
		nc = NODECLASS_AssignmentExpression_subeq;
		break;
	case tLSHIFTEQ:
		nc = NODECLASS_AssignmentExpression_lshifteq;
		break;
	case tRSHIFTEQ:
		nc = NODECLASS_AssignmentExpression_rshifteq;
		break;
	case tURSHIFTEQ:
		nc = NODECLASS_AssignmentExpression_urshifteq;
		break;
	case tANDEQ:
		nc = NODECLASS_AssignmentExpression_andeq;
		break;
	case tXOREQ:
		nc = NODECLASS_AssignmentExpression_xoreq;
		break;
	case tOREQ:
		nc = NODECLASS_AssignmentExpression_oreq;
		break;
	default:
		return n;
	}
	an = NEW_NODE(struct JsAstAssignmentExpressionNode, nc);
	an->lhs = n;
	SKIP;
	PARSE(AssignmentExpression,an->expr);
	parser->is_lhs = 0;
	return (struct JsAstNode *)an;
}

/*
 *	-- 11.12
 *
 *	ConditionalExpression
 *	:	LogicalORExpression
 *	|	LogicalORExpression '?' 
 *			AssignmentExpression ':' AssignmentExpression
 *	;
 *
 *	ConditionalExpressionNoIn
 *	:	LogicalORExpressionNoIn
 *	|	LogicalORExpressionNoIn '?' 
 *			AssignmentExpressionNoIn ':' AssignmentExpressionNoIn
 *	;
 */

static struct JsAstNode *
ConditionalExpression_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;
	struct JsAstConditionalExpressionNode *m;

	PARSE(LogicalORExpression,n);
	if (NEXT != '?') 
		return n;
	m = NEW_NODE(struct JsAstConditionalExpressionNode,
			NODECLASS_ConditionalExpression);
	SKIP;
	m->a = n;
	PARSE(AssignmentExpression,m->b);
	EXPECT(':');
	PARSE(AssignmentExpression,m->c);
	parser->is_lhs = 0;
	return (struct JsAstNode *)m;
}

/*
 *	LogicalORExpression
 *	:	LogicalANDExpression
 *	|	LogicalORExpression tOROR LogicalANDExpression
 *	;
 *
 *	LogicalORExpressionNoIn
 *	:	LogicalANDExpressionNoIn
 *	|	LogicalORExpressionNoIn tOROR LogicalANDExpressionNoIn
 *	;
 */

static struct JsAstNode *
LogicalORExpression_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;
	struct JsAstBinaryNode *m;

	PARSE(LogicalANDExpression,n);
	if (NEXT != tOROR) 
		return n;
	m = NEW_NODE(struct JsAstBinaryNode,
			NODECLASS_LogicalORExpression);
	SKIP;
	m->a = n;
	PARSE(LogicalORExpression,m->b);
	parser->is_lhs = 0;
	return (struct JsAstNode *)m;
}

/*
 *	-- 11.11
 *
 *	LogicalANDExpression
 *	:	BitwiseORExpression
 *	|	LogicalANDExpression tANDAND BitwiseORExpression
 *	;
 *
 *	LogicalANDExpressionNoIn
 *	:	BitwiseORExpressionNoIn
 *	|	LogicalANDExpressionNoIn tANDAND BitwiseORExpressionNoIn
 *	;
 */
static struct JsAstNode *
LogicalANDExpression_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;
	struct JsAstBinaryNode *m;

	PARSE(BitwiseORExpression,n);
	if (NEXT != tANDAND) 
		return n;
	m = NEW_NODE(struct JsAstBinaryNode,
			NODECLASS_LogicalANDExpression);
	SKIP;
	m->a = n;
	PARSE(LogicalANDExpression,m->b);
	parser->is_lhs = 0;
	return (struct JsAstNode *)m;
}

/*
 *	BitwiseORExpression
 *	:	BitwiseXORExpression
 *	|	BitwiseORExpression '|' BitwiseXORExpression
 *	;
 *
 *	BitwiseORExpressionNoIn
 *	:	BitwiseXORExpressionNoIn
 *	|	BitwiseORExpressionNoIn '|' BitwiseXORExpressionNoIn
 *	;
 */
static struct JsAstNode *
BitwiseORExpression_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;
	struct JsAstBinaryNode *m;

	PARSE(BitwiseXORExpression,n);
	if (NEXT != '|') 
		return n;
	m = NEW_NODE(struct JsAstBinaryNode,
			NODECLASS_BitwiseORExpression);
	SKIP;
	m->a = n;
	PARSE(BitwiseORExpression,m->b);
	parser->is_lhs = 0;
	return (struct JsAstNode *)m;
}

/*
 *	BitwiseXORExpression
 *	:	BitwiseANDExpression
 *	|	BitwiseXORExpression '^' BitwiseANDExpression
 *	;
 *
 *	BitwiseXORExpressionNoIn
 *	:	BitwiseANDExpressionNoIn
 *	|	BitwiseXORExpressionNoIn '^' BitwiseANDExpressionNoIn
 *	;
 */
static struct JsAstNode *
BitwiseXORExpression_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;
	struct JsAstBinaryNode *m;

	PARSE(BitwiseANDExpression,n);
	if (NEXT != '^') 
		return n;
	m = NEW_NODE(struct JsAstBinaryNode,
			NODECLASS_BitwiseXORExpression);
	SKIP;
	m->a = n;
	PARSE(BitwiseXORExpression,m->b);
	parser->is_lhs = 0;
	return (struct JsAstNode *)m;
}

/*
 *	-- 11.10
 *
 *	BitwiseANDExpression
 *	:	EqualityExpression
 *	|	BitwiseANDExpression '&' EqualityExpression
 *	;
 *
 *	BitwiseANDExpressionNoIn
 *	:	EqualityExpressionNoIn
 *	|	BitwiseANDExpressionNoIn '&' EqualityExpressionNoIn
 *	;
 */
static struct JsAstNode *
BitwiseANDExpression_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;
	struct JsAstBinaryNode *m;

	PARSE(EqualityExpression,n);
	if (NEXT != '&') 
		return n;
	m = NEW_NODE(struct JsAstBinaryNode,
			NODECLASS_BitwiseANDExpression);
	SKIP;
	m->a = n;
	PARSE(BitwiseANDExpression,m->b);
	parser->is_lhs = 0;
	return (struct JsAstNode *)m;
}

/*
 *	-- 11.9
 *
 *	EqualityExpression
 *	:	RelationalExpression
 *	|	EqualityExpression tEQ RelationalExpression	-- 11.9.1
 *	|	EqualityExpression tNE RelationalExpression	-- 11.9.2
 *	|	EqualityExpression tSEQ RelationalExpression	-- 11.9.4
 *	|	EqualityExpression tSNE RelationalExpression	-- 11.9.5
 *	;
 *
 *	EqualityExpressionNoIn
 *	:	RelationalExpressionNoIn
 *	|	EqualityExpressionNoIn tEQ RelationalExpressionNoIn  -- 11.9.1
 *	|	EqualityExpressionNoIn tNE RelationalExpressionNoIn  -- 11.9.2
 *	|	EqualityExpressionNoIn tSEQ RelationalExpressionNoIn -- 11.9.4
 *	|	EqualityExpressionNoIn tSNE RelationalExpressionNoIn -- 11.9.5
 *	;
 */

static struct JsAstNode *
EqualityExpression_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;
	enum JsAstClassEnum nc;
	struct JsAstBinaryNode *rn;

	PARSE(RelationalExpression,n);
	for (;;) {
	    /* Left associative */
	    switch (NEXT) {
	    case tEQ:
			nc = NODECLASS_EqualityExpression_eq;
			break;
	    case tNE:
			nc = NODECLASS_EqualityExpression_ne;
			break;
	    case tSEQ:
			nc = NODECLASS_EqualityExpression_seq;
			break;
	    case tSNE:
			nc = NODECLASS_EqualityExpression_sne;
			break;
	    default:
			return n;
	    }
	    rn = NEW_NODE(struct JsAstBinaryNode, nc);
	    SKIP;
	    rn->a = n;
	    PARSE(EqualityExpression,rn->b);
	    parser->is_lhs = 0;
	    n = (struct JsAstNode *)rn;
	}
}

/*
 *	-- 11.8
 *
 *	RelationalExpression
 *	:	ShiftExpression
 *	|	RelationalExpression '<' ShiftExpression	 -- 11.8.1
 *	|	RelationalExpression '>' ShiftExpression	 -- 11.8.2
 *	|	RelationalExpression tLE ShiftExpression	 -- 11.8.3
 *	|	RelationalExpression tGE ShiftExpression	 -- 11.8.4
 *	|	RelationalExpression tINSTANCEOF ShiftExpression -- 11.8.6
 *	|	RelationalExpression tIN ShiftExpression	 -- 11.8.7
 *	;
 *
 *	RelationalExpressionNoIn
 *	:	ShiftExpression
 *	|	RelationalExpressionNoIn '<' ShiftExpression	 -- 11.8.1
 *	|	RelationalExpressionNoIn '>' ShiftExpression	 -- 11.8.2
 *	|	RelationalExpressionNoIn tLE ShiftExpression	 -- 11.8.3
 *	|	RelationalExpressionNoIn tGE ShiftExpression	 -- 11.8.4
 *	|	RelationalExpressionNoIn tINSTANCEOF ShiftExpression -- 11.8.6
 *	;
 *
 * The *NoIn productions are implemented by the 'noin' boolean field
 * in the parser state.
 */

static struct JsAstNode *
RelationalExpression_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;
	enum JsAstClassEnum nc;
	struct JsAstBinaryNode *rn;

	PARSE(ShiftExpression,n);
	for (;;) {
	    /* Left associative */
	    switch (NEXT) {
	    case '<':
			nc = NODECLASS_RelationalExpression_lt;
			break;
	    case '>':
			nc = NODECLASS_RelationalExpression_gt;
			break;
	    case tLE:
			nc = NODECLASS_RelationalExpression_le;
			break;
	    case tGE:
			nc = NODECLASS_RelationalExpression_ge;
			break;
	    case tINSTANCEOF:
			nc = NODECLASS_RelationalExpression_instanceof;
			break;
	    case tIN:
			if (!parser->noin) {
				nc = NODECLASS_RelationalExpression_in;
				break;
			} /* else Fallthrough */
		default:
			return n;
	    }
	    rn = NEW_NODE(struct JsAstBinaryNode, nc);
	    SKIP;
	    rn->a = n;
	    PARSE(RelationalExpression,rn->b);
	    parser->is_lhs = 0;
	    n = (struct JsAstNode *)rn;
	}
}

/*
 *	-- 11.7
 *
 *	ShiftExpression
 *	:	AdditiveExpression
 *	|	ShiftExpression tLSHIFT AdditiveExpression	-- 11.7.1
 *	|	ShiftExpression tRSHIFT AdditiveExpression	-- 11.7.2
 *	|	ShiftExpression tURSHIFT AdditiveExpression	-- 11.7.3
 *	;
 */
static struct JsAstNode *
ShiftExpression_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;
	enum JsAstClassEnum nc;
	struct JsAstBinaryNode *sn;

	PARSE(AdditiveExpression,n);
	for (;;) {
	    /* Left associative */
	    switch (NEXT) {
	    case tLSHIFT:
			nc = NODECLASS_ShiftExpression_lshift;
			break;
	    case tRSHIFT:
			nc = NODECLASS_ShiftExpression_rshift;
			break;
	    case tURSHIFT:
			nc = NODECLASS_ShiftExpression_urshift;
			break;
	    default:
			return n;
	    }
	    sn = NEW_NODE(struct JsAstBinaryNode, nc);
	    SKIP;
	    sn->a = n;
	    PARSE(AdditiveExpression,sn->b);
	    parser->is_lhs = 0;
	    n = (struct JsAstNode *)sn;
	}
}

/*
 *	-- 11.6
 *
 *	AdditiveExpression
 *	:	MultiplicativeExpression
 *	|	AdditiveExpression '+' MultiplicativeExpression	-- 11.6.1
 *	|	AdditiveExpression '-' MultiplicativeExpression	-- 11.6.2
 *	;
 */
static struct JsAstNode *
AdditiveExpression_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;
	enum JsAstClassEnum nc;
	struct JsAstBinaryNode *m;

	PARSE(MultiplicativeExpression,n);
	for (;;) {
	    switch (NEXT) {
	    case '+':
			nc = NODECLASS_AdditiveExpression_add;
			break;
	    case '-':
			nc = NODECLASS_AdditiveExpression_sub;
			break;
	    default:
			return n;
	    }
	    parser->is_lhs = 0;
	    SKIP;
	    m = NEW_NODE(struct JsAstBinaryNode, nc);
	    m->a = n;
	    PARSE(MultiplicativeExpression,m->b);
	    n = (struct JsAstNode *)m;
	}
	return n;
}

/*
 *	-- 11.5
 *
 *	MultiplicativeExpression
 *	:	UnaryExpression
 *	|	MultiplicativeExpression '*' UnaryExpression	-- 11.5.1
 *	|	MultiplicativeExpression '/' UnaryExpression	-- 11.5.2
 *	|	MultiplicativeExpression '%' UnaryExpression	-- 11.5.3
 *	;
 */

static struct JsAstNode *
MultiplicativeExpression_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;
	enum JsAstClassEnum nc;
	struct JsAstBinaryNode *m;

	PARSE(UnaryExpression,n);
	for (;;) {
	    /* Left-to-right associative */
	    switch (NEXT) {
	    case '*':
			nc = NODECLASS_MultiplicativeExpression_mul;
			break;
	    case '/':
			nc = NODECLASS_MultiplicativeExpression_div;
			break;
	    case '%':
			nc = NODECLASS_MultiplicativeExpression_mod;
			break;
	    default:
			return n;
	    }
	    SKIP;
	    m = NEW_NODE(struct JsAstBinaryNode, nc);
	    m->a = n;
	    PARSE(UnaryExpression,m->b);
	    parser->is_lhs = 0;
	    n = (struct JsAstNode *)m;
	}
}

/*
 *	-- 11.4
 *
 *	UnaryExpression
 *	:	PostfixExpression
 *	|	tDELETE UnaryExpression				-- 11.4.1
 *	|	tVOID UnaryExpression				-- 11.4.2
 *	|	tTYPEOF UnaryExpression				-- 11.4.3
 *	|	tPLUSPLUS UnaryExpression			-- 11.4.4
 *	|	tMINUSMINUS UnaryExpression			-- 11.4.5
 *	|	'+' UnaryExpression				-- 11.4.6
 *	|	'-' UnaryExpression				-- 11.4.7
 *	|	'~' UnaryExpression				-- 11.4.8
 *	|	'!' UnaryExpression				-- 11.4.9
 *	;
 */
static struct JsAstNode *
UnaryExpression_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *p;
	struct JsAstUnaryNode *n;
	enum JsAstClassEnum nc;

	switch (NEXT) {
	case tDELETE:
		nc = NODECLASS_UnaryExpression_delete;
		break;
	case tVOID:
		nc = NODECLASS_UnaryExpression_void;
		break;
	case tTYPEOF:
		nc = NODECLASS_UnaryExpression_typeof;
		break;
	case tPLUSPLUS:
		nc = NODECLASS_UnaryExpression_preinc;
		break;
	case tMINUSMINUS:
		nc = NODECLASS_UnaryExpression_predec;
		break;
	case '+':
		nc = NODECLASS_UnaryExpression_plus;
		break;
	case '-':
		nc = NODECLASS_UnaryExpression_minus;
		break;
	case '~':
		nc = NODECLASS_UnaryExpression_inv;
		break;
	case '!':
		nc = NODECLASS_UnaryExpression_not;
		break;
	default:
		PARSE(PostfixExpression,p);
		return p;
	}
	n = NEW_NODE(struct JsAstUnaryNode, nc);
	SKIP;
	PARSE(UnaryExpression,n->a);
	parser->is_lhs = 0;
	return (struct JsAstNode *)n;
}

/*
 *	-- 11.3
 *
 *	PostfixExpression
 *	:	LeftHandSideExpression
 *	|	LeftHandSideExpression { NOLINETERM; } tPLUSPLUS    -- 11.3.1
 *	|	LeftHandSideExpression { NOLINETERM; } tMINUSMINUS  -- 11.3.2
 *	;
 */

static struct JsAstNode *
PostfixExpression_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;
	struct JsAstUnaryNode *pen;

	PARSE(LeftHandSideExpression,n);
	if (!NEXT_FOLLOWS_NL && 
	    (NEXT == tPLUSPLUS || NEXT == tMINUSMINUS))
	{
		pen = NEW_NODE(struct JsAstUnaryNode,
			NEXT == tPLUSPLUS
			    ? NODECLASS_PostfixExpression_inc
			    : NODECLASS_PostfixExpression_dec);
		pen->a = n;
		n = (struct JsAstNode *)pen;
		SKIP;
		parser->is_lhs = 0;
	}
	return n;
}

/*
 *	-- 11.2
 *
 *	MemberExpression
 *	:	PrimaryExpression
 *	|	FunctionExpression				-- 11.2.5
 *	|	MemberExpression '[' Expression ']'		-- 11.2.1
 *	|	MemberExpression '.' tIDENT			-- 11.2.1
 *	|	tNEW MemberExpression Arguments			-- 11.2.2
 *	;
 *
 *	NewExpression
 *	:	MemberExpression
 *	|	tNEW NewExpression				-- 11.2.2
 *	;
 *
 *	CallExpression
 *	:	MemberExpression Arguments			-- 11.2.3
 *	|	CallExpression Arguments			-- 11.2.3
 *	|	CallExpression '[' Expression ']'		-- 11.2.1
 *	|	CallExpression '.' tIDENT			-- 11.2.1
 *	;
 *
 *	Arguments
 *	:	'(' ')'						-- 11.2.4
 *	|	'(' ArgumentList ')'				-- 11.2.4
 *	;
 *
 *	ArgumentList
 *	:	AssignmentExpression				-- 11.2.4
 *	|	ArgumentList ',' AssignmentExpression		-- 11.2.4
 *	;
 *
 *	LeftHandSideExpression
 *	:	NewExpression
 *	|	CallExpression
 *	;
 *
 * NOTE:  The standard grammar is complicated in order to resolve an 
 *        ambiguity in parsing 'new expr ( args )' as either
 *	  '(new  expr)(args)' or as 'new (expr(args))'. In fact, 'new'
 *	  is acting as both a unary and a binary operator. Yucky.
 *
 *	  Since recursive descent is single-token lookahead, we
 *	  can rewrite the above as the following equivalent grammar:
 *
 *	MemberExpression
 *	:	PrimaryExpression
 *	|	FunctionExpression		    -- lookahead == tFUNCTION
 *	|	MemberExpression '[' Expression ']'
 *	|	MemberExpression '.' tIDENT
 *	|	tNEW MemberExpression Arguments	    -- lookahead == tNEW
 *	|	tNEW MemberExpression 	            -- lookahead == tNEW
 *
 *	LeftHandSideExpression
 *	:	PrimaryExpression
 *	|	FunctionExpression		    -- lookahead == tFUNCTION
 *	|	LeftHandSideExpression '[' Expression ']'
 *	|	LeftHandSideExpression '.' tIDENT
 *	|	LeftHandSideExpression Arguments
 *	|	MemberExpression		    -- lookahead == tNEW
 *
 */


static struct JsAstArgumentsNode *
Arguments_parse(parser)
	struct JsParser *parser;
{
	struct JsAstArgumentsNode *n;
	struct JsAstArgumentsArg **argp;

	n = NEW_NODE(struct JsAstArgumentsNode,
			NODECLASS_Arguments);
	argp = &n->first;
	n->argc = 0;

	EXPECT('(');
	while (NEXT != ')') {
		n->argc++;
		*argp = (struct JsAstArgumentsArg *)JsMalloc( sizeof(struct JsAstArgumentsArg));
		PARSE(AssignmentExpression,(*argp)->expr);
		argp = &(*argp)->next;
		if (NEXT != ')')
			EXPECTX(',', "',' or ')'");
	}
	*argp = NULL;
	EXPECT(')');
	return n;
}


/* 11.2.1 */
static struct JsAstNode *
MemberExpression_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;
	struct JsAstMemberExpressionNewNode *m;
	struct JsAstMemberExpressionDotNode *dn;
	struct JsAstMemberExpressionBracketNode *bn;

	switch (NEXT) {
        case tFUNCTION:
			PARSE(FunctionExpression,n);
			break;
        case tNEW:
			m = NEW_NODE(struct JsAstMemberExpressionNewNode,
				NODECLASS_MemberExpression_new);
			SKIP;
			PARSE(MemberExpression,m->mexp );
			if (NEXT == '(')
				PARSE(Arguments,m->args);
			else
				m->args = NULL;
			n = (struct JsAstNode *)m;
			break;
	default:
	    PARSE(PrimaryExpression,n);
	}

	for (;;)
	    switch (NEXT) {
	    case '.':
			dn = NEW_NODE(struct JsAstMemberExpressionDotNode,
				NODECLASS_MemberExpression_dot);
			SKIP;
			if (NEXT == tIDENT) {
				dn->mexp = n;
				dn->name = NEXT_VALUE->string;
				n = (struct JsAstNode *)dn;
			}
			EXPECT(tIDENT);
			break;
	    case '[':
			bn = NEW_NODE(struct JsAstMemberExpressionBracketNode,
				NODECLASS_MemberExpression_bracket);
			SKIP;
			bn->mexp = n;
			PARSE(Expression,bn->name);
			n = (struct JsAstNode *)bn;
			EXPECT(']');
			break;
	    default:
			return n;
	    }
}

static struct JsAstNode *
LeftHandSideExpression_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;
	struct JsAstCallExpressionNode *cn;
	struct JsAstMemberExpressionDotNode *dn;
	struct JsAstMemberExpressionBracketNode *bn;

	switch (NEXT) {
	case tFUNCTION:
		PARSE(FunctionExpression,n);	/* 11.2.5 */
		break;
	case tNEW:
		PARSE(MemberExpression,n);
		break;
	default:
		PARSE(PrimaryExpression,n);
	}

	for (;;)  {

	    switch (NEXT) {
	    case '.':
	        dn = NEW_NODE(struct JsAstMemberExpressionDotNode,
		    NODECLASS_MemberExpression_dot);
			SKIP;
			if (NEXT == tIDENT) {
				dn->mexp = n;
				dn->name = NEXT_VALUE->string;
				n = (struct JsAstNode *)dn;
			}
	        EXPECT(tIDENT);
			break;
	    case '[':
			bn = NEW_NODE(struct JsAstMemberExpressionBracketNode,
				NODECLASS_MemberExpression_bracket);
			SKIP;
			bn->mexp = n;
			PARSE(Expression,bn->name);
			n = (struct JsAstNode *)bn;
			EXPECT(']');
			break;
	    case '(':
			cn = NEW_NODE(struct JsAstCallExpressionNode,
				NODECLASS_CallExpression);
			cn->exp = n;
			PARSE(Arguments,cn->args );
			n = (struct JsAstNode *)cn;
			break;
	    default:
			/* Eventually we leave via this clause */
			parser->is_lhs = 1;
			return n;
	    }
	}
}


/*
 *	ObjectLiteral				-- 11.1.5
 *	:	'{' '}'
 *	|	'{' PropertyNameAndValueList '}'
 *	;
 *
 *	PropertyNameAndValueList
 *	:	PropertyName ':' AssignmentExpression
 *	|	PropertyNameAndValueList ',' PropertyName ':' 
 *							AssignmentExpression
 *	;
 *
 *	PropertyName
 *	:	tIDENT
 *	|	StringLiteral
 *	|	NumericLiteral
 *	;
 */
static struct JsAstNode *
ObjectLiteral_parse(parser)
	struct JsParser *parser;
{
	struct JsAstObjectLiteralNode *n;
	struct JsAstObjectLiteralPair **pairp;
	double value;
	char* buf;
	n = NEW_NODE(struct JsAstObjectLiteralNode,
			NODECLASS_ObjectLiteral);
	pairp = &n->first;

	EXPECT('{');
	while (NEXT != '}') {
	    *pairp = (struct JsAstObjectLiteralPair*)JsMalloc( 
			sizeof(struct JsAstObjectLiteralPair));
	    switch (NEXT) {
	    case tIDENT:
	    case tSTRING:
			(*pairp)->name = NEXT_VALUE->string;
			SKIP;
			break;
	    case tNUMBER:
			value = NEXT_VALUE->number;
			buf = (char*)JsMalloc(128);
			if(value - (int)value == 0){
				//整数
				sprintf(buf,"%d",(int)value);
			}else{
				sprintf(buf,"%f",value);
			}
			(*pairp)->name = buf;
			SKIP;
			break;
		default:
			EXPECTED("string, identifier or number");
	    }
	    EXPECT(':');
	    PARSE(AssignmentExpression,(*pairp)->value);
	    if (NEXT != '}') {
		    EXPECTX(',', "',' or '}'"); 
			if (NEXT == '}')
				EXPECTED("string, identifier or number after ','");
	    }
	    pairp = &(*pairp)->next;
	}
	*pairp = NULL;
	EXPECT('}');
	return (struct JsAstNode *)n;
}

/*
 *	ArrayLiteral				-- 11.1.4
 *	:	'[' ']'
 *	|	'[' Elision ']'
 *	|	'[' ElementList ']'
 *	|	'[' ElementList ',' ']'
 *	|	'[' ElementList ',' Elision ']'
 *	;
 *
 *	ElementList
 *	:	Elision AssignmentExpression
 *	|	AssignmentExpression
 *	|	ElementList ',' Elision AssignmentExpression
 *	|	ElementList ',' AssignmentExpression
 *	;
 *
 *	Elision
 *	:	','
 *	|	Elision ','
 *	;
 *
 * NB: I ignore the above elision nonsense and just build a list of
 * (index,expr) nodes with an overall length. It is equivalent 
 * to that in the standard.
 */
static struct JsAstNode *
ArrayLiteral_parse(parser)
	struct JsParser *parser;
{
	struct JsAstArrayLiteralNode *n;
	struct JsAstArrayLiteralElement **elp;
	int index;

	n = NEW_NODE(struct JsAstArrayLiteralNode,
	    NODECLASS_ArrayLiteral);
	elp = &n->first;

	EXPECT('[');
	index = 0;
	while (NEXT != ']'){
		if (NEXT == ',') {
			index++;
			SKIP;
		} else {
			*elp = (struct JsAstArrayLiteralElement*)JsMalloc(
				sizeof(struct JsAstArrayLiteralElement));
			(*elp)->index = index;
			PARSE(AssignmentExpression,(*elp)->expr);
			elp = &(*elp)->next;
			index++;
			if (NEXT != ']')
				EXPECTX(',', "',' or ']'");
		}
	}
	n->length = index;
	*elp = NULL;
	EXPECT(']');
	return (struct JsAstNode *)n;
}

/*------------------------------------------------------------
 * -- 11.1
 *
 *	PrimaryExpression
 *	:	tTHIS				-- 11.1.1
 *	|	tIDENT				-- 11.1.2
 *	|	Literal				-- 11.1.3
 *	|	ArrayLiteral
 *	|	ObjectLiteral
 *	|	'(' Expression ')'		-- 11.1.6
 *	;
 */
static struct JsAstNode *
PrimaryExpression_parse(parser)
	struct JsParser *parser;
{
	struct JsAstNode *n;
	struct JsAstPrimaryExpressionIdentNode *i;

	switch (NEXT) {
	case tTHIS:
		n = NEW_NODE(struct JsAstNode, NODECLASS_PrimaryExpression_this);
		SKIP;
		return n;
	case tIDENT:
		i = NEW_NODE(struct JsAstPrimaryExpressionIdentNode,
			NODECLASS_PrimaryExpression_ident);
		i->string = NEXT_VALUE->string;
		SKIP;
		return (struct JsAstNode *)i;
	case '[':
		PARSE(ArrayLiteral,n);
		return n;
	case '{':
		PARSE(ObjectLiteral,n);
		return n;
	case '(':
		SKIP;
		PARSE(Expression,n);
		EXPECT(')');
		return n;
	default:
		PARSE(Literal,n);
		return n;
	}
}

/* -- 7.8
 *	Literal:
 *	 	NullLiteral
 *	 	BooleanLiteral
 *	 	NumericLiteral
 *	 	StringLiteral
 *
 *	NullLiteral:
 *		tNULL				-- 7.8.1
 *
 *	BooleanLiteral:
 *		tTRUE				-- 7.8.2
 *		tFALSE				-- 7.8.2
 */

static struct JsAstNode *
Literal_parse(parser)
	struct JsParser *parser;
{
	struct JsAstLiteralNode *n;
	struct JsAstNode* p;
	/*
	 * Convert the next token into a regular expression
	 * if possible
	 */

	switch (NEXT) {
	case tNULL:
		n = NEW_NODE(struct JsAstLiteralNode, NODECLASS_Literal);
		n->value = (struct JsValue*) JsMalloc(sizeof(struct JsValue));
		n->value->type = JS_NULL;
		SKIP;
		return (struct JsAstNode *)n;
	case tTRUE:
	case tFALSE:
		n = NEW_NODE(struct JsAstLiteralNode,  NODECLASS_Literal);
		n->value = (struct JsValue*) JsMalloc(sizeof(struct JsValue));
		n->value->type = JS_BOOLEAN;
		n->value->u.boolean = (NEXT == tTRUE);
		SKIP;
		return (struct JsAstNode *)n;
	case tNUMBER:
		PARSE(NumericLiteral,p);
		return p;
	case tSTRING:
		PARSE(StringLiteral,p);
		return p;
	default:
		EXPECTED("null, true, false, number, string");
	}
	/* NOTREACHED */
}

/*
 *	NumericLiteral:
 *		tNUMBER				-- 7.8.3
 */
static struct JsAstNode *
NumericLiteral_parse(parser)
	struct JsParser *parser;
{
	struct JsAstLiteralNode *n;

	EXPECT_NOSKIP(tNUMBER);
	n = NEW_NODE(struct JsAstLiteralNode, NODECLASS_Literal);
	n->value = (struct JsValue*) JsMalloc(sizeof(struct JsValue));
	n->value->type = JS_NUMBER;
	n->value->u.number = NEXT_VALUE->number;
	SKIP;
	return (struct JsAstNode *)n;
}

/*
 *	StringLiteral:
 *		tSTRING				-- 7.8.4
 */
static struct JsAstNode *
StringLiteral_parse(parser)
	struct JsParser *parser;
{
	struct JsAstStringLiteralNode *n;

	EXPECT_NOSKIP(tSTRING);
	n = NEW_NODE(struct JsAstStringLiteralNode, NODECLASS_StringLiteral);
	n->string = NEXT_VALUE->string;
	SKIP;
	return (struct JsAstNode *)n;
}


/*
 *	-- 13
 *
 *	FunctionDeclaration
 *	:	tFUNCTION tIDENT '( ')' '{' FunctionBody '}'
 *	|	tFUNCTION tIDENT '( FormalParameterList ')' 
 *			'{' FunctionBody '}'
 *	;
 *
 *	FunctionExpression
 *	:	tFUNCTION '( ')' '{' FunctionBody '}'
 *	|	tFUNCTION '( FormalParameterList ')' '{' FunctionBody '}'
 *	|	tFUNCTION tIDENT '( ')' '{' FunctionBody '}'
 *	|	tFUNCTION tIDENT '( FormalParameterList ')' 
 *			'{' FunctionBody '}'
 *	;
 *
 *	FormalParameterList
 *	:	tIDENT
 *	|	FormalParameterList ',' tIDENT
 *	;
 *
 *	FunctionBody
 *	:	SourceElements
 *	;
 *
 *  Note: FunctionDeclaration semantics are never called, but defined in
 *  the spec. (Spec bug?)
 */
static struct JsAstNode *
FunctionExpression_parse(parser)
	struct JsParser *parser;
{
	struct JsAstFunctionNode *n;
	struct var *formal;
	int noin_save, is_lhs_save;
	char *name;
	struct JsAstNode *body;
	int count,i;
	struct var* p;
	/* Save parser state */
	noin_save = parser->noin;
	is_lhs_save = parser->is_lhs;
	parser->noin = 0;
	parser->is_lhs = 0;

	n = NEW_NODE(struct JsAstFunctionNode, NODECLASS_FunctionExpression);
	EXPECT(tFUNCTION);
	if (NEXT == tIDENT) {
		name = NEXT_VALUE->string;
		SKIP;
	} else
		name = NULL;
	EXPECT('(');
	PARSE(FormalParameterList,formal);
	EXPECT(')');

	EXPECT('{');
	parser->funcdepth++;
	PARSE(FunctionBody,body);
	parser->funcdepth--;
	EXPECT('}');
	//组织functionNode
	n->name = name;
	n->body = body;
	p  = formal;
	//参数个数
	count = 0 ;
	while(p != NULL){
		count ++;
		p = p->next;
	}
	n->argc = count;
	n->argv = (char**) JsMalloc(sizeof(char*) * count);
	i = 0;
	for( p = formal; i<count && p!= NULL ;++i){
		n->argv[i] = p->name;
		p = p->next;
	}
	/* Restore parser state */
	parser->noin = noin_save;
	parser->is_lhs = is_lhs_save;

	return (struct JsAstNode *)n;
}

static struct var*
FormalParameterList_parse(parser)
	struct JsParser *parser;
{
	struct var **p;
	struct var *result;

	p = &result;

	if (NEXT == tIDENT) {
	    *p = (struct var*)JsMalloc(sizeof(struct var));
	    (*p)->name = NEXT_VALUE->string;
	    p = &(*p)->next;
	    SKIP;
	    while (NEXT == ',') {
			SKIP;
			if (NEXT == tIDENT) {
				*p = (struct var*)JsMalloc(sizeof(struct var));
				(*p)->name = NEXT_VALUE->string;
				p = &(*p)->next;
			}
			EXPECT(tIDENT);
	    }
	}
	*p = NULL;
	return result;
}

static struct JsAstNode *
FunctionBody_parse(parser)
	struct JsParser *parser;
{
    struct JsAstFunctionBodyNode *n;

	n = NEW_NODE(struct JsAstFunctionBodyNode, NODECLASS_FunctionBody);
	PARSE(SourceElements,n->u.a);
	n->is_program = 0;
	return (struct JsAstNode *)n;
}
//-----------------------------------------------------
//接口函数

void JsParseFile(int debug, char* filename , struct JsAstNode** ast){
	struct JsAstNode* program ;
	struct JsParser* parser  ;
	yyscan_t lexer;
	//打开输入流
	FILE* file;
	if(filename == NULL){
		filename = "Console"; //默认为Console
		file = stdin;
	}else if((file = fopen(filename,"r")) == NULL){
		if(debug >= JS_PARSER_DEBUG_ERROR){
			printf("Parse: open file %s fail \n",filename); 
		}
		*ast = NULL;
		return ;
	}
	
	parser = (struct JsParser*) JsMalloc(sizeof(struct JsParser));
	memset(parser,0,sizeof(struct JsParser));
	
	
	//处理lexer
	if(yylex_init(&lexer)) {
		if(debug >= JS_PARSER_DEBUG_ERROR){
			printf("Parse:  init lexer fail \n"); 
		}
		*ast = NULL;
		return ;
	}
	yyset_in(file,lexer);
	parser->lexer = lexer;
	parser->filename =filename;
	
	if(debug >= JS_PARSER_DEBUG_ALL){
		//开启lexer的调试
		yyset_debug(1,lexer);
	}
	parser->debug = debug;
	parser->error  = 0;
	parser->noin = 0;
	parser->is_lhs = 0;
	parser->funcdepth = 0 ;
	//装载第一个Lookahead的token
	yylex0(&parser->value,&parser->token,
		&parser->hasLineToken,parser->lexer);
	
	//parse
	PARSE_NO_CHECK(Program,program);
	
	//析构
	if(file != stdin)
		fclose(file);
	yylex_destroy(lexer);
	
	if(parser->error != 0){
		*ast = NULL;
		return ;
	}
	*ast = program;
	return;
}


void JsParseString(int debug, char* string , struct JsAstNode** ast){
	struct JsAstNode* program ;
	struct JsParser* parser  ;
	YY_BUFFER_STATE bp ;
	yyscan_t lexer;
	if(string == NULL){
		if(debug >= JS_PARSER_DEBUG_ERROR){
			printf("Parse: String is NULL \n"); 
		}
		*ast = NULL;
		return ;
	}
	
	parser = (struct JsParser*) JsMalloc(sizeof(struct JsParser));
	memset(parser,0,sizeof(struct JsParser));
	
	
	//处理lexer
	if(yylex_init(&lexer)) {
		if(debug >= JS_PARSER_DEBUG_ERROR){
			printf("Parse:  init lexer fail \n"); 
		}
		*ast = NULL;
		return ;
	}
	bp = yy_scan_string(string,lexer);
	yy_switch_to_buffer(bp,lexer);
	
	parser->lexer = lexer;
	parser->filename = "StringText";
	
	if(debug >= JS_PARSER_DEBUG_ALL){
		//开启lexer的调试
		yyset_debug(1,lexer);
	}
	parser->debug = debug;
	parser->error  = 0;
	parser->noin = 0;
	parser->is_lhs = 0;
	parser->funcdepth = 0 ;

	//装载第一个Lookahead的token
	yylex0(&parser->value,&parser->token,
			&parser->hasLineToken,parser->lexer);
	
	//parse
	PARSE_NO_CHECK(Program,program);
	//析构
	yy_delete_buffer(bp,lexer);
	yylex_destroy(lexer);

	if(parser->error != 0){
		*ast = NULL;
		return ;
	}
	*ast = program;
	return;
}
