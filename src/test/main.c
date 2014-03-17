////main.c
#include"Object2.h"
#include<stdlib.h>
int main(){
	struct Object* o = CreateObject2(NULL);
	o->cls->p(o);
	return 0;
}
