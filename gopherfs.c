/*
 * Copy me if you can.
 * by 20h
 */

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include "ind.h"
#include "arg.h"
#include "sdb.h"
#include "proto.h"
#include "download.h"

sdb *db;
char *argv0;

static int
gopendir(const char *path, struct fuse_file_info *fi)
{
	sdbe *e;

	e = getelem(db, (char *)path, nil);
	if(e == nil)
		return ~ENOENT;

	return 0;
}


static int
ggetattr(const char *path, struct stat *stbuf)
{
	int res;
	sdbe *e;

	res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if(!strcmp(path, "/")) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 1;
		stbuf->st_size = 1;
	} else {
		e = getelem(db, (char *)path, nil);
		if(e == nil)
			return ~ENOENT;
		if(e->v[0] == '1') {
			stbuf->st_mode = S_IFDIR | 0444;
			stbuf->st_nlink = 2;
		} else {
			stbuf->st_mode = S_IFREG | 0444;
			stbuf->st_nlink = 1;
			stbuf->st_size = 1;
		}
	}

	return res;
}

static int
greaddir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
			struct fuse_file_info *fi)
{
	sdbe *e;
	char *v, *d;

	e = nil;

	filler(buf, ".", 0, 0);
	filler(buf, "..", 0, 0);

	e = getelem(db, (char *)path, e);
	if(e == nil)
		return 0;

        if(checkcache(db, e->k) == 1)
                if(rnpggopher(e->v + sizeof(char) + sizeof(time_t), db, e->k,
                                -1) < 0)
                        return 0;

	for(; e != nil; e = getelem(db, (char *)path, e)) {
		if(!strcmp(e->k, (char *)path))
			continue;
		if(strchr(e->k + strlen(path) + 1, '/'))
			continue;
		v = gmemdup(e->k, strlen(e->k) + 1);
		d = strrchr(v, '/');
		if(d == nil)
			continue;
		*d++ = '\0';
		filler(buf, d, 0, 0);
		free(v);
	}

	return 0;
}

static int
gopen(const char *path, struct fuse_file_info *fi)
{

        if(checkqueue(db, (char *)path) < 0)
                if(downloadfile(db, (char *)path) < 0)
                        return ~ENOENT;

	fi->direct_io = 1;

	return 0;
}

static int
gread(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi)
{

	return getbytes(db, (char *)path, buf, size, offset);
}

static struct fuse_operations goper = {
	.getattr = ggetattr,
	.readdir = greaddir,
	.opendir = gopendir,
	.open = gopen,
	.read = gread,
};

void
usage(void)
{

	tprintf(2, "usage: %s [-h] URI mntpnt [-o option[,...]]\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	char *server, *port, *path, type;
	char *p, *b, *c, *d;
	int res;

	db = createdb();
	path = "";
	type = '1';
	c = nil;
	res = 1;

	ARGBEGIN {
	case 'h':
	default:
		usage();
	} ARGEND;

	if(argc == 0)
		usage();

	if(argc > 0 && argv[0] != nil) {
		c = gmemdup(argv[0], strlen(argv[0]) + 1);
		p = strstr(c, "gopher://");
		if(p == nil) {
			tprintf(2, "Wrong URI Syntax\n");
			goto badending;
		}
		p += 9;
		server = p;

		b = strchr(p, ':');
		d = strchr(p, '/');
		if(b > d)
			b = nil;
		if(b != nil) {
			*b++ = '\0';
			port = b;
		} else
			port = "70";
		if(d != nil) {
			*d++ = '\0';
			type = *d;
			p = strchr(d, '/');
			if(p != nil) {
				*p++ = '\0';
				path = p;
			}
		}

		adddbentry(db, type, "", path, "", server, port);
		free(c);
	}

	res = fuse_main(argc, argv, &goper, nil);

badending:
	destroydb(db);

	return res;
}

