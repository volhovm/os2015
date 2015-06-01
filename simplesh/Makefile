all: simplesh libs
	LD_LIBRARY_PATH="../lib"

simplesh: simplesh.c
	clang -o simplesh simplesh.c  -I../lib/ -g -L../lib/ -lhelpers -lbufio -Wl,-rpath=../lib/

libs:
	make -C ../lib

clean:
	rm -f simplesh
