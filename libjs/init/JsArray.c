#include"JsObject.h"
#include"JsContext.h"
#include"JsEngine.h"
#include"JsVm.h"
#include"JsValue.h"
#include"JsList.h"
#include"JsSys.h"
#include"JsDebug.h"
#include"JsAst.h"
#include"JsECMAScript.h"
#include"JsException.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

//创建一个Array对象, prototype 为NULL表示指向 Object_prototype
//如果argv == NULL 表示不需要添加具体的元素
static struct JsObject* JsCreateArray(struct JsObject* prototype, int length, struct JsValue** argv);
static void JsArrayFunctionInit(struct JsObject* array,struct JsObject* array_proto);	
static void JsArrayProtoInit(struct JsObject* array,struct JsObject* array_proto);

//----------------------------------------------------------------------------
//Const构造函数
static void JsArrayConstCall(struct JsObject *self, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res);
//Inst实例
static void JsArrayInstPut(struct JsObject *self,char *prop, struct JsValue *value, int flags);			

//Array.prototype.toString
static void JsArrayProtoToString(struct JsObject *self, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res);			

//Array.prototype.join
static void JsArrayProtoJoin(struct JsObject *self, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res);
//Array.prototype.pop
static void JsArrayProtoPop(struct JsObject *self, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res);	
//Array.prototype.join
static void JsArrayProtoPush(struct JsObject *self, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res);
			
//----------------------------------------------------------------------------
void JsArrayInit(struct JsVm* vm){

	struct JsObject* array = JsCreateStandardFunctionObject(NULL,NULL,TRUE);
	//Array.prototype 是一个缩减版的Array对象
	struct JsObject* array_proto = JsCreateArray(NULL,0,NULL);
	
	JsArrayFunctionInit(array,array_proto);
	JsArrayProtoInit(array,array_proto);
	//向Global添加Array
	struct JsValue* v = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	v->type = JS_OBJECT;
	v->u.object = array;
	(*vm->Global->Put)(vm->Global,"Array",v,JS_OBJECT_ATTR_STRICT);
}


//----------------------------------------------------------------------------
//创建一个Array对象, prototype 为NULL表示指向 Object_prototype
//如果argv == NULL 表示不需要添加具体的元素
static struct JsObject* JsCreateArray(struct JsObject* prototype, int length, struct JsValue** argv){
	//创建一个标准的对象
	struct JsObject* array = JsCreateStandardObject(NULL);
	if(prototype != NULL)
		array->Prototype = prototype;
	struct JsValue* vLength = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	vLength->type = JS_NUMBER;
	vLength->u.number = length;
	array->Class = "Array";
	(*array->Put)(array,"length",vLength,JS_OBJECT_ATTR_DONTENUM | JS_OBJECT_ATTR_DONTDELETE);
	//构建[item...]
	int i;
	for( i = 0 ; i < length && argv != NULL ;++i){
		//计算bit
		int number = i;
		int bit = 0;
		while(number){
			number /=10;
			bit++;
		}
		char* buf = (char*)JsMalloc(bit+4);
		sprintf(buf,"%d",i);
		(*array->Put)(array,buf,argv[i],JS_OBJECT_ATTR_DEFAULT);
	}
	//修改Put
	array->Put = &JsArrayInstPut;
	return array;
}

static void JsArrayFunctionInit(struct JsObject* array,struct JsObject* array_proto){
	array->Call = &JsArrayConstCall;
	array->Construct = &JsArrayConstCall;
	//Array.prototype
	struct JsValue* vPrototype = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	vPrototype->type = JS_OBJECT;
	vPrototype->u.object = array_proto;
	(*array->Put)(array,"prototype",vPrototype,JS_OBJECT_ATTR_STRICT);
}
static void JsArrayProtoInit(struct JsObject* array,struct JsObject* array_proto){
	
	struct JsValue* p;
	//Array.prototype.constructor
	p= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	p->type = JS_OBJECT;
	p->u.object = array;
	(*array_proto->Put)(array_proto,"constructor",p,JS_OBJECT_ATTR_DONTENUM);
	
	//Array.prototype.toString
	p= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	p->type = JS_OBJECT;
	p->u.object = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	p->u.object->Call = &JsArrayProtoToString;
	(*array_proto->Put)(array_proto,"toString",p,JS_OBJECT_ATTR_DONTENUM);
	
	//Array.prototype.join
	p= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	p->type = JS_OBJECT;
	p->u.object = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	p->u.object->Call = &JsArrayProtoJoin;
	(*array_proto->Put)(array_proto,"join",p,JS_OBJECT_ATTR_DONTENUM);
	
	//Array.prototype.pop
	p= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	p->type = JS_OBJECT;
	p->u.object = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	p->u.object->Call = &JsArrayProtoPop;
	(*array_proto->Put)(array_proto,"pop",p,JS_OBJECT_ATTR_DONTENUM);
	
	//Array.prototype.push
	p= (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	p->type = JS_OBJECT;
	p->u.object = JsCreateStandardFunctionObject(NULL,NULL,FALSE);
	p->u.object->Call = &JsArrayProtoPush;
	(*array_proto->Put)(array_proto,"push",p,JS_OBJECT_ATTR_DONTENUM);
	
}
//----------------------------------------------------------------

static void JsArrayConstCall(struct JsObject *self, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res){
	struct JsValue v;
	int length;
	
	length = argc;
	if(argc == 1){
		//Array(len)
		length = JsToUint32(argv[0]);
	}
	(*self->Get)(self,"prototype",NULL,&v);
	if(v.type != JS_OBJECT || strcmp(v.u.object->Class,"Array") != 0)
		JsAssert(FALSE);
	struct JsObject* array;
	//根据参数数量不同, 选择初始化模式
	if(argc == 1){
		array = JsCreateArray(v.u.object,length,NULL);
	}else{
		array = JsCreateArray(v.u.object,length,argv);
	}
	res->type = JS_OBJECT;
	res->u.object = array;
}

//Array.prototype.toString
static void JsArrayProtoToString(struct JsObject *self, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res){
	if(strcmp(thisobj->Class,"Array") != 0)
		JsThrowString("TypeError");
	JsArrayProtoJoin(NULL,thisobj,0,NULL,res);	
}
//Array.prototype.join
/*
	最后的格式为
	string,string...,string
*/
static void JsArrayProtoJoin(struct JsObject *self, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res){
	struct JsValue r;
	struct JsValue s;
	struct JsValue v;
	int length;
	char* sep = ",";
	
	//长度获取
	(*thisobj->Get)(thisobj,"length",NULL,&v);
	if(v.type == JS_UNDEFINED || v.u.number <= 0){
		res->type = JS_STRING;
		res->u.string = "";
		return;
	}
	length = v.u.number;
	
	if(argc >= 1 && argv[0]->type != JS_UNDEFINED){
		JsToString(argv[0],&v);
		sep = v.u.string;
	}
	//sep 的长度
	int sepLength = strlen(sep);
	
	// "0"
	(*thisobj->Get)(thisobj,"0",NULL,&v);
	if(v.type == JS_NULL || v.type == JS_UNDEFINED){
		s.type = JS_STRING;
		s.u.string ="";
	}else{
		JsToString(&v,&s);
	}
	r.type = JS_STRING;
	int sizeOfMem = strlen(s.u.string)+sepLength+4;
	r.u.string = (char*)JsMalloc(sizeOfMem);
	strcpy(r.u.string,s.u.string);
	int k = 1;
r10:
	if(length == k){
		*res = r;
		return;
	}	
	int number = k;
	int bit = 0 ;
	while(number){
		number /= 10;
		bit ++;
	}
	char* cur = (char*)JsMalloc(bit+4);
	sprintf(cur,"%d",k);
	(*thisobj->Get)(thisobj,cur,NULL,&v);
	if(v.type == JS_UNDEFINED|| v.type == JS_NULL){
		
	}else{
		JsToString(&v,&s);
		sizeOfMem = sizeOfMem + sepLength  + strlen(s.u.string);
		r.u.string = (char*)JsReAlloc(r.u.string,sizeOfMem);
		strcat(r.u.string,sep);
		strcat(r.u.string,s.u.string);
	}
	k++;
	goto r10;
			
}
static void JsArrayProtoPop(struct JsObject *self, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res){
	struct JsValue vLength;
	struct JsValue v;
	(*thisobj->Get)(thisobj,"length",NULL,&vLength);
	if(vLength.type == JS_NUMBER && vLength.u.number > 0){
		int number = vLength.u.number;
		int bit = 0;
		while(number){
			number /= 10;
			bit++;
		}
		char* buf = (char*)JsMalloc(bit+4);
		sprintf(buf,"%d",(int)(vLength.u.number - 1));
		(*thisobj->Get)(thisobj,buf,NULL,res);
		(*thisobj->Delete)(thisobj,buf,&v);
		struct JsValue* l = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
		l->type = JS_NUMBER;
		l->u.number = vLength.u.number - 1;
		(*thisobj->Put)(thisobj,"length",l,JS_OBJECT_ATTR_DONTENUM | JS_OBJECT_ATTR_DONTDELETE);
	}else
		res->type = JS_UNDEFINED;

}

static void JsArrayProtoPush(struct JsObject *self, struct JsObject *thisobj,
			int argc, struct JsValue **argv, struct JsValue *res){
	if(argc <= 0){
		res->type = JS_NUMBER;
		res->u.number = 0;
		return;
	}
	struct JsValue vLength;
	(*thisobj->Get)(thisobj,"length",NULL,&vLength);
	int begin = 0;
	if(vLength.type == JS_NUMBER){
		begin = vLength.u.number > 0 ? vLength.u.number : 0;
	}
	int i;
	for( i= begin ; i < begin+argc ;++i){
		int number = i;
		int bit = 0 ; 
		while(number){
			number /= 10;
			bit++;
		}
		char* buf = (char*)JsMalloc(bit + 4);
		sprintf(buf,"%d",i);
		(*thisobj->Put)(thisobj,buf,argv[i - begin],JS_OBJECT_ATTR_DEFAULT);
	}
	struct JsValue* l = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
	l->type = JS_NUMBER;
	l->u.number = begin + argc;
	(*thisobj->Put)(thisobj,"length",l,JS_OBJECT_ATTR_DONTENUM | JS_OBJECT_ATTR_DONTDELETE);
}


static void JsArrayInstPut(struct JsObject *self,char *prop, struct JsValue *value, int flags){
	struct JsValue v;
	struct JsValue v0;
	if(strcmp(prop,"length") == 0){
		if(value->type != JS_NUMBER || value->u.number < 0)
			JsThrowString("RangeError");
		(*self->Get)(self,"length",NULL,&v);
		if(v.type == JS_NUMBER){
			int i;
			for( i=value->u.number ; i< v.u.number;++i){
				//计算bit
				int number = i;
				int bit = 0;
				while(number){
					number /=10;
					bit++;
				}
				char* buf = (char*)JsMalloc(bit+4);
				sprintf(buf,"%d",i);
				//删除不在新范围内的数组元素
				(*self->Delete)(self,buf,&v0);
			}
		}
	}else{
		//判断是否是array Index
		char *endptr = NULL;
		long index = 0;
		index = strtol(prop,&endptr,10);
		if(*endptr == '\0'){
			//说明该prop被完全解析为数据=>prop为array index
			(*self->Get)(self,"length",NULL,&v);
			if(v.type != JS_NUMBER || v.u.number < 0)
				JsThrowString("RangeError");
			if(index >= v.u.number){
				//如果大于length, 则需要重新写入length
				struct JsValue* l = (struct JsValue*)JsMalloc(sizeof(struct JsValue));
				l->type = JS_NUMBER;
				l->u.number = index + 1;
				//修改length
				//不使用self->Put, 而直接调用父类的Put
				JsStandardPut(self,"length",l,JS_OBJECT_ATTR_DONTENUM | JS_OBJECT_ATTR_DONTDELETE);
			}
		}
	}
	//写入数据
	JsStandardPut(self,prop,value,flags);
}
