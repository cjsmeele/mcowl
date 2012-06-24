#CC=i686-pc-mingw32-gcc
#CFLAGS=-O2 -Wall -pedantic -Wextra -std=c99 -I../../dev/cnbt-mingw -I../libpnm -I/usr/i686-pc-mingw32/usr/include/libnpg15
#LDFLAGS=-L../libpnm -L../../dev/cnbt-mingw -L/usr/i686-pc-mingw32/usr/lib64

CFLAGS=-O2 -Wall -pedantic -Wextra -std=c99 -I../../dev/cnbt -I../libpnm -I/usr/include/libnpg15
LDFLAGS=-L../libpnm -L../../dev/cnbt

all: mcowl pnm2png

mcowl: mcowl.o render.o
	$(CC) $(LDFLAGS) mcowl.o render.o -o mcowl -static -lnbt -lpnm -lz

pnm2png: pnm2png.o
	$(CC) $(LDFLAGS) pnm2png.o -o pnm2png -static -lpnm -lpng -lz -lm 

mcowl.o: mcowl.c mcowl.h
render.o: render.c legend.h mcowl.h
pnm2png.o: pnm2png.c

# note: test regions are not yet included

test: mcowl pnm2png
	./mcowl testworld testregions/loca/Midpoint/region/*.mca

clean:
	rm -f mcowl pnm2png *.o
