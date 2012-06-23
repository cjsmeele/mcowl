CFLAGS=-O2 -Wall -pedantic -Wextra -std=c99 -I../../dev/cnbt -I../libpnm

all: mcowl

mcowl: mcowl.o render.o
	$(CC) $(CFLAGS) mcowl.o render.o -L../../dev/cnbt -L../libpnm -o mcowl -static -lnbt -lz -lpnm

mcowl.o: mcowl.c mcowl.h
render.o: render.c legend.h mcowl.h

# note: test regions are not yet included

test: mcowl
	./mcowl testworld testregions/loca/Midpoint/region/*.mca
	convert world.pnm world.png

#	./mcowl testworld testregions/mp/r.-1.0.mca testregions/mp/r.0.0.mca testregions/mp/r.-1.-1.mca
#	./mcowl testworld testregions/mp/r.-1.0.mca testregions/mp/r.-1.-1.mca
#	./mcowl testworld testregions/mp/r.-1.0.mca


clean:
	rm -f mcowl *.o
