///Object2.c
#include"Object.h"
#include"Object2.h"
#include<stdio.h>
#include<stdlib.h>
void ObjectPrint2(struct Object* self){
	struct Pb2* p = (struct Pb2*)self->pb[1];
	printf("Object2 : %d \n",p->size);
	ObjectPrint(self);
}
struct Object* CreateObject2(struct Object* o){
	if(o == NULL)
		//处于继承链第一个位置
		o = AllocObject(OBJECT2_FLOOR);
	//初始化父对象
	CreateObject(o);
	//初始化pb数据块
	o->pb[OBJECT2_FLOOR] = malloc(sizeof(struct Pb2));
	//初始化sb数据块
	o->sb[OBJECT2_FLOOR] = NULL;
	//重载
	o->cls->p = &ObjectPrint2;
	((struct Pb2*)o->pb[OBJECT2_FLOOR])->size =3;
	return o;
}