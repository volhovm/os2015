all: cat
	LD_LIBRARY_PATH="../lib"

cat: cat.c
	cc -o cat cat.c  -I../lib/ -L../lib/ -lhelpers -Wl,-rpath=../lib/

clean:
	rm -f cat
