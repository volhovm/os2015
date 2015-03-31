all: libbufio.so

libbufio.so: bufio.c bufio.h
	gcc -shared -Wall -I. -fpic -o libbufio.so $<

libhelpers.so: helpers.o
	gcc -o libhelpers.so -shared -Wall -I. helpers.o
helpers.o: helpers.c helpers.h
	gcc -fpic -c helpers.c

clean:
	rm -f *.o *.so
