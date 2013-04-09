#CFLAGS=-O0 -g3 -Wall -pedantic -Wextra -std=c99 -I../cnbt
CFLAGS=-O3 -Wall -pedantic -Wextra -std=c99 -I../cnbt
LDFLAGS=-L../cnbt

all: mcowl

mcowl: mcowl.o render.o blocks.o bitmap.o
	$(CC) $(LDFLAGS) mcowl.o render.o blocks.o bitmap.o -o mcowl -static -lnbt -lz

mcowl.o:  mcowl.c  mcowl.h
render.o: render.c render.h blocks.h mcowl.h
blocks.o: blocks.c blocks.h
bitmap.o: bitmap.c bitmap.h

clean:
	rm -f mcowl *.o
