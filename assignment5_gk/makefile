particle_system : cs415-ass5.o
	g++ -o particle_system cs415-ass5.o -I/usr/include -L/usr/lib -lGL -lGLU -lglut -lXmu -lglui
cs415-ass5.o : cs415-ass5.cpp
	g++ -c cs415-ass5.cpp

clean:
	rm particle_system
	rm -rf *.o
	rm -rf *~
