all: lenwords
	LD_LIBRARY_PATH="../lib"

lenwords: lenwords.c
	cc -o lenwords -I../lib/ -L../lib/ -lhelpers -Wl,-rpath=../lib/ lenwords.c

clean:
	rm -f lenwords
