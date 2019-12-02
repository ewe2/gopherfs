/*
 * Copy me if you can.
 * by 20h
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ind.h"
#include "sdb.h"

void
freeelem(sdbe *e)
{

	if(e != nil) {
		free(e->v);
		free(e->k);
		free(e);
	}

	return;
}

sdbe *
mkelem(char *k, char *v, int l, sdbe *p, sdbe *n)
{
	sdbe *ret;

	ret = greallocz(nil, sizeof(sdbe), 2);
	ret->v = gmemdup(v, l);
	ret->k = gmemdup(k, strlen(k) + 1);
	ret->n = n;
	ret->p = p;

	return ret;
}

sdb *
createdb(void)
{

	return greallocz(nil, sizeof(sdb), 2);
}

void
destroydb(sdb *d)
{

	for(; d->n > 0; d->l = d->l->p, d->n--)
		freeelem(d->l->n);
	freeelem(d->b);
	free(d);

	return;
}

int
addelem(sdb *d, char *k, char *v, int l)
{
	sdbe *ret;

	ret = mkelem(k, v, l, d->l,
			(d->l != nil) ? d->l->n : nil);

	if(d->l != nil)
		d->l->n = ret;
	if(d->b == nil)
		d->b = ret;

	d->l = ret;
	d->n++;

	return 0;
}

sdbe *
getelem(sdb *d, char *k, sdbe *e)
{
	sdbe *cur;

	if(e != nil)
		cur = e->n;
	else
		cur = d->b;

	for(; cur != nil; cur = cur->n)
		if(!strncmp(cur->k, k, strlen(k)))
			return cur;

	return nil;
}

int
delelem(sdb *d, char *k)
{
	sdbe *e;

	e = getelem(d, k, nil);
	if(e == nil)
		return 1;

	if(d->b == e)
		d->b = e->n;
	if(d->l == e)
		d->l = e->p;

	if(e->p != nil)
		e->p->n = e->n;
	if(e->n != nil)
		e->n->p = e->p;

	freeelem(e);

	d->n--;

	return 0;
}

void
printdb(sdb *d)
{
        sdbe *cur;
	int i;

	fprintf(stdout, "d->n == %d\n", d->n);
        for(cur = d->b, i = 0; i < d->n; cur = cur->n, i++)
		fprintf(stdout, "%p \"%s\" -%p-> \"%s\" %p\n", cur->p,
			cur->k, cur, cur->v, cur->n);

	return;
}

