all: revwords
	LD_LIBRARY_PATH="../lib"

revwords: revwords.c
	cc -o revwords -L../lib/ -lhelpers -Wl,-rpath=../lib/ revwords.c

clean:
	rm -f revwords
