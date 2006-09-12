CC=g++
CPPPLAGS+=-Wall -Werror -g
CPPFLAGS+=-I/usr/include
CPPFLAGS+=-L/usr/lib
LDFLAGS= -lGL -lGLU -lglut -lXmu
PROG=triangles

SOURCES=$(wildcard *.C)
OBJECTS=$(SOURCES:.C=.o)

$(PROG) : $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(CPPFLAGS) $(LDFLAGS)

clean:
	rm -f $(OBJECTS) *~

cleanall:
	rm -rf $(PROG) $(OBJECTS) *~
