////Object.h
#ifndef _Object_
#define _Object_
/*******************
cls:
	类似于Java的接口, 子类想对某个函数进行重载 , 
	则只要在具体同名函数的位置替换掉原先的指针 .
	而子类想访问被覆盖掉的父类同名函数, 则直接访问
	父类的函数指针.
	而私有函数, 则都在.c 文件中
pb:
	公共数据结构, 数据结构在.h中
	通过指针来访问自己的和父对象的公用数据
	((struct Pb*)pb[0])->size ; 访问了跟对象的public block的size
sb: 
	私有的数据成员, 数据结构在.c中, 非本对象不可操作
	((struct Sb*)sb[0])->size
FLOOR:
	该对象处于继承链的第几个位置, Root 为 0
注: pb 和 sb的 数据槽 都由new Object的对象来申请空间
　　初始化的方向为先初始化父类
　　析构方向为先析构子类
********************/
struct Object;
//接口操作
typedef void (*PrintFn)(struct Object* self);
#define OBJECT_FLOOR 0
struct Class{
	PrintFn p;
};
//公布的操作
struct Pb{
	int size;
};
struct Object{
	struct Class* cls;
	void** pb;
	void** sb;
};
void ObjectPrint(struct Object* self);
//申请基本的空间
//构建一个基本的空间, floor为子类所在的层, 根为第0层
struct Object* AllocObject(int floor);
//初始化对象
//如果o == NULL, 则Alloc一个空间
struct Object* CreateObject(struct Object* o);
#endif