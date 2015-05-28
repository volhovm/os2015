all: simplesh
	LD_LIBRARY_PATH="../lib"

simplesh: simplesh.c
	cc -o simplesh simplesh.c  -I../lib/ -g -L../lib/ -lhelpers -Wl,-rpath=../lib/

clean:
	rm -f simplesh
