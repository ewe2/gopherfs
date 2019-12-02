PROGRAM = gopherfs
VERSION = -0.7

PREFIX ?= /usr

CC = cc -g 

CFLAGS = -D_FILE_OFFSET_BITS=64 -O2 -Wall -I. -I/usr/include/fuse
LDFLAGS = -L/usr/lib -L. -lfuse -lpthread

CFILES = gopherfs.c proto.c sdb.c ind.c download.c

OBJECTS = ${CFILES:.c=.o}

all:	$(PROGRAM)

${PROGRAM} : ${OBJECTS}
	${CC} -o ${PROGRAM} ${OBJECTS} ${LDFLAGS}

.SUFFIXES : .c .H

.c.o :
	${CC} ${CFLAGS} -c $<
.c :
	${CC} ${CFLAGS} -c $<


clean :
	@rm -f *.o ${PROGRAM} core *~

install: $(PROGRAM)
	@mkdir -p ${PREFIX}/bin
	@cp -f ${PROGRAM} ${PREFIX}/bin
	@chmod 755 ${PREFIX}/bin/${PROGRAM}

uninstall:
	@rm -f ${PREFIX}/bin/$(PROGRAM)

dist:
	@mkdir -p "${PROGRAM}${VERSION}"
	@ln README Makefile *.c *.h "${PROGRAM}${VERSION}"
	@tar -cf "${PROGRAM}${VERSION}.tar" "${PROGRAM}${VERSION}"
	@gzip "${PROGRAM}${VERSION}.tar"
	@mv "${PROGRAM}${VERSION}.tar.gz" "${PROGRAM}${VERSION}.tgz"
	@rm -rf "${PROGRAM}${VERSION}"

