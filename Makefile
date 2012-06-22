CFLAGS=-O2 -Wall -std=c99 -I../../dev/cnbt

all: mcowl

mcowl: mcowl.o render.o libnbt.a
	$(CC) $(CFLAGS) mcowl.o render.o -L. -static -lnbt -lz -o mcowl

mcowl.o: mcowl.c mcowl.h
render.o: render.c legend.h mcowl.h

# note: test regions are not yet included

test: mcowl
	./mcowl testworld testregions/mp/*.mca
#	./mcowl testworld testregions/mp/r.-1.0.mca testregions/mp/r.0.0.mca testregions/mp/r.-1.-1.mca
#	./mcowl testworld testregions/mp/r.-1.0.mca testregions/mp/r.-1.-1.mca
#	./mcowl testworld testregions/mp/r.-1.0.mca


clean:
	rm -f mcowl *.o
