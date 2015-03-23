all: lenwords
	LD_LIBRARY_PATH="../lib"

lenwords: lenwords.c
	cc -o lenwords -L../lib/ -lhelpers -Wl,-rpath=../lib/ lenwords.c

clean:
	rm -f lenwords
