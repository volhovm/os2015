all: buffilter
	LD_LIBRARY_PATH="../lib"

buffilter: buffilter.c buffilter.h
	cc -g -o buffilter buffilter.c buffilter.h -I../lib/ -I. -L../lib/ -lbufio -lhelpers -Wl,-rpath=../lib/

clean:
	rm -f buffilter
