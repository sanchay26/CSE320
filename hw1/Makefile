# Define (atleast) the following targets: all, clean
# all must build the executable file named mapreduce.
# clean must remove all object files and executable files created.
# Look at the introduction doc for information on how make a Makefile.
# This make file must build according to the structure described in the
# introduction doc, if it doesn't you will get a ZERO!
CFLAGS = -Wall -Werror


all:clean gcc1 makefolder 
	@mkdir -p bin
	gcc -g -o bin/mapreduce build/main.o build/map_reduce.o 

gcc1:	
	@mkdir -p build
	gcc $(CFLAGS) -c ./src/main.c -o build/main.o

makefolder:
	@mkdir -p build
	gcc $(CFLAGS) -c ./src/map_reduce.c -o build/map_reduce.o

clean:
	rm -f *.o map_reduce