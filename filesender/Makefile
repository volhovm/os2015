all: filesender
	LD_LIBRARY_PATH="../lib"

filesender: filesender.c libs
	clang -g -o filesender filesender.c  -I../lib/ -L../lib/ -lbufio -Wl,-rpath=../lib/

libs:
	make -C ../lib

clean:
	rm -f filesender
