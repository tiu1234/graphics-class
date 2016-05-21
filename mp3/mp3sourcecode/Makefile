CXX = g++
LINKER = g++
CXX_FLAGS = -Wall -pedantic -Wno-long-long -g -O2
LDFLAGS = 
LDGLFLAGS = -lGL -lGLU -lglut -lGLEW -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf -lz -lm

all: mp3

main.o: main.cpp
	$(CXX) $(CXX_FLAGS) -c main.cpp

mp3: main.o 
	$(LINKER) $(LDFLAGS) main.o -o mp3 $(LDGLFLAGS)

clean:
	rm -f *.o mp3
