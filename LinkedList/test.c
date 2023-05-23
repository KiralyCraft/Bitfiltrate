/*
 * test.c
 *
 *  Created on: May 10, 2023
 *      Author: kiraly
 */

#include "dlinkedmap.h"

int main()
{
	printf("Consoel pleaz\n");
	dlinkedmap_t* _theMap = dlinkedmap_createMap();

	int a = 2;
	int b = 3;
	int c = 4;
	int d = 6;
	printf("%d\n",dlinkedmap_put(&a,&b,_theMap));
	printf("%d\n",*((int*)dlinkedmap_get(&a,_theMap)));
	printf("%d\n",dlinkedmap_put(&a,&c,_theMap));
	printf("%d\n",*((int*)dlinkedmap_get(&a,_theMap)));
	printf("%d\n",dlinkedmap_put(&d,&a,_theMap));
	printf("%d\n",*((int*)dlinkedmap_get(&a,_theMap)));

	printf("%d\n",dlinkedmap_remove(&a,_theMap));
	printf("%d\n",*((int*)dlinkedmap_get(&d,_theMap)));

	printf("%d\n",dlinkedmap_get(&a,_theMap));

}
