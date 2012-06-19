CFLAGS=-O1 -Wall -std=c99 -I../cnbt

all: mcowl

mcowl: mcowl.o libnbt.a
	$(CC) $(CFLAGS) mcowl.o -L. -lnbt -lz -o mcowl

mcowl.o: mcowl.c

test: mcowl
	./mcowl r.-1.0.mca

clean:
	rm -f mcowl mcowl.o
