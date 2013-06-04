CFLAGS=-g3 -Wall -pedantic -Wextra -std=c99 -I../cnbt
#CFLAGS=-O3 -Wall -pedantic -Wextra -std=c99 -I../cnbt
LDFLAGS=-static -L../cnbt -lnbt $(shell pkg-config --libs zlib)

all: mcowl

mcowl: mcowl.o render.o blocks.o bitmap.o
	$(CC) -o mcowl mcowl.o render.o blocks.o bitmap.o $(LDFLAGS)

mcowl.o:  mcowl.c  mcowl.h
render.o: render.c render.h blocks.h mcowl.h
blocks.o: blocks.c blocks.h
bitmap.o: bitmap.c bitmap.h

clean:
	rm -f mcowl *.o
