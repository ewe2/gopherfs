/*
 * Copy me if you can.
 * by 20h
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ind.h"
#include "sdb.h"
#include "proto.h"

void
mkrecpathfile(char *path)
{
	struct stat st;
	char *cpath, *c, *n;
	int fd;

	cpath = gmemdup(path, strlen(path) + 1);

	c = n = strchr(cpath + 1, '/');
	for(;c != nil; n = strchr(c, '/')) {
		if(n == nil) {
			fd = creat(cpath, 0755);
			if(fd < 0) {
				perror("creat");
				exit(1);
			}
			close(fd);

			return;
		}

		c = n;
		*c = '\0';
		if(stat(cpath, &st) < 0)
			mkdir(cpath, S_IRWXU);
		if(!S_ISDIR(st.st_mode)) {
			perror("mkrecpathfile");
			exit(1);
		}

		*c++ = '/';
	}

	return;
}

void
clearpath(char *str)
{
	char *b, *c;

	c = strchr(str, '/');
	for(; c != nil; c = strchr(b, '/')) {
		*c++ = '_';
		b = c;
	}

	return;
}

char *
mkcachestr(sdb *db, char *path)
{
	char *home, *fpath, *d, *c, *a, *b;
	sdbe *e, *r;

	a = nil;
	c = nil;
	fpath = nil;

	home = getenv("HOME");
	if(home == nil)
		home = "/tmp";

	e = getelem(db, path, nil);
	if(e == nil)
		goto badend;
	a = gmemdup(path, strlen(path) + 1);
	b = strrchr(a, '/');
	if(b == nil)
		goto badend;
	*b = '\0';

	r = getelem(db, a, nil);
	if(r == nil)
		goto badend;
	free(a);
	a = gmemdup(r->v + sizeof(char) + sizeof(time_t), strlen(r->v +
			sizeof(char) + sizeof(time_t)) + 1);
	b = strchr(a, '/');
	if(b == nil)
		goto badend;
	*b = '\0';

	c = gmemdup(e->v + sizeof(char) + sizeof(time_t), strlen(e->v +
			sizeof(char) + sizeof(time_t)) + 1);
	d = strchr(c, '/');
	if(d == nil)
		goto badend;
	*d++ = '\0';
	clearpath(d);
	fpath = greallocz(nil, strlen(home) + strlen(a) + strlen(d) +
			strlen(GOPHERPATH) + strlen(path) + 3, 2);
	sprintf(fpath, "%s%s%s/%s", home, GOPHERPATH, a, d);

badend:
	if(a != nil)
		free(a);
	if(c != nil)
		free(c);

	return fpath;
}

int
checkqueue(sdb *db, char *path)
{
	char *fpath;
	sdbe *e;
	struct stat st;
	int res;

	res = 0;

	e = getelem(db, path, nil);
	if(e == nil)
		return -1;

	fpath = mkcachestr(db, path);
	if(fpath == nil)
		return -1;
	if(stat(fpath, &st) < 0) {
		res = -1;
		goto badend;
	}

	if(time(nil) - st.st_mtime > FCACHE) {
		res = -1;
		goto badend;
	}

badend:
	free(fpath);

	return res;
}

int
downloadfile(sdb *db, char *path)
{
        char *fpath;
        sdbe *e;
        struct stat st;
        int fd, res;

        res = 0;

        e = getelem(db, path, nil);
        if(e == nil)
                return -1;

        fpath = mkcachestr(db, path);
        if(fpath == nil)
                return -1;
        if(stat(fpath, &st) < 0)
		mkrecpathfile(fpath);

	fd = open(fpath, O_WRONLY);
	if(fd < 0) {
		res = -1;
		goto badend;
	}

	if(rnpggopher(e->v + sizeof(char) + sizeof(time_t), db, e->k, fd) < 0) {
		close(fd);
		res = -1;
		goto badend;
	}
	close(fd);

badend:
	free(fpath);

	return res;
}

int
getbytes(sdb *db, char *path, char *buf, int size, int offset)
{
        char *fpath;
        sdbe *e;
        struct stat st;
        int fd, res;

        res = 0;

        e = getelem(db, path, nil);
        if(e == nil)
                return -1;

        fpath = mkcachestr(db, path);
        if(fpath == nil)
                return -1;
        if(stat(fpath, &st) < 0) {
                res = -1;
                goto badend;
        }

        if(st.st_size <= offset) {
                res = 0;
                goto badend;
        }

	fd = open(fpath, O_RDONLY);
	if(fd < 0) {
		res = -1;
		goto badend;
	}

	if(lseek(fd, offset, SEEK_SET) < offset) {
		close(fd);
		res = -1;
		goto badend;
	}

	res = (offset + size > st.st_size) ? size - (offset + size -
		st.st_size) : size;
	if(read(fd, buf, res) < 0) {
		close(fd);
		res = -1;
		goto badend;
	}
	close(fd);

badend:
	free(fpath);

	return res;
}

