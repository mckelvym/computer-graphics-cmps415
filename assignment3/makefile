CC=g++
CFLAGS=-I/usr/include -L/usr/lib
LDFLAGS=-lglut -lGL -lGLU -lXmu
BINDIR=~/bin
PROGS=ass

OBJECTS=assignment3_v3.o
ass : ${OBJECTS}
	${CC} -o $@ ${OBJECTS} ${CFLAGS} ${LDFLAGS}

all: ${PROGS}
	make install -s
clean:
	rm -f ${PROGS} *.o *~
install:
	cp ${PROGS} ${BINDIR}
