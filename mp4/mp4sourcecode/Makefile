CXX = g++
LINKER = g++
CXX_FLAGS = -Wall -pedantic -Wno-long-long -g -O2
LDFLAGS = 
LDGLFLAGS = -lGL -lGLU -lglut -lGLEW -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf -lz -lm

all: mp4

main.o: main.cpp
	$(CXX) $(CXX_FLAGS) -c main.cpp

mp4: main.o 
	$(LINKER) $(LDFLAGS) main.o -o mp4 $(LDGLFLAGS)

clean:
	rm -f *.o mp4
