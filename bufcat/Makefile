all: bufcat
	LD_LIBRARY_PATH="../lib"

bufcat: bufcat.c
	cc -o bufcat bufcat.c  -I../lib/ -L../lib/ -lbufio -Wl,-rpath=../lib/

clean:
	rm -f bufcat
