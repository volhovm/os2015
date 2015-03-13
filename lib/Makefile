libhelpers.so: helpers.o
	gcc -o libhelpers.so -shared -Wall -I. helpers.o
helpers.o: helpers.c helpers.h
	gcc -fpic -c helpers.c helpers.h
clean:
	rm -f *.o *.so
