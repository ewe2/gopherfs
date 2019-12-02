/*
 * Copy me if you can.
 * by 20h
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ind.h"

void *
greallocz(void *p, int l, int z)
{

	p = realloc(p, l);
	if(p == nil)
		exit(1);
	if(z > 0)
		memset(p, 0, l);

	return p;
}

void *
gmemdup(void *p, int l)
{
	char *ret;

	ret = greallocz(nil, l, 2);
	memmove(ret, p, l);

	return (void *)ret;
}

