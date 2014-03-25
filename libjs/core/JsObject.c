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
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<setjmp.h>
/*
	私有数据块
*/
struct JsStandardSelfBlock{
	/*临界数据*/
	JsList 					propertys;
	/*临界资源 迭代器记录区*/
	JsList					iterators;
	/*属性(propertys, iterators)锁*/
	JsLock 					lock;
	/*存储JsFunction的位置*/
	struct JsFunction*		function;
};
struct JsProperty{
	char* name;
	struct JsValue* value;
	int attr;
};
//当一个对象迭代完毕后, 重置属性, 并且挂载到下一个对象的iterators上
//如果该对象的属性在迭代过程中有被删除的, 则该迭代器失效useable = FALSE;
//如果再使用该迭代器, 则throw = error
struct JsIterator{
	int useable ;//是否可以使用
	int pos; //当前迭代到第几个属性
	int size;//创建迭代器时候, 对象的属性大小
	int slot;//在Object.iterators的第几个槽中
	struct JsObject* obj;//正在迭代的对象
};
struct JsFunction {
	enum JsFunctionType{
		JS_FUNCTION_NATIVE, //预示着body为一个回调函数
		JS_FUNCTION_EVAL //预设着body为AST
	}type;
	int argc;
	char **argv;
	void *data; //存储的数据(AST, USER)
	JsFunctionFn fn; //指向具体的函数(EVAL, Standard)
	char *name;	/* optional function name */
	int sync;//是否是Sync函数
	JsLock fSyncLock;//函数-Sync锁
};

/*
	o: 子对象是否已经构建完对象的基本内存结构
	isf: 是否是函数
	level: 	1 函数基本类型(只拷贝scope)
			2 使用hasInstance这个标准函数
			3 全部是标准函数
	scope : 函数特有属性
*/
static struct JsObject* JsCreateBaseObject(struct JsObject* o,int isf,int level,JsList scope);
//通知被删除属性的对象的迭代器
static void noticeDel(struct JsStandardSelfBlock* sb);
//创建一个Activation对象, Class = "Activation"
static struct JsObject* JsCreateActivationObject(struct JsObject* fun,int argc, struct JsValue** argv);
//创建一个Arguments对象
static struct JsValue* JsCreateArguments(struct JsObject* fun,int argc, struct JsValue** argv);

////////////////////////////////////////////////////////////////////////////////////
//构造一个JsObject的空间, floor 声明了pb 和 sb 数组的长度
// RootObject为 0 即JsStandardObject所在的层数
struct JsObject* JsAllocObject(int floor){
	if(floor < 0){
		JsAssert(FALSE);
	}
	floor ++;
	struct JsObject* o;
	o = (struct JsObject*)JsMalloc(sizeof(struct JsObject));
	//数据槽
	o->pb = (void**) JsMalloc(sizeof(void*) * floor );
	o->sb = (void**) JsMalloc(sizeof(void*) * floor );
	return o;
}
struct JsObject* JsCreateStandardObject(struct JsObject* o){
	return JsCreateBaseObject(o,FALSE,0,NULL);
}
struct JsObject* JsCreateStandardFunctionObject(struct JsObject* o,JsList scope,int inst){
	if(inst == TRUE)
		return JsCreateBaseObject(o,TRUE,1,scope);
	else
		return JsCreateBaseObject(o,TRUE,0,scope);
}
struct JsObject* JsCreateStandardSpecFunction(struct JsObject* o,JsList scope,int type,
		int argc, char** argv,void* data,JsFunctionFn fn,char* name,int sync){
	
	struct JsObject* obj = JsCreateBaseObject(o,TRUE,2,scope);
	
	struct JsFunction* function = (struct JsFunction* )JsMalloc(sizeof(struct JsFunction));
	//添加到sb
	
	((struct JsStandardSelfBlock*)obj->sb[JS_STANDARD_OBJECT_FLOOR])->function = function;
	//配置Function 内置属性
	if(type == 0)
		function->type = JS_FUNCTION_NATIVE;
	else
		function->type = JS_FUNCTION_EVAL;
		
	function->argc = argc; 
	function->argv = argv;
	function->data = data;
	function->fn = fn;
	function->name = name;
	function->sync = sync;
	if(sync){
		function->fSyncLock = JsCreateLock();
	}
	
	//配置prototype
	struct JsObject* prototypeObj = JsCreateStandardObject(NULL);
	struct JsValue* constructor = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	constructor->type = JS_OBJECT;
	constructor->u.object = obj;
	(*prototypeObj->Put)(prototypeObj,"constructor",constructor,JS_OBJECT_ATTR_DONTENUM);
	
	//配置Function Object显示属性
	struct JsValue* prototype = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	prototype->type = JS_OBJECT;
	prototype->u.object = prototypeObj;
	(*obj->Put)(obj,"prototype",prototype,JS_OBJECT_ATTR_DONTDELETE);
	
	struct JsValue* length = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	length->type = JS_NUMBER;
	length->u.number = argc;
	(*obj->Put)(obj,"length",length,JS_OBJECT_ATTR_STRICT);
	
	return obj;
}

static struct JsObject* JsCreateBaseObject(struct JsObject* o,int isf,int level,JsList scope){
	struct JsObject* obj;
	struct JsStandardSelfBlock* sb;
	struct JsValue v;
	if(o == NULL)
		obj = JsAllocObject(JS_STANDARD_OBJECT_FLOOR);
	else
		obj = o;
	//pb = (struct JsObjectPb*)obj->pb[JS_STANDARD_OBJECT_FLOOR] = JsMalloc(sizeof(struct JsPb));
	obj->Prototype = NULL;
	obj->Class = NULL;
	obj->SyncLock = JsCreateLock();
	obj->Scope = NULL;
	if(isf == TRUE){
		JsFindValue(NULL,"Function",&v);
		obj->Class = "Function";
		obj->Scope = JsCreateList();
		if(scope)
			JsListCopy(obj->Scope,scope);
		else
			JsListPush(obj->Scope,JsGetVm()->Global);

	}else{
		JsFindValue(NULL,"Object",&v);
		obj->Class = "Object";
	}
	//FindValue.res = JS_UNDEFINED  表示还没有初始化Object和Function
	if(v.type != JS_UNDEFINED){
		JsAssert(v.type == JS_OBJECT);
		(*v.u.object->Get)(v.u.object,"prototype",NULL,&v);
		JsAssert(v.type == JS_OBJECT);
		obj->Prototype =  v.u.object;
	}
	
	//选择使用的cls
	obj->Get = 				&JsStandardGet;
	obj->Put = 				&JsStandardPut;
	obj->CanPut = 			&JsStandardCanPut;
	obj->HasProperty = 		&JsStandardHasProperty;
	obj->Delete = 			&JsStandardDelete;
	obj->DefaultValue = 	&JsStandardDefaultValue;
	obj->HasOwnProperty = 	&JsStandardHasOwnProperty;
	obj->NextValue = 		&JsStandardNextValue;
	obj->Construct = 		&JsDarkCall;
	obj->Call = 			&JsDarkCall;
	obj->HasInstance = 		&JsDarkHasInstance;
	if(isf == TRUE){
		if(level == 1){
			obj->HasInstance = 		&JsStandardHasInstance;
		}
		if(level == 2){
			obj->Construct = 		&JsStandardConstruct;
			obj->Call = 			&JsStandardCall;
			obj->HasInstance = 		&JsStandardHasInstance;
		}
	}
	//构建PB
	obj->pb[JS_STANDARD_OBJECT_FLOOR] = NULL;
	//构建sb
	sb = (struct JsStandardSelfBlock* )JsMalloc(sizeof(struct JsStandardSelfBlock));	
	sb->propertys = JsCreateList();	
	sb->iterators = JsCreateList();
	sb->lock = JsCreateLock();
	sb->function = NULL;
	obj->sb[JS_STANDARD_OBJECT_FLOOR] = sb;
	
	return obj;
}

void JsDarkGet(struct JsObject *self, char *prop,int* flag, struct JsValue *res){
	JsThrowString("TypeError");
}
void JsDarkPut(struct JsObject *self,char *prop, struct JsValue *value, int flags){
	JsThrowString("TypeError");
}
void JsDarkBoolean(struct JsObject *self,char *prop,struct JsValue* res){
	JsThrowString("TypeError");
}
void JsDarkCall(struct JsObject *self, struct JsObject *thisobj,
				int argc, struct JsValue **argv, struct JsValue *res){
	JsThrowString("TypeError");
				
}
void JsDarkHasInstance(struct JsObject *self, struct JsValue *instance,struct JsValue* res){
	JsThrowString("TypeError");
}
void JsDarkDefaultValueFn(struct JsObject *self,int type, struct JsValue *res){
	JsThrowString("TypeError");
}
char* JsDarkNextValueFn(struct JsObject** next, JsIter* iter,int initialized){
	JsThrowString("TypeError");
	return NULL;
}


/* Standard Object对象方法, 可以给Standard类型的对象填充的内部属性, 不包含函数填充*/
void JsStandardGet(struct JsObject *self, char *prop,int* flag, struct JsValue *res){
	int i,size;
	struct JsProperty* p;
	struct JsStandardSelfBlock* sb;
	sb = (struct JsStandardSelfBlock*)self->sb[JS_STANDARD_OBJECT_FLOOR];
	//锁住
	JsLockup(sb->lock);
	size = JsListSize(sb->propertys);
	for(i=0;i<size;++i){
		p = (struct JsProperty*)JsListGet(sb->propertys,i);
		if(strcmp(p->name,prop) == 0){
			*res = *p->value;
			if(flag != NULL)
				*flag = p->attr;
			JsUnlock(sb->lock);
			return;
		}
	}
	//没有发现
	JsUnlock(sb->lock);

	if(!self->Prototype)
		res->type = JS_UNDEFINED;
	else
		(*self->Prototype->Get)(self->Prototype,prop,flag,res);

}
				
void JsStandardPut(struct JsObject *self, char *prop, struct JsValue *val0, int flags){
	int i,size;
	struct JsProperty* p;
	struct JsStandardSelfBlock* sb;
	struct JsValue v;
	sb = (struct JsStandardSelfBlock*)self->sb[JS_STANDARD_OBJECT_FLOOR];
	JsAssert(val0 != NULL);
	JsLockup(sb->lock);
	(*self->CanPut)(self,prop,&v);
	if(v.u.boolean  == FALSE){
		JsUnlock(sb->lock);
		return;
	}
	//拷贝指针指向的内存
	struct JsValue* val = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	*val = *val0;
	size = JsListSize(sb->propertys);
	for(i=0;i<size;++i){
		p = (struct JsProperty*)JsListGet(sb->propertys,i);
		if(strcmp(p->name,prop) == 0){
			//发现同名属性, 则覆盖value. attr 不修改
			p->value = val;
			JsUnlock(sb->lock);
			return;
		}
	}
	//没有发现该属性, 则新添加一个新的属性
	p = (struct JsProperty*)JsMalloc(sizeof(struct JsProperty));
	p->name = prop;
	p->value = val;
	p->attr = flags;
	JsListPush(sb->propertys,p);
	JsUnlock(sb->lock);
	return;
}
				
void  JsStandardCanPut(struct JsObject *self, char *prop,struct JsValue* res){
	int i,size;
	struct JsProperty* p;
	struct JsStandardSelfBlock* sb;
	res->type = JS_BOOLEAN;
	sb = (struct JsStandardSelfBlock*)self->sb[JS_STANDARD_OBJECT_FLOOR];
	JsLockup(sb->lock);
	size = JsListSize(sb->propertys);
	for(i=0;i<size;++i){
		p = (struct JsProperty*)JsListGet(sb->propertys,i);
		if(strcmp(p->name,prop) == 0){
			JsUnlock(sb->lock);
			if(p->attr & JS_OBJECT_ATTR_READONLY)
				res->u.boolean  = FALSE;
			else
				res->u.boolean  = TRUE;
			return;
		}
	}
	JsUnlock(sb->lock);
	if(!self->Prototype){
		res->u.boolean  = TRUE;
		return;
	}
	return ((*self->Prototype->CanPut)(self->Prototype,prop,res));
}
void JsStandardHasProperty(struct JsObject *self, char *prop,struct JsValue* res){
	int i,size;
	struct JsProperty* p;
	struct JsStandardSelfBlock* sb;
	res->type = JS_BOOLEAN;
	sb = (struct JsStandardSelfBlock*)self->sb[JS_STANDARD_OBJECT_FLOOR];
	JsLockup(sb->lock);
	size = JsListSize(sb->propertys);
	for(i=0;i<size;++i){
		p = (struct JsProperty*)JsListGet(sb->propertys,i);
		if(strcmp(p->name,prop) == 0){
			JsUnlock(sb->lock);
			res->u.boolean = TRUE;
			return;
		}
	}
	JsUnlock(sb->lock);
	if(!self->Prototype){
		res->u.boolean = FALSE;
		return;
	}
	return ((*self->Prototype->HasProperty)(self->Prototype,prop,res));
}
						
void JsStandardDelete(struct JsObject *self, char *prop,struct JsValue* res){
	int i,size;
	struct JsProperty* p;
	struct JsStandardSelfBlock* sb;
	res->type = JS_BOOLEAN;
	sb = (struct JsStandardSelfBlock*)self->sb[JS_STANDARD_OBJECT_FLOOR];
	JsLockup(sb->lock);
	size = JsListSize(sb->propertys);
	for(i=0;i<size;++i){
		p = (struct JsProperty*)JsListGet(sb->propertys,i);
		if(strcmp(p->name,prop) == 0){
			if(p->attr & JS_OBJECT_ATTR_DONTDELETE){
				JsUnlock(sb->lock);
				res->u.boolean = FALSE;
				return;
			}
			//删除该属性
			JsListRemove(sb->propertys,i);
			//对Iterator做处理
			noticeDel(sb);
			//解锁
			JsUnlock(sb->lock);
			res->u.boolean = TRUE;
			return;
		}
	}
	JsUnlock(sb->lock);	
	res->u.boolean = TRUE;
	return;
}
//self已经被上锁
static void noticeDel(struct JsStandardSelfBlock* sb){
	
	int size, i;
	struct JsIterator* iter ;
	size = JsListSize(sb->iterators);
	for(i=0;i< size; ++i){
		iter = (struct JsIterator*) JsListGet(sb->iterators,JS_LIST_FIRST);
		iter->useable = FALSE;
		JsListRemove(sb->iterators,JS_LIST_FIRST);
	}
}	
void JsStandardDefaultValue(struct JsObject *self, int hint, struct JsValue *res){
	struct JsValue res0;
//r1:
	if(hint != JS_OBJECT_HIT_TYPE_NUMBER)
		(*self->Get)(self,"toString",NULL,&res0);
	else
		(*self->Get)(self,"valueOf",NULL,&res0);
//r2:
	if(res0.type != JS_OBJECT || res0.u.object->Call == NULL)
		goto r5;
//r3:
	(*res0.u.object->Call)(res0.u.object,self,0,NULL,res);
//r4:
	if(res->type != JS_OBJECT && res->type != JS_REFERENCE 
			&& res->type != JS_COMPLETION){
		return ;
	}
r5: 	
	if(hint != JS_OBJECT_HIT_TYPE_NUMBER)
		(*self->Get)(self,"valueOf",NULL,&res0);
	else
		(*self->Get)(self,"toString",NULL,&res0);
//r6:
	if(res0.type != JS_OBJECT || res0.u.object->Call == NULL)
		goto r9;
//r7:
	(*res0.u.object->Call)(res0.u.object,self,0,NULL,res);
//r8
	//Primitive类型数据才被接受
	if(res->type != JS_OBJECT && res->type != JS_REFERENCE 
			&& res->type != JS_COMPLETION){
		return ;
	}
r9:
	JsThrowString("TypeError");
	return;
	
}
void JsStandardHasOwnProperty(struct JsObject *self, char *prop,struct JsValue* res){
	int i,size;
	struct JsProperty* p;
	struct JsStandardSelfBlock* sb;
	res->type =JS_BOOLEAN;
	sb = (struct JsStandardSelfBlock*)self->sb[JS_STANDARD_OBJECT_FLOOR];
	JsLockup(sb->lock);
	size = JsListSize(sb->propertys);
	for(i=0;i<size;++i){
		p = (struct JsProperty*)JsListGet(sb->propertys,i);
		if(strcmp(p->name,prop) == 0){
			JsUnlock(sb->lock);
			res->u.boolean = TRUE;
			return;
		}
	}
	JsUnlock(sb->lock);
	//没有发现
	res->u.boolean = FALSE;
	return;
				
}		

char* JsStandardNextValue(struct JsObject** next, JsIter* piter,int initialized){
	
	struct JsStandardSelfBlock* sb;
	struct JsIterator* iter;
	struct JsProperty* p;
	
	//初始化阶段
	if(initialized == FALSE){
		sb = (struct JsStandardSelfBlock*) (*next)->sb[JS_STANDARD_OBJECT_FLOOR];
		JsLockup(sb->lock);
		iter = (struct JsIterator*) JsMalloc(sizeof(struct JsIterator));
		iter->useable = TRUE;
		iter->pos = -1;
		iter->size = JsListSize(sb->propertys);
		iter->obj = (*next);
		JsListPush(sb->iterators,iter);
		//添加该迭代器在obj中的位置
		iter->slot = JsListSize(sb->iterators) - 1;
		//赋值
		*piter = iter;
		JsUnlock(sb->lock);
	}
	
	//迭代阶段
	JsAssert(piter != NULL && *piter != NULL);
	
	iter = *piter;
	
	if(iter->useable == FALSE){
		JsThrowString("IteratorError");
		return NULL;
	}
	//迭代已经完成
	if(iter->obj == NULL)
		return NULL;
		
	sb = (struct JsStandardSelfBlock*)iter->obj->sb[JS_STANDARD_OBJECT_FLOOR];
	JsLockup(sb->lock);
	for(;iter->pos < iter->size -1;){
		iter->pos ++;
		p = (struct JsProperty*) JsListGet(sb->propertys,iter->pos);
		if(p->attr & JS_OBJECT_ATTR_DONTENUM)
			continue;
		char* name = p->name;
		//解锁
		JsUnlock(sb->lock);
		//找到符合条件的name
		return name;
	}
	//该对象已经循环完毕
	//从对象上删除它
	JsListRemove(sb->iterators,iter->slot);
	//暂时标记为不可用
	iter->useable = FALSE;
	//修改对象
	iter->obj = iter->obj->Prototype;
	//修改next指针
	*next = iter->obj;
	
	iter->pos = -1;
	JsUnlock(sb->lock);

	if(iter->obj == NULL)
		return NULL;
	return (*iter->obj->NextValue)(&iter->obj,piter, FALSE);
}
/* 和standard 函数连用*/
void JsStandardCall(struct JsObject *self,struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res){
	struct JsFunction* f;
	struct JsObject* ac;
	struct JsStandardSelfBlock* sb;
	JsList newScope;
	JsList saveScope;
	struct JsObject* saveThisObj;
	int saveVarattr;
	//从线程中获得JsEngine
	struct JsEngine* e = JsGetTlsEngine();
	sb = (struct JsStandardSelfBlock*)self->sb[JS_STANDARD_OBJECT_FLOOR];
	f = (struct JsFunction*)sb->function;
	if(f->fn == NULL){
		res->type = JS_UNDEFINED;
		return;
	}
	if(f->sync)
		JsLockup(f->fSyncLock);
	//ActivationObject对象处理
	ac = JsCreateActivationObject(self,argc,argv);
	//创建一个新的Scope List
	newScope = JsCreateList();
	JsListCopy(newScope,self->Scope);
	JsListPush(newScope,ac);
	
	//处理上下文
	saveScope = e->exec->scope;
	e->exec->scope = newScope;
	
	saveThisObj = e->exec->thisObj;
	e->exec->thisObj = (thisobj != NULL ? thisobj : e->vm->Global);
	
	saveVarattr = e->exec->varattr;
	e->exec->varattr = JS_OBJECT_ATTR_DONTDELETE;
	
	//执行函数
	res->type =JS_UNDEFINED;
	struct JsValue* error = NULL;
	JS_TRY(0){
		(*f->fn)(e,f->data,res);
	}JS_CATCH(error){
		//释放资源锁
		if(f->sync)
			JsUnlock(f->fSyncLock);
		//继续抛出错误
		JsReThrowException(error);
	}
	//还原环境
	e->exec->scope = saveScope;
	e->exec->thisObj = saveThisObj;
	e->exec->varattr = saveVarattr;
	
	
	//结果处理
	if(res->type == JS_COMPLETION){

		if(res->u.completion.type == JS_COMPLETION_RETURN){
			*res = *res->u.completion.value;
		}else if(res->u.completion.type == JS_COMPLETION_THROW){
			//不处理
		}else{
			//JS_COMPLETION_NORMAL, JS_COMPLETION_BREAK, JS_COMPLETION_CONTINUE
			res->type = JS_UNDEFINED;
		}
	}else if(res->type == JS_REFERENCE){
		res->type = JS_UNDEFINED;
	}//res->type = JS_OBJECT, JS_NULL, JS_BOOLEAN ...

	if(f->sync)
		JsUnlock(f->fSyncLock);
	return;
}

static struct JsObject* JsCreateActivationObject(struct JsObject* fun, int argc, 
					struct JsValue** argv){
	int i;
	struct JsStandardSelfBlock* sb;
	sb = (struct JsStandardSelfBlock*)fun->sb[JS_STANDARD_OBJECT_FLOOR];
	struct JsFunction* f = (struct JsFunction*)sb->function;
	//构建Activation
	struct JsObject* activationObj = JsCreateStandardObject(NULL);
	//不使用默认"Object"
	activationObj->Class = "Activation";
	//配置arguments对象
	(*activationObj->Put)(activationObj,"arguments",JsCreateArguments(fun,argc,argv),JS_OBJECT_ATTR_DONTDELETE);
	//构建Function Params
	for(i=0;i<f->argc && i< argc;++i){
		(*activationObj->Put)(activationObj,f->argv[i],argv[i],JS_OBJECT_ATTR_DONTDELETE);
	}
	//处理未被赋值的Params
	for(;i<f->argc;++i){
		struct JsValue* undef = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
		(*activationObj->Put)(activationObj,f->argv[i],undef,JS_OBJECT_ATTR_DONTDELETE);
	}
	return activationObj;
}
static struct JsValue* JsCreateArguments(struct JsObject* fun,int argc, struct JsValue** argv){
	int i;
	struct JsObject* argumentsObj = JsCreateStandardObject(NULL);
	struct JsValue* length = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	length->type = JS_NUMBER;
	length->u.number = argc;
	//length
	(*argumentsObj->Put)(argumentsObj,"length",length,JS_OBJECT_ATTR_DONTENUM);
	struct JsValue* callee = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	callee->type = JS_OBJECT;
	callee->u.object = fun;
	(*argumentsObj->Put)(argumentsObj,"callee",callee,JS_OBJECT_ATTR_DONTENUM);
	
	//构建arguments[0]...[i]
	for(i =0 ; i < argc;++i){
		//计算整数的位数
		int number = i;
		int bit = 0;
		while(number){
			number /=10;
			bit++;
		}
		char* buf = (char*)JsMalloc(bit + 4);
		sprintf(buf,"%d",i);
		(*argumentsObj->Put)(argumentsObj,buf,argv[i],JS_OBJECT_ATTR_DONTENUM);
	}
	//构建JsValue
	struct JsValue* arguments = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	arguments->type = JS_OBJECT;
	arguments->u.object  = argumentsObj;
	return arguments;
}
	
void JsStandardConstruct(struct JsObject *self, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res){
			
	struct JsObject* this0 = JsCreateStandardObject(NULL);
	struct JsValue res0;
	//获得构造函数的prototype属性
	(*self->Get)(self,"prototype",NULL,&res0);
	if(res0.type == JS_OBJECT)
		this0->Prototype = res0.u.object;
	//Call
	(*self->Call)(self,this0,argc,argv,res);
	if(res->type != JS_OBJECT){
		res->type = JS_OBJECT;
		res->u.object = this0;
	}
}
void JsStandardHasInstance(struct JsObject *self, struct JsValue *instance,
			struct JsValue* res){
	struct JsValue res0;
	struct JsObject* prototype;
	if(instance->type != JS_OBJECT){
		res->type = JS_BOOLEAN;
		res->u.boolean = FALSE;
		return;
	}
	(*self->Get)(self,"prototype",NULL,&res0);
	if(res0.type != JS_OBJECT){
		JsThrowString("TypeError");
		return;
	}
	prototype = instance->u.object->Prototype;
	//循环查询
	while(prototype != NULL){
		if(prototype == res0.u.object){
			res->type = JS_BOOLEAN;
			res->u.boolean = TRUE;
			return;
		}
		//下一个对象的原形
		prototype = prototype->Prototype;
	}
	//没有查询到
	res->type = JS_BOOLEAN;
	res->u.boolean = FALSE;
}