#include"JsObject.h"
#include"JsContext.h"
#include"JsEngine.h"
#include"JsVm.h"
#include"JsList.h"
#include"JsValue.h"
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
#include<math.h>
#include<setjmp.h>

/**Eval Function*/
static void Literal_eval(struct JsAstNode *na, struct JsContext *context, 
        struct JsValue *res);
static void StringLiteral_eval(struct JsAstNode *na, struct JsContext *context, 
        struct JsValue *res);
static void PrimaryExpression_this_eval(struct JsAstNode *n, 
        struct JsContext *context, struct JsValue *res);
static void PrimaryExpression_ident_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void ArrayLiteral_eval(struct JsAstNode *na, struct JsContext *context, 
        struct JsValue *res);
static void ObjectLiteral_eval(struct JsAstNode *na, struct JsContext *context, 
        struct JsValue *res);
static void Arguments_eval(struct JsAstNode *na, struct JsContext *context, 
        struct JsValue *res);
static void MemberExpression_new_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void MemberExpression_dot_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void MemberExpression_bracket_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void CallExpression_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void PostfixExpression_inc_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void PostfixExpression_dec_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void UnaryExpression_delete_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void UnaryExpression_void_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void UnaryExpression_typeof_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void UnaryExpression_preinc_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void UnaryExpression_predec_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void UnaryExpression_plus_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void UnaryExpression_minus_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void UnaryExpression_inv_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void UnaryExpression_not_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void MultiplicativeExpression_mul_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void MultiplicativeExpression_div_common(struct JsValue *r2, 
        struct JsValue *r4, struct JsContext *context, 
	struct JsValue *res);
static void MultiplicativeExpression_div_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void MultiplicativeExpression_mod_common(struct JsValue *r2, 
        struct JsValue *r4, struct JsContext *context, 
		struct JsValue *res);
static void MultiplicativeExpression_mul_common(struct JsValue *r2, 
        struct JsValue *r4, struct JsContext *context, 
		struct JsValue *res);
static void MultiplicativeExpression_mod_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void AdditiveExpression_add_common(struct JsValue *r2, 
        struct JsValue *r4, struct JsContext *context,
		struct JsValue *res);
static void AdditiveExpression_add_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void AdditiveExpression_sub_common(struct JsValue *r2, 
        struct JsValue *r4, struct JsContext *context,
		struct JsValue *res);
static void AdditiveExpression_sub_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void ShiftExpression_lshift_common(struct JsValue *r2, 
        struct JsAstNode *bn, struct JsContext *context, struct JsValue *res);
static void ShiftExpression_lshift_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void ShiftExpression_rshift_common(struct JsValue *r2, 
        struct JsValue *r4, struct JsContext *context, 
		struct JsValue *res);
static void ShiftExpression_rshift_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void ShiftExpression_urshift_common(struct JsValue *r2, 
        struct JsValue *r4, struct JsContext *context,
		struct JsValue *res);
static void ShiftExpression_urshift_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void RelationalExpression_lt_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void RelationalExpression_gt_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void RelationalExpression_le_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void RelationalExpression_ge_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void RelationalExpression_instanceof_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void RelationalExpression_in_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void EqualityExpression_eq_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void EqualityExpression_ne_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void EqualityExpression_seq_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void EqualityExpression_sne_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void BitwiseANDExpression_common(struct JsValue *r2, 
        struct JsValue *r4, struct JsContext *context, 
		struct JsValue *res);
static void BitwiseANDExpression_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void BitwiseXORExpression_common(struct JsValue *r2, 
        struct JsValue *r4, struct JsContext *context, 
		struct JsValue *res);
static void BitwiseXORExpression_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void BitwiseORExpression_common(struct JsValue *r2, 
        struct JsValue *r4, struct JsContext *context,
		struct JsValue *res);
static void BitwiseORExpression_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void LogicalANDExpression_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void LogicalORExpression_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void ConditionalExpression_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void AssignmentExpression_simple_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void AssignmentExpression_muleq_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void AssignmentExpression_diveq_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void AssignmentExpression_modeq_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void AssignmentExpression_addeq_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void AssignmentExpression_subeq_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void AssignmentExpression_lshifteq_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void AssignmentExpression_rshifteq_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void AssignmentExpression_urshifteq_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void AssignmentExpression_andeq_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void AssignmentExpression_xoreq_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void AssignmentExpression_oreq_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void Expression_comma_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void Block_empty_eval(struct JsAstNode *n, struct JsContext *context, 
        struct JsValue *res);
static void StatementList_eval(struct JsAstNode *na, struct JsContext *context, 
        struct JsValue *res);
static void VariableStatement_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void VariableDeclarationList_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void VariableDeclaration_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void EmptyStatement_eval(struct JsAstNode *n, struct JsContext *context, 
        struct JsValue *res);
static void ExpressionStatement_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void IfStatement_eval(struct JsAstNode *na, struct JsContext *context, 
        struct JsValue *res);
static void IterationStatement_dowhile_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void IterationStatement_while_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void IterationStatement_for_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void IterationStatement_forin_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void IterationStatement_forvarin_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void ContinueStatement_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void BreakStatement_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void ReturnStatement_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void ReturnStatement_undef_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void SwitchStatement_caseblock(struct JsAstSwitchStatementNode *n, 
        struct JsContext *context, struct JsValue *input, 
        struct JsValue *res);
static void SwitchStatement_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void ThrowStatement_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static struct JsValue* TryStatement_help(struct JsAstTryStatementNode *n, 
        struct JsContext *context, struct JsValue *C, 
        struct JsValue *res);
static void TryStatement_catch_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void TryStatement_finally_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void TryStatement_catchfinally_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void FunctionExpression_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void FunctionBody_eval(struct JsAstNode *na, struct JsContext *context, 
        struct JsValue *res);
static void SourceElements_eval(struct JsAstNode *na, 
        struct JsContext *context, struct JsValue *res);
static void CallExpression_eval_common(struct JsContext *,
		struct JsLocation*, struct JsValue *, int, 
		struct JsValue **, struct JsValue *);
static void UnaryExpression_delete_eval_common(struct JsContext *,
		struct JsValue *, struct JsValue *);
static void UnaryExpression_typeof_eval_common(struct JsContext *,
		struct JsValue *, struct JsValue *);
static void UnaryExpression_inv_eval_common(struct JsContext *,
		struct JsValue *, struct JsValue *);
static void JsFunctionCall(struct JsEngine* e,void* data,struct JsValue* res);

//JsAstClassEnum 对应的处理函数
static void (*JsNodeClassEval[NODECLASS_MAX])(struct JsAstNode* ,
	struct JsContext*,struct JsValue*) = {
	 NULL
	,NULL                                   /*Unary*/
	,NULL                                   /*Binary*/
    ,Literal_eval                           /*Literal*/
    ,StringLiteral_eval                     /*StringLiteral*/
    ,NULL					                /*RegularExpressionLiteral*/
    ,PrimaryExpression_this_eval            /*PrimaryExpression_this*/
    ,PrimaryExpression_ident_eval           /*PrimaryExpression_ident*/
    ,ArrayLiteral_eval                      /*ArrayLiteral*/
    ,ObjectLiteral_eval                     /*ObjectLiteral*/
    ,Arguments_eval                         /*Arguments*/
    ,MemberExpression_new_eval              /*MemberExpression_new*/
    ,MemberExpression_dot_eval              /*MemberExpression_dot*/
    ,MemberExpression_bracket_eval          /*MemberExpression_bracket*/
    ,CallExpression_eval                    /*CallExpression*/
    ,PostfixExpression_inc_eval             /*PostfixExpression_inc*/
    ,PostfixExpression_dec_eval             /*PostfixExpression_dec*/
    ,UnaryExpression_delete_eval            /*UnaryExpression_delete*/
    ,UnaryExpression_void_eval              /*UnaryExpression_void*/
    ,UnaryExpression_typeof_eval            /*UnaryExpression_typeof*/
    ,UnaryExpression_preinc_eval            /*UnaryExpression_preinc*/
    ,UnaryExpression_predec_eval            /*UnaryExpression_predec*/
    ,UnaryExpression_plus_eval              /*UnaryExpression_plus*/
    ,UnaryExpression_minus_eval             /*UnaryExpression_minus*/
    ,UnaryExpression_inv_eval               /*UnaryExpression_inv*/
    ,UnaryExpression_not_eval               /*UnaryExpression_not*/
    ,MultiplicativeExpression_mul_eval      /*MultiplicativeExpression_mul*/
    ,MultiplicativeExpression_div_eval      /*MultiplicativeExpression_div*/
    ,MultiplicativeExpression_mod_eval      /*MultiplicativeExpression_mod*/
    ,AdditiveExpression_add_eval            /*AdditiveExpression_add*/
    ,AdditiveExpression_sub_eval            /*AdditiveExpression_sub*/
    ,ShiftExpression_lshift_eval            /*ShiftExpression_lshift*/
    ,ShiftExpression_rshift_eval            /*ShiftExpression_rshift*/
    ,ShiftExpression_urshift_eval           /*ShiftExpression_urshift*/
    ,RelationalExpression_lt_eval           /*RelationalExpression_lt*/
    ,RelationalExpression_gt_eval           /*RelationalExpression_gt*/
    ,RelationalExpression_le_eval           /*RelationalExpression_le*/
    ,RelationalExpression_ge_eval           /*RelationalExpression_ge*/
    ,RelationalExpression_instanceof_eval   /*RelationalExpression_instanceof*/
    ,RelationalExpression_in_eval           /*RelationalExpression_in*/
    ,EqualityExpression_eq_eval             /*EqualityExpression_eq*/
    ,EqualityExpression_ne_eval             /*EqualityExpression_ne*/
    ,EqualityExpression_seq_eval            /*EqualityExpression_seq*/
    ,EqualityExpression_sne_eval            /*EqualityExpression_sne*/
    ,BitwiseANDExpression_eval              /*BitwiseANDExpression*/
    ,BitwiseXORExpression_eval              /*BitwiseXORExpression*/
    ,BitwiseORExpression_eval               /*BitwiseORExpression*/
    ,LogicalANDExpression_eval              /*LogicalANDExpression*/
    ,LogicalORExpression_eval               /*LogicalORExpression*/
    ,ConditionalExpression_eval             /*ConditionalExpression*/
    ,NULL                                   /*AssignmentExpression*/
    ,AssignmentExpression_simple_eval       /*AssignmentExpression_simple*/
    ,AssignmentExpression_muleq_eval        /*AssignmentExpression_muleq*/
    ,AssignmentExpression_diveq_eval        /*AssignmentExpression_diveq*/
    ,AssignmentExpression_modeq_eval        /*AssignmentExpression_modeq*/
    ,AssignmentExpression_addeq_eval        /*AssignmentExpression_addeq*/
    ,AssignmentExpression_subeq_eval        /*AssignmentExpression_subeq*/
    ,AssignmentExpression_lshifteq_eval     /*AssignmentExpression_lshifteq*/
    ,AssignmentExpression_rshifteq_eval     /*AssignmentExpression_rshifteq*/
    ,AssignmentExpression_urshifteq_eval    /*AssignmentExpression_urshifteq*/
    ,AssignmentExpression_andeq_eval        /*AssignmentExpression_andeq*/
    ,AssignmentExpression_xoreq_eval        /*AssignmentExpression_xoreq*/
    ,AssignmentExpression_oreq_eval         /*AssignmentExpression_oreq*/
    ,Expression_comma_eval                  /*Expression_comma*/
    ,Block_empty_eval                       /*Block_empty*/
    ,StatementList_eval                     /*StatementList*/
    ,VariableStatement_eval                 /*VariableStatement*/
    ,VariableDeclarationList_eval           /*VariableDeclarationList*/
    ,VariableDeclaration_eval               /*VariableDeclaration*/
    ,EmptyStatement_eval                    /*EmptyStatement*/
    ,ExpressionStatement_eval               /*ExpressionStatement*/
    ,IfStatement_eval                       /*IfStatement*/
    ,IterationStatement_dowhile_eval        /*IterationStatement_dowhile*/
    ,IterationStatement_while_eval          /*IterationStatement_while*/
    ,IterationStatement_for_eval            /*IterationStatement_for*/
    ,IterationStatement_for_eval            /*IterationStatement_forvar*/
    ,IterationStatement_forin_eval          /*IterationStatement_forin*/
    ,IterationStatement_forvarin_eval       /*IterationStatement_forvarin*/
    ,ContinueStatement_eval                 /*ContinueStatement*/
    ,BreakStatement_eval                    /*BreakStatement*/
    ,ReturnStatement_eval                   /*ReturnStatement*/
    ,ReturnStatement_undef_eval             /*ReturnStatement_undef*/
    ,NULL				                    /*WithStatement*/
    ,SwitchStatement_eval                   /*SwitchStatement*/
    ,NULL				                    /*LabelledStatement*/
    ,ThrowStatement_eval                    /*ThrowStatement*/
    ,NULL                                   /*TryStatement*/
    ,TryStatement_catch_eval                /*TryStatement_catch*/
    ,TryStatement_finally_eval              /*TryStatement_finally*/
    ,TryStatement_catchfinally_eval         /*TryStatement_catchfinally*/
    ,NULL                                   /*Function*/
    ,NULL /* FunctionDeclaration_eval */    /*FunctionDeclaration*/
    ,FunctionExpression_eval                /*FunctionExpression*/
    ,FunctionBody_eval                      /*FunctionBody*/
    ,SourceElements_eval                    /*SourceElements*/
};	


/*EVAL Marco*/
#define EVALFN(node) JsNodeClassEval[(node)->astClass]

# define EVAL(node, ctxt, res)                          \
    do {                                                \
        (*EVALFN(node))(node, ctxt, res);               \
    } while (0)
	
/*CAST NODE*/
#define CAST_NODE(node,type) \
	((struct type*)node)
	
#define JS_SET_COMPLETION(res,t,v ) 	\
	do{									\
		res->type = JS_COMPLETION; 		\
		res->u.completion.type = t; 	\
		res->u.completion.value = v; 	\
	}while(0)

//跟踪最新的语句
#define TRACE(loc,context, event)	\
	do{							\
		context->pc = loc;		\
		if(JsGetVm()->debug == TRUE && JsGetVm()->trace != NULL){	\
			JsGetVm()->trace(context->engine,loc,event);	\
		}	\
	}while(0)
/*
	e->exec 为执行的状态
	注意修改Engine的状态
	如果不想知道res, 则使用NULL
*/
void JsEval(struct JsEngine* e,struct JsAstNode* ast, struct JsValue* res){
	EVAL(ast,e->exec,res);
}

/*********                    static                       **********/
/* 14 */
static void
SourceElements_eval(na, context, res)
	struct JsAstNode *na; /* (struct SourceElements_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstSourceElementsNode *n = CAST_NODE(na, JsAstSourceElementsNode);
	struct JsAstSourceElement *e;

	/*
	 * NB: strictly, this should 'evaluate' the
	 * FunctionDeclarations, but they only yield <NORMAL, NULL, NULL>
	 * so, we don't. We just run the non-functiondecl statements
	 * instead. It has the same result.
	 */
	JS_SET_COMPLETION(res,JS_COMPLETION_NORMAL, NULL);
	for (e = n->statements; e; e = e->next) {
		EVAL(e->node, context, res);
		if (res->u.completion.type != JS_COMPLETION_NORMAL)
			break;
	}
}

/* 12.1 */
static void
Block_empty_eval(n, context, res)
	struct JsAstNode *n;
	struct JsContext *context;
	struct JsValue *res;
{
	JS_SET_COMPLETION(res,JS_COMPLETION_NORMAL, NULL);
}


/* 12.1 */
static void
StatementList_eval(na, context, res)
	struct JsAstNode *na; /* (struct Binary_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue *val;

	EVAL(n->a, context, res);
	if (res->u.completion.type == JS_COMPLETION_NORMAL) {
		val = res->u.completion.value;
		EVAL(n->b, context, res);
		if (res->u.completion.value == NULL)
			res->u.completion.value = val;
	}
}


/* 12.2 */
static void
VariableStatement_eval(na, context, res)
	struct JsAstNode *na; /* (struct Unary_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstUnaryNode *n = CAST_NODE(na, JsAstUnaryNode);
	struct JsValue v;

	TRACE(na->location,context, JS_TRACE_STATEMENT);
	EVAL(n->a, context, &v);
	JS_SET_COMPLETION(res,JS_COMPLETION_NORMAL, NULL);
}

/* 12.2 */
static void
VariableDeclarationList_eval(na, context, res)
	struct JsAstNode *na; /* (struct Binary_node) */
	struct JsContext *context;
	struct JsValue *res;		/* unused */
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);

	EVAL(n->a, context, res);
	EVAL(n->b, context, res);
}


/* 12.2 */
static void
VariableDeclaration_eval(na, context, res)
	struct JsAstNode *na; /* (struct VariableDeclaration_node) */
	struct JsContext *context;
	struct JsValue *res;		/* unused */
{
	struct JsAstVariableDeclarationNode *n = 
		CAST_NODE(na, JsAstVariableDeclarationNode);
	struct JsValue r2, r3;
	/*
		遇到
		var a;
		var a = 1;
		function a(){
		
		};
		的时候, 直接在当前scope中添加a = undefined; ...
	*/
	if (n->init) {
		EVAL(n->init, context, &r2);
	}else{
		//var a; 类型
		r2.type = JS_UNDEFINED;
	}
	JsGetValue(&r2, &r3);
	//直接添加到当前Scope中
	struct JsObject* top = (struct JsObject*)JsListGet(context->scope,JS_LIST_END);
	(*top->Put)(top,n->var,&r3,context->varattr);
}
/* 12.3 */
static void
EmptyStatement_eval(na, context, res)
	struct JsAstNode *na;
	struct JsContext *context;
	struct JsValue *res;
{
	TRACE(na->location, context, JS_TRACE_STATEMENT);
	JS_SET_COMPLETION(res, JS_COMPLETION_NORMAL, NULL);
}

/* 12.4 */
static void
ExpressionStatement_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstUnaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstUnaryNode *n = CAST_NODE(na, JsAstUnaryNode);
	struct JsValue r1;
	struct JsValue *v = (struct JsValue*)JsMalloc(sizeof(struct JsValue));

	TRACE(na->location, context, JS_TRACE_STATEMENT);
	EVAL(n->a, context, &r1);
	JsGetValue(&r1, v);
	JS_SET_COMPLETION(res, JS_COMPLETION_NORMAL, v);
}


/* 12.5 */
static void
IfStatement_eval(na, context, res)
	struct JsAstNode *na; /* (struct IfStatement_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstIfStatementNode *n = CAST_NODE(na, JsAstIfStatementNode);
	struct JsValue r1, r2, r3;

	TRACE(na->location, context, JS_TRACE_STATEMENT);
	EVAL(n->cond, context, &r1);
	JsGetValue( &r1, &r2);
	JsToBoolean(&r2, &r3);
	if (r3.u.boolean == TRUE)
		EVAL(n->btrue, context, res);
	else if (n->bfalse)
		EVAL(n->bfalse, context, res);
	else
		JS_SET_COMPLETION(res, JS_COMPLETION_NORMAL, NULL);
}

/* 12.6.1 */
static void
IterationStatement_dowhile_eval(na, context, res)
	struct JsAstNode *na; /* (struct IterationStatement_while_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstIterationStatementWhileNode *n = 
		CAST_NODE(na, JsAstIterationStatementWhileNode);
	struct JsValue *v, r7, r8, r9;
	v = NULL;
	
step2:	
	EVAL(n->body, context, res);
	if (res->u.completion.value)
	    v = res->u.completion.value;
	//continue
	if (res->u.completion.type == JS_COMPLETION_CONTINUE)
	    goto step7;
	//break
	if (res->u.completion.type == JS_COMPLETION_BREAK)
	    goto step11;
	//return throw
	if (res->u.completion.type != JS_COMPLETION_NORMAL)
	    goto out;
step7: 
	TRACE(na->location, context, JS_TRACE_STATEMENT);
 	EVAL(n->cond, context, &r7);
	JsGetValue( &r7, &r8);
	JsToBoolean(&r8, &r9);
	if (r9.u.boolean == TRUE)
	    goto step2;
step11:
	JS_SET_COMPLETION(res, JS_COMPLETION_NORMAL, v);
out: 
	return;
}



/* 12.6.2 */
static void
IterationStatement_while_eval(na, context, res)
	struct JsAstNode *na; /* (struct IterationStatement_while_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstIterationStatementWhileNode *n = 
		CAST_NODE(na, JsAstIterationStatementWhileNode);
	struct JsValue *v, r2, r3, r4;

	v = NULL;
step2: 
	TRACE(na->location, context, JS_TRACE_STATEMENT);
 	EVAL(n->cond, context, &r2);
	JsGetValue( &r2, &r3);
	JsToBoolean(&r3, &r4);
	if (r4.u.boolean == FALSE) {
	    JS_SET_COMPLETION(res, JS_COMPLETION_NORMAL, v);
	    return;
	}
	EVAL(n->body, context, res);
	if (res->u.completion.value)
		v = res->u.completion.value;
	if (res->u.completion.type == JS_COMPLETION_CONTINUE)
		goto step2;
	if (res->u.completion.type == JS_COMPLETION_BREAK){
	    JS_SET_COMPLETION(res, JS_COMPLETION_NORMAL, v);
	    return;
	}
	if (res->u.completion.type != JS_COMPLETION_NORMAL)
	    return;
	goto step2;
}


/* 12.6 for( ;;) for(var ;;)*/
static void
IterationStatement_for_eval(na, context, res)
	struct JsAstNode *na; /* (struct IterationStatement_for_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstIterationStatementForNode *n = 
		CAST_NODE(na, JsAstIterationStatementForNode);
	struct JsValue *v, r2, r3, r6, r7, r8, r16, r17;

	if (n->init) {
	    TRACE(n->init->location, context, JS_TRACE_STATEMENT);
	    EVAL(n->init, context, &r2);
	    JsGetValue( &r2, &r3);		/* r3 not used */
	}
	v = NULL;
step5:	
	if (n->cond) {
	    TRACE(n->cond->location, context, JS_TRACE_STATEMENT);
	    EVAL(n->cond, context, &r6);
	    JsGetValue( &r6, &r7);
	    JsToBoolean(&r7, &r8);
	    if (r8.u.boolean == FALSE)
			goto step19;
	}else{
	    TRACE(na->location, context, JS_TRACE_STATEMENT);
	}
	EVAL(n->body, context, res);
	if (res->u.completion.value)
	    v = res->u.completion.value;
	//break
	if (res->u.completion.type == JS_COMPLETION_BREAK)
		goto step19;
	//break
	if (res->u.completion.type == JS_COMPLETION_CONTINUE)
		goto step15;
	//return or throw
	if (res->u.completion.type != JS_COMPLETION_NORMAL)
		return;
step15: 
	if (n->incr) {
	    TRACE(n->incr->location, context, JS_TRACE_STATEMENT);
	    EVAL(n->incr, context, &r16);
	    JsGetValue( &r16, &r17);	/* r17 not used */
	}
	goto step5;
step19:	
	JS_SET_COMPLETION(res, JS_COMPLETION_NORMAL, v);
}


/* 12.6 for(in) */
static void
IterationStatement_forin_eval(na, context, res)
	struct JsAstNode *na; /* (struct IterationStatement_forin_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstIterationStatementForinNode *n = 
		CAST_NODE(na, JsAstIterationStatementForinNode);

	struct JsValue *v, r1, r2, r3, r5, r6, v0;

    TRACE(na->location, context, JS_TRACE_STATEMENT);
	EVAL(n->list, context, &r1);
	JsGetValue(&r1, &r2);
	JsToObject(&r2, &r3);
	v = NULL;

	JsIter iter;
	//必须有新指针
	struct JsObject* next = r3.u.object;
	char* prop = (*next->NextValue)(&next,&iter,FALSE);
	for(; prop != NULL && next != NULL;prop = (*next->NextValue)(&next,&iter,TRUE)){
		
		(*r3.u.object->HasProperty)(r3.u.object,prop,&v0);
		if(v0.u.boolean == FALSE)
		    continue;	/* property was deleted! */
	    r5.type = JS_STRING;
		r5.u.string = prop;
		
	    EVAL(n->lhs, context, &r6);
	    JsPutValue(&r6, &r5, &v0);
	    EVAL(n->body, context, res);
	    if (res->u.completion.value)
			v = res->u.completion.value;
	    if (res->u.completion.type == JS_COMPLETION_BREAK)
		    break;
	    if (res->u.completion.type == JS_COMPLETION_CONTINUE)
		    continue;
	    if (res->u.completion.type != JS_COMPLETION_NORMAL)
		    return;
	}
	JS_SET_COMPLETION(res, JS_COMPLETION_NORMAL, v);
}


/* 12.6 for(var in) */
static void
IterationStatement_forvarin_eval(na, context, res)
	struct JsAstNode *na; /* (struct IterationStatement_forin_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstIterationStatementForinNode *n = 
		CAST_NODE(na, JsAstIterationStatementForinNode);

	struct JsValue *v, r2, r3, r4, r6, r7, v0;

	struct JsAstVariableDeclarationNode *lhs  = 
		CAST_NODE(n->lhs, JsAstVariableDeclarationNode);

	TRACE(na->location, context, JS_TRACE_STATEMENT);
	EVAL(n->lhs, context, &v0);
	EVAL(n->list, context, &r2);
	JsGetValue(&r2, &r3);
	JsToObject(&r3, &r4);
	v = NULL;
	
	JsIter iter;
	//必须有新指针
	struct JsObject* next = r4.u.object;
	char* prop = (*next->NextValue)(&next,&iter,FALSE);
	for(;prop != NULL && next != NULL; prop = (*next->NextValue)(&next,&iter,TRUE)){
		
		(*r4.u.object->HasProperty)(r4.u.object,prop,&v0);
		if(v0.u.boolean == FALSE)
		    continue;	/* property was deleted! */
	    r6.type = JS_STRING;
		r6.u.string = prop;
		
	    /* spec bug: "see 0" in step 7 */
	    JsFindValueRef(context,lhs->var, &r7);
	    JsPutValue(&r7, &r6,&v0);
	    EVAL(n->body, context, res);
	    if (res->u.completion.value)
			v = res->u.completion.value;
	    if (res->u.completion.type == JS_COMPLETION_BREAK)
		    break;
	    if (res->u.completion.type == JS_COMPLETION_CONTINUE)
		    continue;
	    if (res->u.completion.type != JS_COMPLETION_NORMAL)
		    return;
	}
	JS_SET_COMPLETION(res, JS_COMPLETION_NORMAL, v);
}

/* 12.7 */
static void
ContinueStatement_eval(na, context, res)
	struct JsAstNode *na; /* (struct ContinueStatement_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	//struct JsAstContinueStatementNode *n = CAST_NODE(na, JsAstContinueStatementNode);

	TRACE(na->location, context, JS_TRACE_STATEMENT);
	JS_SET_COMPLETION(res, JS_COMPLETION_CONTINUE, NULL);
}

/* 12.8 */
static void
BreakStatement_eval(na, context, res)
	struct JsAstNode *na; /* (struct BreakStatement_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	//struct BreakStatement_node *n = CAST_NODE(na, BreakStatement);

	TRACE(na->location, context, JS_TRACE_STATEMENT);
	JS_SET_COMPLETION(res, JS_COMPLETION_BREAK, NULL);
}


/* 12.9 */
static void
ReturnStatement_eval(na, context, res)
	struct JsAstNode *na; /* (struct ReturnStatement_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstReturnStatementNode *n = CAST_NODE(na, JsAstReturnStatementNode);
	struct JsValue r2, *v;

	TRACE(na->location, context, JS_TRACE_STATEMENT);
	EVAL(n->expr, context, &r2);
	v = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	JsGetValue(&r2, v);
	JS_SET_COMPLETION(res, JS_COMPLETION_RETURN, v);
}

/* 12.9 */
static void
ReturnStatement_undef_eval(na, context, res)
	struct JsAstNode *na; /* (struct ReturnStatement_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	static struct JsValue undef = { JS_UNDEFINED };

	TRACE(na->location, context, JS_TRACE_STATEMENT);
	JS_SET_COMPLETION(res, JS_COMPLETION_RETURN, &undef);
}

static void
SwitchStatement_caseblock(n, context, input, res)
	struct JsAstSwitchStatementNode *n;
	struct JsContext *context;
	struct JsValue *input, *res;
{
	struct JsAstCaseList *c;
	struct JsValue cc1, cc2, cc3;

	/*
	 * Note, this should be functionally equivalent
	 * to the standard. We search through the in-order
	 * case statements to find an expression that is
	 * strictly equal to 'input', and then run all
	 * the statements from there till we break or reach
	 * the end. If no expression matches, we start at the
	 * default case, if one exists.
	 */
	for (c = n->cases; c; c = c->next){
	    if (!c->expr) 
			continue;
	    EVAL(c->expr, context, &cc1);
		JsGetValue(&cc1, &cc2);
	    JsEqualityExpressionSeqCompare(input, &cc2, &cc3);
	    if(cc3.u.boolean == TRUE)
			break;
	}
	if (!c)
	    c = n->defcase;	/* can be NULL, meaning no default */
	JS_SET_COMPLETION(res, JS_COMPLETION_NORMAL, NULL);
	for (; c; c = c->next) {
	    if (c->body)
			EVAL(c->body, context, res);
	    if (res->u.completion.type != JS_COMPLETION_NORMAL)
			break;
	}
}


/* 12.11 */
static void
SwitchStatement_eval(na, context, res)
	struct JsAstNode *na; /* (struct SwitchStatement_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstSwitchStatementNode *n = CAST_NODE(na, JsAstSwitchStatementNode);
	struct JsValue *v, r1, r2;

	TRACE(na->location, context, JS_TRACE_STATEMENT);
	EVAL(n->cond, context, &r1);
	JsGetValue( &r1, &r2);
	SwitchStatement_caseblock(n, context, &r2, res);
	if (res->u.completion.type == JS_COMPLETION_BREAK)
	{
		v = res->u.completion.value;
		JS_SET_COMPLETION(res, JS_COMPLETION_NORMAL, v);
	}
}


/* 12.13 */
static void
ThrowStatement_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstUnaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstUnaryNode *n = CAST_NODE(na, JsAstUnaryNode);
	struct JsValue r1, *r2;
	
	r2 = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	
	TRACE(na->location, context, JS_TRACE_STATEMENT);
	EVAL(n->a, context, &r1);
	JsGetValue( &r1, r2);
	TRACE(na->location, context, JS_TRACE_THROW);
	
	JsThrowException(r2);
	/* NOTREACHED */
}

/*
 * Helper function to evaluate the catch clause in a new scope.
 * Return true if an exception was caught while executing the
 * catch clause.
 */
static struct JsValue*
TryStatement_help(n, context, C, res)
	struct JsAstTryStatementNode *n;
	struct JsContext *context;
	struct JsValue *C, *res;
{
	struct JsObject *r2;
	struct JsContext* sc;
	//配置新Scope
	r2 = JsCreateStandardObject(NULL);
	(*r2->Put)(r2,n->ident, C, JS_OBJECT_ATTR_DONTDELETE);
	JsListPush(context->scope,r2);
	//保存上下文资源
	sc = JsCopyContext(context);
		
	JS_TRY(0){
	    EVAL(n->bcatch, context, res);
	}
	//Finally
	JsListRemove(context->scope,JS_LIST_END);
	struct JsValue* e = NULL;
	JS_CATCH(e){
		//还原
		*context = *sc;
	}
	return e;
}


/* 12.14 try catch */
static void
TryStatement_catch_eval(na, context, res)
	struct JsAstNode *na; /* (struct TryStatement_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstTryStatementNode *n = CAST_NODE(na, JsAstTryStatementNode);
	struct JsContext* sc;
	TRACE(na->location, context, JS_TRACE_STATEMENT);
	//保存上下文资源

	sc = JsCopyContext(context);
	JS_TRY(0){
		EVAL(n->block, context, res);
	}
	struct JsValue* e = NULL;
	JS_CATCH(e){
		//还原上下文
		*context = *sc;
		//执行Catch内容
		e = TryStatement_help(n, context,e,res);
		if (e){
			//如果Catch抛出错误, 则继续抛出该错误
		    TRACE(na->location, context, JS_TRACE_THROW);
			JsReThrowException(e);
		}
	}
}


/* 12.14 try finally */
static void
TryStatement_finally_eval(na, context, res)
	struct JsAstNode *na; /* (struct TryStatement_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstTryStatementNode *n = CAST_NODE(na, JsAstTryStatementNode);
	struct JsValue r2;
	struct JsContext* sc;
	TRACE(na->location, context, JS_TRACE_STATEMENT);
	struct JsValue* e = NULL;
	
	sc = JsCopyContext(context);
	JS_TRY(0){
	    EVAL(n->block, context, res);
	}
	JS_CATCH(e){
		*context = *sc;
	}
	EVAL(n->bfinally, context, &r2);
	if (r2.type == JS_COMPLETION 
		&& r2.u.completion.type != JS_COMPLETION_NORMAL){
	    *res = r2; 		/* break, return etc */
	}else if ( e != NULL) {
		//如果发现try中存在throw, 则继续抛出
	    TRACE(na->location, context, JS_TRACE_THROW);
	    JsReThrowException(e);
	}
}

/* 12.14 try catch finally */
static void
TryStatement_catchfinally_eval(na, context, res)
	struct JsAstNode *na; /* (struct TryStatement_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstTryStatementNode *n = CAST_NODE(na, JsAstTryStatementNode);
	struct JsValue r6;
	struct JsValue *e = NULL;
	TRACE(na->location, context, JS_TRACE_STATEMENT);
	
	struct JsContext* sc ;
	sc = JsCopyContext(context);
	JS_TRY(0){
		EVAL(n->block, context, res);
	}JS_CATCH(e){
		*context = *sc; 
		//try 的 except 一定会被catch消耗,
		//如果catch中没有except, 则e = NULL
		e = TryStatement_help(n, context,e,res);
	}
	
	sc = JsCopyContext(context);
	JS_TRY(1){
		EVAL(n->bfinally, context, &r6);
	}
	if(JsCheckException()){
		//还原上下文
		*context = *sc;
		e = JsCatchException();
	}else if(r6.type == JS_COMPLETION 
		&& r6.u.completion.type != JS_COMPLETION_NORMAL){
		*res = r6;
	}
	if(e != NULL){
		TRACE(na->location, context, JS_TRACE_THROW);
		JsReThrowException(e);
	}
}



/* 11.14 */
static void
Expression_comma_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2, r3;

	EVAL(n->a, context, &r1);
	JsGetValue(&r1, &r2);
	EVAL(n->b, context, &r3);
	JsGetValue(&r3, res);
}


/* 11.13.2 |= */
static void
AssignmentExpression_oreq_eval(na, context, res)
	struct JsAstNode *na; /* (struct AssignmentExpression_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstAssignmentExpressionNode *n = 
		CAST_NODE(na, JsAstAssignmentExpressionNode);
	struct JsValue r1, r2, r3, r4,v0;

	EVAL(n->lhs, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->expr, context, &r3);
	JsGetValue( &r3, &r4);
	BitwiseORExpression_common(&r2, &r4, context, res);
	JsPutValue(&r1, res,&v0);
}


/* 11.13.2 ^= */
static void
AssignmentExpression_xoreq_eval(na, context, res)
	struct JsAstNode *na; /* (struct AssignmentExpression_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstAssignmentExpressionNode *n = 
		CAST_NODE(na, JsAstAssignmentExpressionNode);
	struct JsValue r1, r2, r3, r4,v0;

	EVAL(n->lhs, context, &r1);
	JsGetValue(&r1, &r2);
	EVAL(n->expr, context, &r3);
	JsGetValue(&r3, &r4);
	BitwiseXORExpression_common(&r2, &r4, context, res);
	JsPutValue(&r1, res,&v0);
}

/* 11.13.2 &= */
static void
AssignmentExpression_andeq_eval(na, context, res)
	struct JsAstNode *na; /* (struct AssignmentExpression_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstAssignmentExpressionNode *n = 
		CAST_NODE(na, JsAstAssignmentExpressionNode);
	struct JsValue r1, r2, r3, r4,v0;

	EVAL(n->lhs, context, &r1);
	JsGetValue(&r1, &r2);
	EVAL(n->expr, context, &r3);
	JsGetValue(&r3, &r4);
	BitwiseANDExpression_common(&r2, &r4, context, res);
	JsPutValue( &r1, res,&v0);
}


/* 11.13.2 >>>= */
static void
AssignmentExpression_urshifteq_eval(na, context, res)
	struct JsAstNode *na; /* (struct AssignmentExpression_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstAssignmentExpressionNode *n = 
		CAST_NODE(na, JsAstAssignmentExpressionNode);
	struct JsValue r1, r2, r3, r4,v0;

	EVAL(n->lhs, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->expr, context, &r3);
	JsGetValue( &r3, &r4);
	ShiftExpression_urshift_common(&r2, &r4, context, res);
	JsPutValue( &r1, res,&v0);
}


/* 11.13.2 >>= */
static void
AssignmentExpression_rshifteq_eval(na, context, res)
	struct JsAstNode *na; /* (struct AssignmentExpression_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstAssignmentExpressionNode *n = 
		CAST_NODE(na, JsAstAssignmentExpressionNode);
	struct JsValue r1, r2, r3, r4, v0;

	EVAL(n->lhs, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->expr, context, &r3);
	JsGetValue( &r3, &r4);
	ShiftExpression_rshift_common(&r2, &r4, context, res);
	JsPutValue(&r1, res,&v0);
}

/* 11.13.2 <<= */
static void
AssignmentExpression_lshifteq_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstAssignmentExpressionNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstAssignmentExpressionNode *n = 
		CAST_NODE(na, JsAstAssignmentExpressionNode);
	struct JsValue r1, r2,v0;

	EVAL(n->lhs, context, &r1);
	JsGetValue( &r1, &r2);
	ShiftExpression_lshift_common(&r2, n->expr, context, res);
	JsPutValue( &r1, res,&v0);
}


/* 11.13.2 -= */
static void
AssignmentExpression_subeq_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstAssignmentExpressionNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstAssignmentExpressionNode *n = 
		CAST_NODE(na, JsAstAssignmentExpressionNode);
	struct JsValue r1, r2, r3, r4, v0;

	EVAL(n->lhs, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->expr, context, &r3);
	JsGetValue( &r3, &r4);
	AdditiveExpression_sub_common(&r2, &r4, context, res);
	JsPutValue( &r1, res,&v0);
}

/* 11.13.2 += */
static void
AssignmentExpression_addeq_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstAssignmentExpressionNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstAssignmentExpressionNode *n = 
		CAST_NODE(na, JsAstAssignmentExpressionNode);
	struct JsValue r1, r2, r3, r4, v0;

	EVAL(n->lhs, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->expr, context, &r3);
	JsGetValue( &r3, &r4);
	AdditiveExpression_add_common(&r2, &r4, context, res);
	JsPutValue( &r1, res, &v0);
}

/* 11.13.2 %= */
static void
AssignmentExpression_modeq_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstAssignmentExpressionNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstAssignmentExpressionNode *n = 
		CAST_NODE(na, JsAstAssignmentExpressionNode);
	struct JsValue r1, r2, r3, r4, v0;

	EVAL(n->lhs, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->expr, context, &r3);
	JsGetValue( &r3, &r4);
	MultiplicativeExpression_mod_common(&r2, &r4, context, res);
	JsPutValue( &r1, res, &v0);
}


/* 11.13.2 /= */
static void
AssignmentExpression_diveq_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstAssignmentExpressionNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstAssignmentExpressionNode *n = 
		CAST_NODE(na, JsAstAssignmentExpressionNode);
	struct JsValue r1, r2, r3, r4, v0;

	EVAL(n->lhs, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->expr, context, &r3);
	JsGetValue( &r3, &r4);
	MultiplicativeExpression_div_common(&r2, &r4, context, res);
	JsPutValue( &r1, res,&v0);
}


/* 11.13.2 *= */
static void
AssignmentExpression_muleq_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstAssignmentExpressionNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstAssignmentExpressionNode *n = 
		CAST_NODE(na, JsAstAssignmentExpressionNode);
	struct JsValue r1, r2, r3, r4, v0;

	EVAL(n->lhs, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->expr, context, &r3);
	JsGetValue( &r3, &r4);
	MultiplicativeExpression_mul_common(&r2, &r4, context, res);
	JsPutValue( &r1, res, &v0);
}

/* 11.13.1 */
static void
AssignmentExpression_simple_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstAssignmentExpressionNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstAssignmentExpressionNode *n = 
		CAST_NODE(na, JsAstAssignmentExpressionNode);
	struct JsValue r1, r2, v0;

	EVAL(n->lhs, context, &r1);
	EVAL(n->expr, context, &r2);
	JsGetValue( &r2, res);
	JsPutValue( &r1, res,&v0);
}


/* 11.12 */
static void
ConditionalExpression_eval(na, context, res)
	struct JsAstNode *na; /* (struct ConditionalExpression_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstConditionalExpressionNode *n = 
		CAST_NODE(na, JsAstConditionalExpressionNode);
	struct JsValue r1, r2, r3, t;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	JsToBoolean(&r2, &r3);
	if (r3.u.boolean == TRUE)
		EVAL(n->b, context, &t);
	else
		EVAL(n->c, context, &t);
	JsGetValue( &t, res);
}

/* 11.11 */
static void
LogicalORExpression_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r3, r5;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, res);
	JsToBoolean(res, &r3);
	if (r3.u.boolean == TRUE)
		return;
	EVAL(n->b, context, &r5);
	JsGetValue( &r5, res);
}

/* 11.11 */
static void
LogicalANDExpression_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r3, r5;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, res);
	JsToBoolean(res, &r3);
	if (r3.u.boolean == FALSE)
		return;
	EVAL(n->b, context, &r5);
	JsGetValue( &r5, res);
}

/* 11.10 */
static void
BitwiseORExpression_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2, r3, r4;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->b, context, &r3);
	JsGetValue( &r3, &r4);
	BitwiseORExpression_common(&r2, &r4, context, res);
}

static void
BitwiseORExpression_common(r2, r4, context, res)
	struct JsValue *r2, *r4, *res;
	struct JsContext *context;
{
	int r5, r6;

	r5 = JsToInt32(r2);
	r6 = JsToInt32(r4);
	res->type = JS_NUMBER;
	res->u.number =  (r5 | r6);
}

/* 11.10 */
static void
BitwiseXORExpression_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2, r3, r4;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->b, context, &r3);
	JsGetValue( &r3, &r4);
	BitwiseXORExpression_common(&r2, &r4, context, res);
}

static void
BitwiseXORExpression_common(r2, r4, context, res)
	struct JsValue *r2, *r4, *res;
	struct JsContext *context;
{
	int r5, r6;

	r5 = JsToInt32( r2);
	r6 = JsToInt32( r4);
	res->type = JS_NUMBER;
	res->u.number = (r5 ^ r6);
}

/* 11.10 */
static void
BitwiseANDExpression_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2, r3, r4;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->b, context, &r3);
	JsGetValue( &r3, &r4);
	BitwiseANDExpression_common(&r2, &r4, context, res);
}


static void
BitwiseANDExpression_common(r2, r4, context, res)
	struct JsValue *r2, *r4, *res;
	struct JsContext *context;
{
	int r5, r6;

	r5 = JsToInt32(r2);
	r6 = JsToInt32(r4);
	res->type = JS_NUMBER;
	res->u.number = ( r5 & r6);
}


/* 11.9.5 */
static void
EqualityExpression_sne_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2, r3, r4, r5;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->b, context, &r3);
	JsGetValue( &r3, &r4);
	JsEqualityExpressionSeqCompare( &r4, &r2, &r5);
	res->type = JS_BOOLEAN;
	if(r5.u.boolean == TRUE)
		res->u.boolean = FALSE;
	else
		res->u.boolean = TRUE;
}

/* 11.9.4 */
static void
EqualityExpression_seq_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2, r3, r4;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->b, context, &r3);
	JsGetValue( &r3, &r4);
	JsEqualityExpressionSeqCompare( &r4, &r2, res);
}

/* 11.9.2 */
static void
EqualityExpression_ne_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2, r3, r4, t;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->b, context, &r3);
	JsGetValue( &r3, &r4);
	JsEqualityExpressionEqCompare( &r4, &r2, &t);
	res->type = JS_BOOLEAN;
	if(t.u.boolean  == TRUE){
		res->u.boolean  = FALSE;
	}else{
		res->u.boolean = TRUE;
	}

}

/* 11.9.1 */
static void
EqualityExpression_eq_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2, r3, r4;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->b, context, &r3);
	JsGetValue( &r3, &r4);
	JsEqualityExpressionEqCompare( &r4, &r2, res);
}


/* 11.8.7 */
static void
RelationalExpression_in_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2, r3, r4, r6;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->b, context, &r3);
	JsGetValue( &r3, &r4);
	if (r4.type != JS_OBJECT){
		JsThrowString("in not object");
	}
	JsToString(&r2, &r6);
	(*r4.u.object->HasProperty)(r4.u.object,r6.u.string,res);
}


/* 11.8.6 */
static void
RelationalExpression_instanceof_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2, r3, r4;


	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->b, context, &r3);
	JsGetValue( &r3, &r4);
	if (r4.type != JS_OBJECT)
		JsThrowString("instanceof not object");
	(*r4.u.object->HasInstance)(r4.u.object,&r2,res);
}

/* 11.8.4 */
static void
RelationalExpression_ge_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2, r3, r4, r5;

	
	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->b, context, &r3);
	JsGetValue( &r3, &r4);
	JsRelationalExpressionCompare(&r2, &r4, &r5);
	if (r5.type == JS_UNDEFINED){
		res->type = JS_BOOLEAN;
		res->u.boolean = FALSE;
	}
	else{
		res->type = JS_BOOLEAN;
		if(r5.u.boolean == TRUE){
			res->u.boolean = FALSE;
		}else{
			res->u.boolean = TRUE;
		}
	}
}


/* 11.8.3 */
static void
RelationalExpression_le_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2, r3, r4, r5;

	
	
	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->b, context, &r3);
	JsGetValue( &r3, &r4);
	JsRelationalExpressionCompare( &r4, &r2, &r5);
	if (r5.type == JS_UNDEFINED){
		res->type = JS_BOOLEAN;
		res->u.boolean = FALSE;
	}
	else{
		res->type = JS_BOOLEAN;
		if(r5.u.boolean == TRUE){
			res->u.boolean = FALSE;
		}else{
			res->u.boolean = TRUE;
		}
	}
}


/* 11.8.2 */
static void
RelationalExpression_gt_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2, r3, r4;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->b, context, &r3);
	JsGetValue( &r3, &r4);
	JsRelationalExpressionCompare( &r4, &r2, res);
	if (res->type == JS_UNDEFINED){
		res->type = JS_BOOLEAN;
		res->u.boolean = FALSE;
	}
}


/* 11.8.1 */
static void
RelationalExpression_lt_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2, r3, r4;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->b, context, &r3);
	JsGetValue( &r3, &r4);
	JsRelationalExpressionCompare(&r2, &r4, res);
	if (res->type == JS_UNDEFINED){
		res->type = JS_BOOLEAN;
		res->u.boolean = FALSE;
	}
}


/* 11.7.3 */
static void
ShiftExpression_urshift_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2, r3, r4;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->b, context, &r3);
	JsGetValue( &r3, &r4);
	ShiftExpression_urshift_common(&r2, &r4, context, res);
}


static void
ShiftExpression_urshift_common(r2, r4, context, res)
	struct JsValue *r2, *r4, *res;
	struct JsContext *context;
{
	unsigned int r5, r6;

	r5 = JsToUint32(r2);
	r6 = JsToUint32(r4);
	res->type = JS_NUMBER;
	res->u.number =( r5 >> (r6 & 0x1f));
}


/* 11.7.2 */
static void
ShiftExpression_rshift_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2, r3, r4;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->b, context, &r3);
	JsGetValue( &r3, &r4);
	ShiftExpression_rshift_common(&r2, &r4, context, res);
}

static void
ShiftExpression_rshift_common(r2, r4, context, res)
	struct JsValue *r2, *r4, *res;
	struct JsContext *context;
{
	int r5;
	unsigned int r6;

	r5 = JsToInt32(r2);
	r6 = JsToUint32(r4);
	
	res->type = JS_NUMBER;
	res->u.number =( r5 >> (r6 & 0x1f));
}


/* 11.7.1 */
static void
ShiftExpression_lshift_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	ShiftExpression_lshift_common(&r2, n->b, context, res);
}

static void
ShiftExpression_lshift_common(r2, bn, context, res)
	struct JsValue *r2, *res;
	struct JsAstNode *bn;
	struct JsContext *context;
{
	struct JsValue r3, r4;
	int r5;
	unsigned int r6;

	EVAL(bn, context, &r3);
	JsGetValue( &r3, &r4);
	r5 = JsToInt32(r2);
	r6 = JsToUint32(&r4);
	
	res->type = JS_NUMBER;
	res->u.number =( r5 << (r6 & 0x1f));

}


/* 11.6.2 */
static void
AdditiveExpression_sub_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2, r3, r4;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->b, context, &r3);
	JsGetValue( &r3, &r4);
	AdditiveExpression_sub_common(&r2, &r4, context, res);
}

static void
AdditiveExpression_sub_common(r2, r4, context, res)
	struct JsValue *r2, *r4, *res;
	struct JsContext *context;
{
	struct JsValue r5, r6;

	JsToNumber(r2, &r5);
	JsToNumber(r4, &r6);
	res->type = JS_NUMBER;
	res->u.number = r5.u.number - r6.u.number;
}


/* 11.6.1 */
static void
AdditiveExpression_add_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2, r3, r4;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->b, context, &r3);
	JsGetValue( &r3, &r4);
	AdditiveExpression_add_common(&r2, &r4, context, res);
}


static void
AdditiveExpression_add_common(r2, r4, context, res)
	struct JsValue *r2, *r4, *res;
	struct JsContext *context;
{
	struct JsValue r5, r6,
			 r8, r9, r12, r13;
	char *s;
	//默认hit JS_OBJECT_HIT_TYPE_STRING
	JsToPrimitive(r2, JS_OBJECT_HIT_TYPE_STRING, &r5);
	JsToPrimitive(r4, JS_OBJECT_HIT_TYPE_STRING, &r6);
	if (!(r5.type == JS_STRING || r6.type== JS_STRING)){
		JsToNumber(&r5, &r8);
		JsToNumber(&r6, &r9);
		res->type = JS_NUMBER;
		res->u.number =  r8.u.number + r9.u.number;
	}else{
		JsToString(&r5, &r12);
		JsToString(&r6, &r13);
		s = (char*)JsMalloc(strlen(r12.u.string) 
			+ strlen(r13.u.string) + 4);
		strcpy(s,r12.u.string);
		strcat(s,r13.u.string);
		res->type = JS_STRING;
		res->u.string = s;
	}
}


/* 11.5.3 */
static void
MultiplicativeExpression_mod_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2, r3, r4;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->b, context, &r3);
	JsGetValue( &r3, &r4);
	MultiplicativeExpression_mod_common(&r2, &r4, context, res);
}


static void
MultiplicativeExpression_mod_common(r2, r4, context, res)
	struct JsValue *r2, *r4, *res;
	struct JsContext *context;
{
	struct JsValue r5, r6;

	JsToNumber(r2, &r5);
	JsToNumber(r4, &r6);
	res->type = JS_NUMBER;
	res->u.number =  fmod(r5.u.number, r6.u.number);
}

/* 11.5.2 */
static void
MultiplicativeExpression_div_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2, r3, r4;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->b, context, &r3);
	JsGetValue( &r3, &r4);
    MultiplicativeExpression_div_common(&r2, &r4, context, res);
}


static void
MultiplicativeExpression_div_common(r2, r4, context, res)
	struct JsValue *r2, *r4, *res;
	struct JsContext *context;
{
	struct JsValue r5, r6;

	JsToNumber( r2, &r5);
	JsToNumber( r4, &r6);
	res->type = JS_NUMBER;
	res->u.number = r5.u.number / r6.u.number;
}

/* 11.5.1 */
static void
MultiplicativeExpression_mul_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstBinaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstBinaryNode *n = CAST_NODE(na, JsAstBinaryNode);
	struct JsValue r1, r2, r3, r4;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->b, context, &r3);
	JsGetValue( &r3, &r4);
	MultiplicativeExpression_mul_common(&r2, &r4, context, res);
}

static void
MultiplicativeExpression_mul_common(r2, r4, context, res)
	struct JsValue *r2, *r4, *res;
	struct JsContext *context;
{
	struct JsValue r5, r6;

	JsToNumber( r2, &r5);
	JsToNumber( r4, &r6);
	res->type = JS_NUMBER;
	res->u.number = r5.u.number * r6.u.number;
}

/* 11.4.9 */
static void
UnaryExpression_not_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstUnaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstUnaryNode *n = CAST_NODE(na, JsAstUnaryNode);
	struct JsValue r1, r2, r3;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	JsToBoolean( &r2, &r3);
	res->type = JS_BOOLEAN;
	if(r3.u.boolean == TRUE){
		res->u.boolean  = FALSE;
	}else{
		res->u.boolean  = TRUE;
	}
}

/* 11.4.8 */
static void
UnaryExpression_inv_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstUnaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstUnaryNode *n = CAST_NODE(na, JsAstUnaryNode);
	struct JsValue r1, r2;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	UnaryExpression_inv_eval_common(context, &r2, res);
}

static void
UnaryExpression_inv_eval_common(context, r2, res)
	struct JsContext *context;
	struct JsValue *r2, *res;
{
	int r3;

	r3 = JsToInt32(r2);
	res->type = JS_NUMBER;
	res->u.number = ~r3;
}

/* 11.4.7 */
static void
UnaryExpression_minus_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstUnaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstUnaryNode *n = CAST_NODE(na, JsAstUnaryNode);
	struct JsValue r1, r2;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	JsToNumber( &r2, res);
	res->type =JS_NUMBER;
	res->u.number = -(res->u.number);
}

/* 11.4.6 */
static void
UnaryExpression_plus_eval(na, context, res)
	struct JsAstNode *na; /* (struct Unary_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstUnaryNode *n = CAST_NODE(na, JsAstUnaryNode);
	struct JsValue r1, r2;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	JsToNumber( &r2, res);
}

/* 11.4.5 */
static void
UnaryExpression_predec_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstUnaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstUnaryNode *n = CAST_NODE(na, JsAstUnaryNode);
	struct JsValue r1, r2,v0;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	JsToNumber( &r2, res);
	res->u.number--;
	JsPutValue( &r1, res,&v0);
}

/* 11.4.4 */
static void
UnaryExpression_preinc_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstUnaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstUnaryNode *n = CAST_NODE(na, JsAstUnaryNode);
	struct JsValue r1, r2, v0;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	JsToNumber( &r2, res);
	res->u.number++;
	JsPutValue( &r1, res, &v0);
}


/* 11.4.3 */
static void
UnaryExpression_typeof_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstUnaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstUnaryNode *n = CAST_NODE(na, JsAstUnaryNode);
	struct JsValue r1;

	EVAL(n->a, context, &r1);
	UnaryExpression_typeof_eval_common(context, &r1, res);
}


static void
UnaryExpression_typeof_eval_common(context, r1, res)
	struct JsContext *context;
	struct JsValue *r1, *res;
{
	struct JsValue r4;
	char *s;
	res->type = JS_STRING;
	if (r1->type == JS_REFERENCE 
		&& r1->u.reference.base == NULL) {
		res->u.string = "undefined";
		return;
	}
	JsGetValue( r1, &r4);
	switch (r4.type) {
	case JS_UNDEFINED:	s = "undefined"; break;
	case JS_NULL:		s = "null"; break;
	case JS_BOOLEAN:	s = "boolean"; break;
	case JS_NUMBER:		s = "number"; break;
	case JS_STRING:		s = "string"; break;
	// 不识别function, 可以通过Object.prototype.toString.apply(obj)
	//	来区分Function 和 Object
	case JS_OBJECT:		s = "object"; break;
	default:			s = "unknow";break;
	}
	res->u.string = s;
}


/* 11.4.2 */
static void
UnaryExpression_void_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstUnaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstUnaryNode *n = CAST_NODE(na, JsAstUnaryNode);
	struct JsValue r1, r2;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	res->type = JS_UNDEFINED;
}


/* 11.4.1 */
static void
UnaryExpression_delete_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstUnaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstUnaryNode *n = CAST_NODE(na, JsAstUnaryNode);
	struct JsValue r1;

	EVAL(n->a, context, &r1);
	UnaryExpression_delete_eval_common(context, &r1, res);
}

static void
UnaryExpression_delete_eval_common(context, r1, res)
	struct JsContext *context;
	struct JsValue *r1, *res;
{
	res->type = JS_BOOLEAN;
	if (r1->type != JS_REFERENCE) {
		res->u.boolean = FALSE;
		return;
	}
	/*
	 * spec bug: if the base is null, it isn't clear what is meant 
	 * to happen. We return true as if the fictitous property 
	 * owner existed.
	 */
	int flag = TRUE;
	if (r1->u.reference.base){
		struct JsValue v0;
		(*r1->u.reference.base->Delete)(r1->u.reference.base,r1->u.reference.name,&v0);
		flag = v0.u.boolean;
	}
	res->u.boolean  = flag;
}
/* 11.3.2 */
static void
PostfixExpression_dec_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstUnaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstUnaryNode *n = CAST_NODE(na, JsAstUnaryNode);
	struct JsValue r1, r2, r3, v0;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	JsToNumber( &r2, res);
	r3.type = JS_NUMBER;
	r3.u.number = res->u.number - 1;
	JsPutValue( &r1, &r3, &v0);
}


/* 11.3.1 */
static void
PostfixExpression_inc_eval(na, context, res)
	struct JsAstNode *na; /* (struct JsAstUnaryNode) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstUnaryNode *n = CAST_NODE(na, JsAstUnaryNode);
	struct JsValue r1, r2, r3, v0;

	EVAL(n->a, context, &r1);
	JsGetValue( &r1, &r2);
	JsToNumber( &r2, res);
	r3.type = JS_NUMBER;
	r3.u.number = res->u.number + 1;
	JsPutValue( &r1, &r3, &v0);
}


/* 11.2.3 */
static void
CallExpression_eval(na, context, res)
	struct JsAstNode *na; /* (struct CallExpression_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstCallExpressionNode *n = CAST_NODE(na, JsAstCallExpressionNode);
	struct JsValue r1, *args, **argv;
	int argc, i;

	EVAL(n->exp, context, &r1);
	argc = n->args->argc;
	if (argc > 0) {
		args = (struct JsValue*)JsMalloc( sizeof(struct JsValue) * argc);
		argv = (struct JsValue**)JsMalloc( sizeof(struct JsValue*) * argc);
		Arguments_eval((struct JsAstNode *)n->args, context, args);
		for (i = 0; i < argc; i++)
			argv[i] = &args[i];
	}else 
		argv = NULL;
	CallExpression_eval_common(context, na->location, &r1, argc, argv, res);
}

/* 11.2.3 */
static void
CallExpression_eval_common(context, loc, r1, argc, argv, res)
	struct JsContext *context;
	struct JsLocation *loc;
	struct JsValue *r1;
	int argc;
	struct JsValue **argv;
	struct JsValue *res;
{
	struct JsValue r3;
	struct JsObject *r6, *r7;

	JsGetValue( r1, &r3);
	if (r3.type == JS_UNDEFINED)	/* nonstandard */
		JsThrowString("no such function");
	if (r3.type != JS_OBJECT)
		JsThrowString("not a function");
	/*
	默认配置了DarkCall
	if (!SEE_OBJECT_HAS_CALL(r3.u.object))
		SEE_error_throw_string(interp, interp->TypeError,
			STR(not_callable));
	*/
	if(r3.u.object->Call == NULL)
		JsThrowString("not callable function");
	
	//配置this
	if (r1->type == JS_REFERENCE)
		r6 = r1->u.reference.base;
	else
		r6 = NULL;
	if (r6 != NULL && strcmp(r6->Class,"Activation") == 0)
		r7 = NULL;
	else
		r7 = r6;
	//如果引用的对象为Activation的时候, 使用Global作为this
	if(!r7)
		r7 = JsGetVm()->Global;
	//Stack位置处理
	JsListPush(context->stack,loc);
	TRACE(loc, context, JS_TRACE_CALL);
	(*r3.u.object->Call)(r3.u.object,r7,argc,argv,res);
	TRACE(loc, context, JS_TRACE_RETURN);
	JsListRemove(context->stack,JS_LIST_END);
}


/* 11.2.1 */
static void
MemberExpression_bracket_eval(na, context, res)
	struct JsAstNode *na; /* (struct MemberExpression_bracket_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstMemberExpressionBracketNode *n = 
		CAST_NODE(na, JsAstMemberExpressionBracketNode);
	struct JsValue r1, r2, r3, r4, r5, r6;

	EVAL(n->mexp, context, &r1);
	JsGetValue( &r1, &r2);
	EVAL(n->name, context, &r3);
	JsGetValue( &r3, &r4);
	JsToObject( &r2, &r5);
	JsToString( &r4, &r6);

	res->type = JS_REFERENCE;
	res->u.reference.base = r5.u.object;
	res->u.reference.name = r6.u.string;
}


/* 11.2.1 */
static void
MemberExpression_dot_eval(na, context, res)
	struct JsAstNode *na; /* (struct MemberExpression_dot_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstMemberExpressionDotNode *n = 
		CAST_NODE(na, JsAstMemberExpressionDotNode);
	struct JsValue r1, r2, r5;

	EVAL(n->mexp, context, &r1);
	JsGetValue( &r1, &r2);
	JsToObject(&r2, &r5);
	res->type = JS_REFERENCE;
	res->u.reference.base = r5.u.object;
	res->u.reference.name = n->name;

}


/* 11.2.2 */
static void
MemberExpression_new_eval(na, context, res)
	struct JsAstNode *na; /* (struct MemberExpression_new_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstMemberExpressionNewNode *n = 
		CAST_NODE(na, JsAstMemberExpressionNewNode);
	struct JsValue r1, r2, *args, **argv;

	int argc, i;

	EVAL(n->mexp, context, &r1);
	JsGetValue( &r1, &r2);
	
	if (n->args) {
		argc = n->args->argc;
		args = (struct JsValue*)JsMalloc( sizeof(struct JsValue) * argc);
		argv = (struct JsValue**)JsMalloc( sizeof(struct JsValue*) * argc);
		Arguments_eval((struct JsAstNode *)n->args, context, args);
		for (i = 0; i < argc; i++)
			argv[i] = &args[i];
	} else {
		argc = 0;
		argv = NULL;
	}
	if (r2.type != JS_OBJECT)
		JsThrowString("new not an object");
	if (r2.u.object->Construct == NULL)
		JsThrowString("not_a_constructor");
	JsListPush(context->stack,na->location);
	TRACE(na->location, context, JS_TRACE_CALL);
	//Call, 不使用this指针.
	(*r2.u.object->Construct)(r2.u.object,NULL,argc, argv, res);
	TRACE(na->location, context, JS_TRACE_RETURN);
	JsListRemove(context->stack,JS_LIST_END);
}


/* 11.2.4 */
static void
Arguments_eval(na, context, res)
	struct JsAstNode *na; /* (struct Arguments_node) */
	struct JsContext *context;
	struct JsValue *res;		/* Assumed pointer to array */
{
	struct JsAstArgumentsNode *n = CAST_NODE(na, JsAstArgumentsNode);
	struct JsAstArgumentsArg *arg;
	struct JsValue v;

	for (arg = n->first; arg; arg = arg->next) {
		EVAL(arg->expr, context, &v);
		JsGetValue( &v, res);
		res++;
	}
}



/* 11.1.5 */
static void
ObjectLiteral_eval(na, context, res)
	struct JsAstNode *na; /* (struct ObjectLiteral_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstObjectLiteralNode *n = CAST_NODE(na, JsAstObjectLiteralNode);
	struct JsValue valuev, v;
	struct JsObject *o;
	struct JsAstObjectLiteralPair *pair;

	o = JsCreateStandardObject(NULL);
	for (pair = n->first; pair; pair = pair->next) {
		EVAL(pair->value, context, &valuev);
		JsGetValue( &valuev, &v);
		(*o->Put)(o,pair->name,&v,JS_OBJECT_ATTR_DEFAULT);
	}
	res->type = JS_OBJECT;
	res->u.object = o;
}


/* 11.1.4 */
static void
ArrayLiteral_eval(na, context, res)
	struct JsAstNode *na; /* (struct ArrayLiteral_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstArrayLiteralNode *n = CAST_NODE(na, JsAstArrayLiteralNode);
	struct JsAstArrayLiteralElement *element;
	struct JsValue expv, elv, v0;
	char *ind;
	struct JsObject* array;

	TRACE(na->location, context, JS_TRACE_CALL);
	//查询Array对象
	JsFindValue(NULL,"Array",&v0);
	//确定Array正确
	JsAssert(v0.type == JS_OBJECT && v0.u.object != NULL);
	array = v0.u.object;
	//构建空的Array
	(*array->Construct)(array,NULL,0,NULL,res);
	TRACE(na->location, context, JS_TRACE_RETURN);
	//配置length
	v0.type = JS_NUMBER;
	v0.u.number =  n->length;
	(*res->u.object->Put)(res->u.object,"length",&v0,JS_OBJECT_ATTR_DEFAULT);
	//加入数组元素
	int i;
	for (i=0,element = n->first; element; element = element->next , i++) {
		EVAL(element->expr, context, &expv);
		JsGetValue( &expv, &elv);
		//创建index string
		int bit = 0;
		int number = i;
		while(number){
			number /= 10;
			bit++;
		}
		ind = (char*)JsMalloc(bit+4);
		sprintf(ind,"%d",i);
		//添加到Array中
		(*res->u.object->Put)(res->u.object,ind,&elv,JS_OBJECT_ATTR_DEFAULT);
	}
}


/* 11.1.2 */
static void
PrimaryExpression_ident_eval(na, context, res)
	struct JsAstNode *na; /* (struct PrimaryExpression_ident_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstPrimaryExpressionIdentNode *n = 
		CAST_NODE(na, JsAstPrimaryExpressionIdentNode);
	JsFindValueRef(context,n->string, res);
}


/* 11.1.1 */
static void
PrimaryExpression_this_eval(n, context, res)
	struct JsAstNode *n;
	struct JsContext *context;
	struct JsValue *res;
{
	JsAssert( context->thisObj != NULL);
	res->type = JS_OBJECT;
	res->u.object = context->thisObj;
}

/* 7.8.4 */
static void
StringLiteral_eval(na, context, res)
	struct JsAstNode *na; /* (struct StringLiteral_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstStringLiteralNode *n = CAST_NODE(na, JsAstStringLiteralNode);
	res->type = JS_STRING;
	res->u.string = n->string;
}

/* 7.8 */
static void
Literal_eval(na, context, res)
	struct JsAstNode *na; /* (struct Literal_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstLiteralNode *n = CAST_NODE(na, JsAstLiteralNode);
	*res = *n->value;
}

//Eval类型函数Call的函数
static void JsFunctionCall(struct JsEngine* e,void* data,struct JsValue* res){
	//function body 不可能为NULL
	JsAssert(data != NULL);
	struct JsAstNode* body = (struct JsAstNode*)data;
	
	EVAL(body, e->exec, res);
}
/* 13 */
static void
FunctionExpression_eval(na, context, res)
	struct JsAstNode *na; /* (struct Function_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstFunctionNode *n = CAST_NODE(na, JsAstFunctionNode);

	//创建一个对象
	struct JsObject* fun = JsCreateStandardSpecFunction(NULL,context->scope,n->argc, n->argv,
		&JsFunctionCall,n->body,n->name,FALSE);
	if(n->name != NULL){
		//添加到当前Scope中
		struct JsObject* top = (struct JsObject*)JsListGet(context->scope,JS_LIST_END);
		struct JsValue* vFun = (struct JsValue* )JsMalloc(sizeof(struct JsValue));
		vFun->type = JS_OBJECT;
		vFun->u.object = fun;
		//只读属性
		(*top->Put)(top,n->name,vFun,JS_OBJECT_ATTR_READONLY);
	}
	res->type =JS_OBJECT;
	res->u.object = fun;
}

/* 13 */
static void
FunctionBody_eval(na, context, res)
	struct JsAstNode *na; /* (struct Unary_node) */
	struct JsContext *context;
	struct JsValue *res;
{
	struct JsAstFunctionBodyNode *n = CAST_NODE(na, JsAstFunctionBodyNode);
	struct JsValue v;

	EVAL(n->u.a, context, &v);
	//SourceElememts 运行结果只有JS_COMPLETION类型
	JsAssert(v.type == JS_COMPLETION);
	//throw 直接抛出, 不会出现在这里
	JsAssert(v.u.completion.type == JS_COMPLETION_NORMAL 
		|| v.u.completion.type == JS_COMPLETION_RETURN);
	
	
	//处理Res
	if(v.u.completion.type == JS_COMPLETION_NORMAL ||
		v.u.completion.value == NULL)
		//return ; 或者 没有return
		res->type = JS_UNDEFINED;
	else
		*res = *v.u.completion.value;
}

