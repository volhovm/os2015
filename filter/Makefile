all: filter
	LD_LIBRARY_PATH="../lib"

filter: filter.c filter.h
	cc -o filter filter.c filter.h -I../lib/ -L../lib/ -lhelpers -Wl,-rpath=../lib/

clean:
	rm -f filter
