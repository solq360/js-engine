///Object2.h
#ifndef _Object2_
#define _Object2_
#define OBJECT2_FLOOR 1
#include"Object.h"
//新子类的共享数据
struct Pb2{
	int size;
};
void ObjectPrint2(struct Object* self);
struct Object* CreateObject2(struct Object* o);
#endif