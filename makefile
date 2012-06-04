
CXX = g++ -std=c++0x -Wall
CC  = gcc

mg_obj = .random.o

mapgen : mapgen.cpp Vector.h Grid.h ${mg_obj}
	${CXX} mapgen.cpp ${mg_obj} -o mapgen

.random.o : random.*
	${CXX} -c random.cpp -o .random.o
