#include "btHashMap.h"

#include <stdio.h>

int main()
{
	btHashMap<btHashInt, btHashInt> btMap;
	
	int k = 1234, v = 5678;
	btMap.insert(btHashInt(k), btHashInt(v));
	
	btHashInt* pVal = btMap.find(btHashInt(k));
	if(pVal == NULL) {
		printf("key: %d not found in btMap\n", k);
	}
	else {
		printf("found key: %d, value: %d in btMap\n", k, v);
	}
	
	return 0;
}