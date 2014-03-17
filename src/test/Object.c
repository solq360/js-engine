//Object.c
#include<stdlib.h>
#include<stdio.h>
#include"Object.h"

struct Sb{
	int num;
};
void ObjectPrint(struct Object* self){
	struct Pb* p;
	struct Sb* s;
	p = (struct Pb*)self->pb[OBJECT_FLOOR];
	s = (struct Sb*)self->sb[OBJECT_FLOOR];
	printf("root Object Pb: %d\n root Object Sb: %d\n", p->size,s->num);
}
//申请基本的空间
//构建一个基本的空间, floor为子类所在的层, 根为第0层
struct Object* AllocObject(int floor){
	if(floor <= 0){
		//error
	}
	floor ++;
	struct Object* o;
	o = (struct Object*)malloc(sizeof(struct Object));
	o->cls = (struct Class* )malloc(sizeof(struct Class));
	//数据槽
	o->pb = (void**) malloc(sizeof(void*) * floor );
	o->sb = (void**) malloc(sizeof(void*) * floor );
	return o;
}

//初始化对象
//如果o == NULL, 则Alloc一个空间
struct Object* CreateObject(struct Object* o){
	if(o == NULL)
		o = AllocObject(OBJECT_FLOOR);
	//初始化pb数据块
	o->pb[OBJECT_FLOOR] = (struct Pb*)malloc(sizeof(struct Pb));
	//初始化sb数据块
	o->sb[OBJECT_FLOOR] = (struct Sb*)malloc(sizeof(struct Sb));
	o->cls->p = &ObjectPrint;
	((struct Pb*)o->pb[OBJECT_FLOOR])->size = 1;
	((struct Sb*)o->sb[OBJECT_FLOOR])->num = 2;
	return o;
}