# Define (atleast) the following targets: all, clean
# all must build the executable file named mapreduce.
# clean must remove all object files and executable files created.
# Look at the introduction doc for information on how make a Makefile.
# This make file must build according to the structure described in the
# introduction doc, if it doesn't you will get a ZERO!
CFLAGS = -Wall -Werror

all:
	gcc $(CFLAGS) ./src/map_reduce.c ./src/main.c -o map_reduce

clean:
	rm -f *.o map_reduce