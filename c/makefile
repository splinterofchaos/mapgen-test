CC=gcc
CFLAGS=-Wall

COMPILE=${CC} ${CFLAGS}

mapgen : mapgen.c .bsp
	${COMPILE} -o mapgen mapgen.c .bsp

.bsp : bsp.* mapgen.h
	${COMPILE} -c -o .bsp bsp.c
