all: filter
	LD_LIBRARY_PATH="../lib"

filter: filter.c
	cc -o filter filter.c -I../lib/ -L../lib/ -lhelpers -Wl,-rpath=../lib/

clean:
	rm -f filter
