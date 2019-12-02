/*
 * Copy me if you can.
 * by 20h
 */

#ifndef PROTO_H
#define PROTO_H

#include "sdb.h"
#include <stdarg.h>

void tprintf(int sock, char *fmt, ...);
char *readln(int fd);
int connecttcp(char *host, char *service);
int sendgopherreq(char *server, char *port, char *str);
void adddbentry(sdb *db, char type, char *dpath, char *path, char *name,
		char *server, char *port);
int parsegopher(int sock, sdb *db, char *dpath);
void writetofile(int sock, int fd);
int rnpggopher(char *srv, sdb *db, char *dpath, int fd);
int checkcache(sdb *db, char *path);

#endif

