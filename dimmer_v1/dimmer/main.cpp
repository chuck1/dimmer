
#define __LINUX

#include <stdio.h>

typedef unsigned char uint8_t;

#include "List.h"

void func(unsigned char, unsigned char)
{
}

int main()
{

	Item * i0 = new Item(0, &func, 0, 0);
	Item * i1 = new Item(15, &func, 0, 0);
	Item * i2 = new Item(20, &func, 0, 0);

	Item * list = i0;

	list->insert(i1);
	list->insert(i2);


	list = list->update(1000, 0);

	list->print();

	
	i0->_M_count = 0;
	i1->_M_count = 0;
	i2->_M_count = 0;


	list = list->update(1000, 0);

	list->print();
}


