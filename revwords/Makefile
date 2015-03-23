all: revwords
	LD_LIBRARY_PATH="../lib"

revwords: revwords.c
	cc -o revwords revwords.c -I../lib/ -L../lib/ -lhelpers -Wl,-rpath=../lib/

clean:
	rm -f revwords
