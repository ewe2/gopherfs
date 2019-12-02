/*
 * Copy me if you can.
 * by 20h
 */

#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include "ind.h"
#include "sdb.h"

void
tprintf(int sock, char *fmt, ...)
{
        va_list fmtargs;
	int fd2;
	FILE *fp;

	fd2 = dup(sock);
	fp = fdopen(fd2, "w");
	if(fp == nil) {
		perror("fdopen");
		return;
	}

        va_start(fmtargs, fmt);
        vfprintf(fp, fmt, fmtargs);
        va_end(fmtargs);

	fclose(fp);

        return;
}

char *
readln(int fd)
{
        char *ret;
        int len, st;

        len = 1;

        ret = greallocz(nil, 2, 2);
        while((st = read(fd, &ret[len - 1], 1)) > 0 && ret[len - 1] != '\n')
                ret = greallocz(ret, ++len + 1, 0);
	if(st < 0) {
		free(ret);
		return nil;
	}

        if(ret[len - 1] != '\n') {
                free(ret);
                return nil;
        }
        ret[len - 1] = '\0';
	if(ret[len - 2] == '\r')
		ret[len - 2] = '\0';

        return ret;
}

int
connecttcp(char *host, char *service)
{
        int sock;
	struct addrinfo *ai, *a;

	sock = -1;

	if(getaddrinfo(host, service, nil, &ai) < 0) {
		perror("getaddrinfo");
		return -1;
	}

	for(a = ai; a; a = a->ai_next) {
		sock = socket(a->ai_family, a->ai_socktype, a->ai_protocol);
		if(sock < 0) {
			perror("socket");
			sock = -1;
			break;
		}

		if(connect(sock, a->ai_addr, a->ai_addrlen) < 0) {
			perror("connect");
			sock = -1;
			break;
		} else
			break;
	}

	freeaddrinfo(ai);

        return sock;
}

int
sendgopherreq(char *server, char *port, char *str)
{
	int sock;

	sock = connecttcp(server, port);
	if(sock < 0)
		return -1;

	tprintf(sock, "%s\r\n", str);

	return sock;
}

void
adddbentry(sdb *db, char type, char *dpath, char *path, char *name,
		char *server, char *port)
{
	char *uri, *more;
	char *rdp;
	int mlen;
	time_t tim;
	sdbe *e;

	if(dpath[strlen(dpath) - 1] == '/')
		rdp = "";
	else
		rdp = "/";
	uri = greallocz(nil, strlen(dpath) + strlen(rdp) + strlen(name) + 1, 2);
        sprintf(uri, "%s%s%s", dpath, rdp, name);

	mlen = sizeof(char) + sizeof(time_t) + strlen(server) + strlen(port) +
		strlen(path) + 3;
	more = greallocz(nil, mlen, 2); 
        tim = time(nil);
        more[0] = type;
        memcpy(more + sizeof(char), &tim, sizeof(tim));
        sprintf(more + sizeof(char) + sizeof(time_t), "%s:%s/%s", server,
			port, path); 

        e = getelem(db, uri, nil);
        if(e != nil) {
		free(e->v);
                e->v = gmemdup(more, mlen);
                e->l = mlen;
        } else
                addelem(db, uri, more, mlen);

	free(more);
	free(uri);

	return;
}

int
parsegopher(int sock, sdb *db, char *dpath)
{
	char *ln, type, *name, *path, *server, *port, *end;

	while((ln = readln(sock)) != nil) {
		type = ln[0];
		name = ln + 1;
		path = strchr(name, '\t');
		if(path == nil) {
			free(ln);
			continue;
		}
		*path++ = '\0';
		server = strchr(path, '\t');
		if(server == nil) {
			free(ln);
			continue;
		}
		*server++ = '\0';
		port = strchr(server, '\t');
		if(port == nil) {
			free(ln);
			continue;
		}
		*port++ = '\0';
		end = strchr(port, '\t');
		if(end != nil)
			*end = '\0';
		end = strchr(port, '\r');
		if(end != nil)
			*end = '\0';

		if(name[0] == '.') {
			if(name[1] == '\0') {
				free(ln);
				continue;
			}
			if(name[1] == '.' && name[2] == '\0') {
				free(ln);
				continue;
			}
		}

		adddbentry(db, type, dpath, path, name, server, port);
		free(ln);
	}

	return 0;
}

void
writetofile(int sock, int fd)
{
	char buf[8192];
	int l;

	for(;;) {
		l = read(sock, buf, sizeof(buf));
		if(l <= 0)
			return;
		write(fd, buf, l);
	}

	return;
}

int
rnpggopher(char *srv, sdb *db, char *dpath, int fd)
{
	char *server, *port, *path;
	int sock, res;

	res = 0;

	server = gmemdup(srv, strlen(srv) + 1);
	port = strchr(server, ':');
	path = strchr(server, '/');
	if(port == nil)
		port = "70";
	else
		*port++ = '\0';
	if(path == nil)
		path = "";
	else
		*path++ = '\0';

	sock = sendgopherreq(server, port, path);
	if(sock < 0) {
		perror("sendgopherreq");
		res = -1;
	} else {
		if(fd < 0)
			parsegopher(sock, db, dpath);
		else
			writetofile(sock, fd);
	}

	free(server);
	close(sock);

	return res;
}

int
checkcache(sdb *db, char *path)
{
	sdbe *e;
	time_t tim;
	int i;

	e = getelem(db, path, nil);
	for(i = 0; e != nil; e = getelem(db, path, e), i++) {
		if(!strcmp(e->k, (char *)path))
			continue;
		if(strchr(e->k + strlen(path) + 1, '/'))
			continue;

		memmove(&tim, e->v + sizeof(char), sizeof(time_t));
		if(time(nil) - tim > ECACHE)
			return 1;
	}

	if(i < 2)
		return 1;

	return 0;
}

