all: polling forking
	LD_LIBRARY_PATH="../lib"

forking: forking.c libs
	clang -g -o forking forking.c  -I../lib/ -L../lib/ -lbufio -Wl,-rpath=../lib/

polling: polling.c libs
	clang -g -o polling polling.c  -I../lib/ -L../lib/ -lbufio -Wl,-rpath=../lib/

libs:
	make -C ../lib

clean:
	rm -f forking polling
