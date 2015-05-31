all: simplesh
	LD_LIBRARY_PATH="../lib"

simplesh: simplesh.c
	clang -o simplesh simplesh.c  -I../lib/ -g -L../lib/ -lhelpers -lbufio -Wl,-rpath=../lib/

clean:
	rm -f simplesh
