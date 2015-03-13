all: cat
	LD_LIBRARY_PATH="../lib"

run: all
	./cat

cat: cat.c
	cc -o cat -L../lib/ -lhelpers -Wl,-rpath=../lib/ cat.c

clean:
	rm -f cat
