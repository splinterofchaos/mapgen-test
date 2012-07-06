
CXX = g++ -std=c++0x 
CC  = gcc

CFLAGS=-Wall -Wextra

COMPILE=${CXX} ${CFLAGS}

obj = .random.o .bsp.o .grid.o

mapgen : mapgen.cpp Vector.h Grid.h Bsp.h ${obj}
	${COMPILE} mapgen.cpp ${obj} -o mapgen

.bsp.o : Bsp.* mapgen.h .grid.o
	${COMPILE} -c Bsp.cpp -o .bsp.o

.grid.o : Grid.* .random.o Vector.h
	${COMPILE} -c Grid.cpp -o .grid.o

.random.o : random.*
	${COMPILE} -c random.cpp -o .random.o

