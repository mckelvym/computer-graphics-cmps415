#   I compiled and ran the sample.cpp program.
#   To compile the program in the WANT lab under Linux, 
#   save this as a makefile, and:
#
#   make sample
#
#   The above command should produce an output file named sample. 
#   To run the program
#
#   ./sample
#
# Makefile for OpenGL .cpp

##### MACROS #####

CC = g++

INCDIR = /usr/include

GL_LIBS = -lGL -lGLU -lglut -lXmu

LIB_DEP = /usr/lib/

##### RULES #####

.SUFFIXES:
.SUFFIXES: .cpp


.cpp: $(LIB_DEP)
	@echo $(CFLAGS) "-" $< "-" $@
	$(CC) $(CFLAGS) $<  -o $@ -I$(INCDIR) -L$(LIB_DEP) $(GL_LIBS)


## 

##### TARGETS ######

default:
	@echo "Specify a target configuration"

clean:
	-rm *.o *~

realclean:
	-rm $(PROGS)
	-rm *.o *~

targets: $(PROGS)
