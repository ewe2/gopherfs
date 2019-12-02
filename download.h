/*
 * Copy me if you can.
 * by 20h
 */

#ifndef DOWNLOAD_H
#define DOWNLOAD_H

void mkrecpathfile(char *path);
char *mkcachestr(char *descr, char *path);
int checkqueue(sdb *db, char *path);
int downloadfile(sdb *db, char *path);
int getbytes(sdb *db, char *path, char *buf, int size, int offset);
void *downloadthr(void *q);

#endif

