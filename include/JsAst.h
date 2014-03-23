#ifndef JsAstH
#define JsAstH

#include"JsType.h"

struct JsValue;
struct JsLocation;

enum JsAstClassEnum {
    NODECLASS_None                               =  0,
	NODECLASS_Unary                              =  1,
	NODECLASS_Binary                             =  2,
	NODECLASS_Literal                            =  3,
	NODECLASS_StringLiteral                      =  4,
	NODECLASS_RegularExpressionLiteral           =  5,
	NODECLASS_PrimaryExpression_this             =  6,
	NODECLASS_PrimaryExpression_ident            =  7,
	NODECLASS_ArrayLiteral                       =  8,
	NODECLASS_ObjectLiteral                      =  9,
	NODECLASS_Arguments                          = 10,
	NODECLASS_MemberExpression_new               = 11,
	NODECLASS_MemberExpression_dot               = 12,
	NODECLASS_MemberExpression_bracket           = 13,
	NODECLASS_CallExpression                     = 14,
	NODECLASS_PostfixExpression_inc              = 15,
	NODECLASS_PostfixExpression_dec              = 16,
	NODECLASS_UnaryExpression_delete             = 17,
	NODECLASS_UnaryExpression_void               = 18,
	NODECLASS_UnaryExpression_typeof             = 19,
	NODECLASS_UnaryExpression_preinc             = 20,
	NODECLASS_UnaryExpression_predec             = 21,
	NODECLASS_UnaryExpression_plus               = 22,
	NODECLASS_UnaryExpression_minus              = 23,
	NODECLASS_UnaryExpression_inv                = 24,
	NODECLASS_UnaryExpression_not                = 25,
	NODECLASS_MultiplicativeExpression_mul       = 26,
	NODECLASS_MultiplicativeExpression_div       = 27,
	NODECLASS_MultiplicativeExpression_mod       = 28,
	NODECLASS_AdditiveExpression_add             = 29,
	NODECLASS_AdditiveExpression_sub             = 30,
	NODECLASS_ShiftExpression_lshift             = 31,
	NODECLASS_ShiftExpression_rshift             = 32,
	NODECLASS_ShiftExpression_urshift            = 33,
	NODECLASS_RelationalExpression_lt            = 34,
	NODECLASS_RelationalExpression_gt            = 35,
	NODECLASS_RelationalExpression_le            = 36,
	NODECLASS_RelationalExpression_ge            = 37,
	NODECLASS_RelationalExpression_instanceof    = 38,
	NODECLASS_RelationalExpression_in            = 39,
	NODECLASS_EqualityExpression_eq              = 40,
	NODECLASS_EqualityExpression_ne              = 41,
	NODECLASS_EqualityExpression_seq             = 42,
	NODECLASS_EqualityExpression_sne             = 43,
	NODECLASS_BitwiseANDExpression               = 44,
	NODECLASS_BitwiseXORExpression               = 45,
	NODECLASS_BitwiseORExpression                = 46,
	NODECLASS_LogicalANDExpression               = 47,
	NODECLASS_LogicalORExpression                = 48,
	NODECLASS_ConditionalExpression              = 49,
	NODECLASS_AssignmentExpression               = 50,
	NODECLASS_AssignmentExpression_simple        = 51,
	NODECLASS_AssignmentExpression_muleq         = 52,
	NODECLASS_AssignmentExpression_diveq         = 53,
	NODECLASS_AssignmentExpression_modeq         = 54,
	NODECLASS_AssignmentExpression_addeq         = 55,
	NODECLASS_AssignmentExpression_subeq         = 56,
	NODECLASS_AssignmentExpression_lshifteq      = 57,
	NODECLASS_AssignmentExpression_rshifteq      = 58,
	NODECLASS_AssignmentExpression_urshifteq     = 59,
	NODECLASS_AssignmentExpression_andeq         = 60,
	NODECLASS_AssignmentExpression_xoreq         = 61,
	NODECLASS_AssignmentExpression_oreq          = 62,
	NODECLASS_Expression_comma                   = 63,
	NODECLASS_Block_empty                        = 64,
	NODECLASS_StatementList                      = 65,
	NODECLASS_VariableStatement                  = 66,
	NODECLASS_VariableDeclarationList            = 67,
	NODECLASS_VariableDeclaration                = 68,
	NODECLASS_EmptyStatement                     = 69,
	NODECLASS_ExpressionStatement                = 70,
	NODECLASS_IfStatement                        = 71,
	NODECLASS_IterationStatement_dowhile         = 72,
	NODECLASS_IterationStatement_while           = 73,
	NODECLASS_IterationStatement_for             = 74,
	NODECLASS_IterationStatement_forvar          = 75,
	NODECLASS_IterationStatement_forin           = 76,
	NODECLASS_IterationStatement_forvarin        = 77,
	NODECLASS_ContinueStatement                  = 78,
	NODECLASS_BreakStatement                     = 79,
	NODECLASS_ReturnStatement                    = 80,
	NODECLASS_ReturnStatement_undef              = 81,
	NODECLASS_WithStatement                      = 82,
	NODECLASS_SwitchStatement                    = 83,
	NODECLASS_LabelledStatement                  = 84,
	NODECLASS_ThrowStatement                     = 85,
	NODECLASS_TryStatement                       = 86,
	NODECLASS_TryStatement_catch                 = 87,
	NODECLASS_TryStatement_finally               = 88,
	NODECLASS_TryStatement_catchfinally          = 89,
	NODECLASS_Function                           = 90,
	NODECLASS_FunctionDeclaration                = 91,
	NODECLASS_FunctionExpression                 = 92,
	NODECLASS_FunctionBody                       = 93,
	NODECLASS_SourceElements                     = 94
	#define NODECLASS_MAX                        95
};



struct JsAstNode {
        enum JsAstClassEnum astClass;
        struct JsLocation* location;     /* source location */
};
 
struct JsAstLiteralNode {
	struct JsAstNode node;
	struct JsValue* value;
};

struct JsAstStringLiteralNode {
	struct JsAstNode node;
	char *string;
};

struct JsAstPrimaryExpressionIdentNode {
	struct JsAstNode node;
	char *string;
};

struct JsAstArrayLiteralNode {
	struct JsAstNode node;
	int length;
	struct JsAstArrayLiteralElement {
		int index;
		struct JsAstNode *expr;
		struct JsAstArrayLiteralElement *next;
	} *first;
};

struct JsAstObjectLiteralNode {
	struct JsAstNode node;
	struct JsAstObjectLiteralPair {
		struct JsAstNode *value;
		struct JsAstObjectLiteralPair *next;
		char *name;
	} *first;
};

struct JsAstArgumentsNode {				/* declare for early use */
	struct JsAstNode node;
	int	argc;
	struct JsAstArgumentsArg {
		struct JsAstNode *expr;
		struct JsAstArgumentsArg *next;
	} *first;
};

struct JsAstMemberExpressionNewNode {
	struct JsAstNode node;
	struct JsAstNode *mexp;
	struct JsAstArgumentsNode *args;
};

struct JsAstMemberExpressionDotNode {
	struct JsAstNode node;
	struct JsAstNode *mexp;
	char *name;
};

struct JsAstMemberExpressionBracketNode {
	struct JsAstNode node;
	struct JsAstNode *mexp, *name;
};

struct JsAstCallExpressionNode {
	struct JsAstNode node;
	struct JsAstNode *exp;
	struct JsAstArgumentsNode *args;
};

struct JsAstUnaryNode {
	struct JsAstNode node;
	struct JsAstNode *a;
};

struct JsAstBinaryNode {
	struct JsAstNode node;
	struct JsAstNode *a, *b;
};

struct JsAstConditionalExpressionNode {
	struct JsAstNode node;
	struct JsAstNode *a, *b, *c;
};

struct JsAstAssignmentExpressionNode {
	struct JsAstNode node;
	struct JsAstNode *lhs, *expr;
};

struct JsAstVariableDeclarationNode {
	struct JsAstNode node;
	char* var;
	struct JsAstNode *init;
};

struct JsAstIfStatementNode {
	struct JsAstNode node;
	struct JsAstNode *cond, *btrue, *bfalse;
};

struct JsAstIterationStatementWhileNode {
	struct JsAstNode  node;
	unsigned int target;
	struct JsAstNode *cond, *body;
};

struct JsAstIterationStatementForNode {
	struct JsAstNode node;
	unsigned int target;
	struct JsAstNode *init, *cond, *incr, *body;
};

struct JsAstIterationStatementForinNode {
	struct JsAstNode node;
	unsigned int target;
	struct JsAstNode *lhs, *list, *body;
};

struct JsAstContinueStatementNode {
	struct JsAstNode node;
	unsigned int target;
};

struct JsAstBreakStatementNode {
	struct JsAstNode node;
	unsigned int target;
};

struct JsAstReturnStatementNode {
	struct JsAstNode node;
	struct JsAstNode *expr;
};

struct JsAstSwitchStatementNode {
	struct JsAstNode node;
	unsigned int target;
	struct JsAstNode *cond;
	struct JsAstCaseList {
		struct JsAstNode *expr;	/* NULL for default case */
		struct JsAstNode *body;
		struct JsAstCaseList *next;
	} *cases, *defcase;
};

struct JsAstLabelledStatementNode {
	struct JsAstUnaryNode unary;
	unsigned int target;
};

struct JsAstTryStatementNode {
	struct JsAstNode node;
	struct JsAstNode *block, *bcatch, *bfinally;
	char *ident;
};

struct JsAstFunctionNode {
	struct JsAstNode node;
	int argc;
	char** argv;
	char* name;
	struct JsAstNode* body; 
};

struct JsAstFunctionBodyNode {
	struct JsAstUnaryNode u;
	int is_program;
};

struct JsAstSourceElementsNode {
	struct JsAstNode node;
	struct JsAstSourceElement {
		struct JsAstNode *node;
		struct JsAstSourceElement *next;
	} *statements;
};

#endif