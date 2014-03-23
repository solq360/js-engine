#ifndef JsTokenH
#define JsTokenH



#define tEND		0		/* end of file */
#define tANDAND		257 // &&
#define tANDEQ		258 // &=
#define tBREAK		259 // break
#define tCASE		260 // case
#define tCATCH		261 // catch
#define tCONTINUE	262 // continue
#define tDEFAULT	263 // defualt
#define tDELETE		264 // delete
#define tDIV		'/' // /
#define tDIVEQ		266 // /=
#define tDO		    267 // do
#define tELSE		268 // else
#define tEQ		    269 // = 
#define tFINALLY	270 // finally
#define tFOR		271 // for
#define tFUNCTION	272 // function
#define tGE		    273 // >=
#define tIF		    274 // if
#define tIN		    275 // in
#define tINSTANCEOF	276 // instanceof
#define tLE		    277 // <=
#define tLSHIFT		278 // <<
#define tLSHIFTEQ	279 // <<=
#define tMINUSEQ	280 // -=
#define tMINUSMINUS	281 // --
#define tMODEQ		282 // %=
#define tNE		    283 // !=
#define tNEW		284 // new
#define tOREQ		285 // |=
#define tOROR		286 // ||
#define tPLUSEQ		287 // +=
#define tPLUSPLUS	288 // ++
#define tREGEX		289 // nouse
#define tRESERVED	290	/* any "reserved" keyword */
#define tRETURN		291 // return
#define tRSHIFT		292 // >>
#define tRSHIFTEQ	293 // >>=
#define tSEQ		294 // ===
#define tSNE		295 // !==
#define tSTAREQ		296 // *=
#define tSWITCH		297 // switch
#define tTHIS		298 // this
#define tTHROW		299 // throw
#define tTRY		300 // try
#define tTYPEOF		301 // typeof
#define tURSHIFT	302 // >>>
#define tURSHIFTEQ	303 // >>>=
#define tVAR		304 // var
#define tVOID		305 // void
#define tWHILE		306 // while
#define tWITH		307 // with
#define tXOREQ		308 // ^=
#define tNUMBER		309	/* numeric constant */
#define tSTRING		310	/* string constant */
#define tIDENT		311	/* non-keyword identifier */
#define tCOMMENT	312	/* internal: any kind of comment */
#define tLINETERMINATOR	313	/* internal: end of line */
#define tTRUE		314 // true
#define tNULL		315 // null
#define tFALSE		316 // false

//通过token 获得它的字符串
const char* JsTokenName(int token);
#define NO_USED_TOKEN -1
//支援lexer
typedef union YYSTYPE {
	char* string;
	double number;
} YYSTYPE;
#endif
