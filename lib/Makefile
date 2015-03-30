all: libbufio.so libhelpers.so

libbufio.so: bufio.o
	gcc -o libbufio.so -shared -Wall -I. bufio.o
bufio.o: bufio.c bufio.h
	gcc -fpic -c bufio.c

libhelpers.so: helpers.o
	gcc -o libhelpers.so -shared -Wall -I. helpers.o
helpers.o: helpers.c helpers.h
	gcc -fpic -c helpers.c

clean:
	rm -f *.o *.so
