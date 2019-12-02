/*
 * Copy me if you can.
 * by 20h
 */

#ifndef IND_H
#define IND_H

#define nil NULL

#define FCACHE 86400
#define ECACHE 300

#define GOPHERPATH "/.gopherfs/cache/"

void *greallocz(void *p, int l, int z);
void *gmemdup(void *p, int l);

#endif

