CC = gcc
CFLAGS = -g -Wall -Werror -pedantic -Wextra -I include


all: clean utf

utf:
	@mkdir -p build
	@mkdir -p bin
	
	$(CC) $(CFLAGS) -c src/utfconverter.c -o build/utfconverter.o
	$(CC) -g src/utfconverter.c -o bin/utf

clean:
	rm -rf build
	rm -rf bin

