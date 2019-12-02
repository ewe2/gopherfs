/*
 * Copy me if you can.
 * by 20h
 */

#ifndef DB_H
#define DB_H

typedef struct sdbe sdbe;
struct sdbe {
	sdbe *n;
	sdbe *p;
	char *k;
	char *v;
	int l;
};

typedef struct sdb sdb;
struct sdb {
	sdbe *b;
	sdbe *l;
	int n;
};

void freeelem(sdbe *e);
sdbe *mkelem(char *v, char *k, int l, sdbe *p, sdbe *n);
sdb *createdb(void);
void destroydb(sdb *d);
int addelem(sdb *d, char *k, char *v, int l);
sdbe *getelem(sdb *d, char *k, sdbe *e);
int delelem(sdb *d, char *k);
void printdb(sdb *d);

#endif

