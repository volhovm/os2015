helpers.so: helpers.o
	gcc -o helpers.so -shared helpers.o
helpers.o: helpers.c helpers.h
	gcc -fPIC -c helpers.c helpers.h
clean:
	rm -f *.o *.so
