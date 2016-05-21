CXX = g++
LINKER = g++
CXX_FLAGS = -Wall -pedantic -Wno-long-long -g -O2
LDFLAGS = 
LDGLFLAGS = -lGL -lGLU -lglut -lGLEW 

all: mp1

main.o: main.cpp
	$(CXX) $(CXX_FLAGS) -c main.cpp

mp1: main.o 
	$(LINKER) $(LDFLAGS) main.o -o mp1 $(LDGLFLAGS)

clean:
	rm -f *.o mp1
