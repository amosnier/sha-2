CFLAGS = -O3 -Wall -Wextra -Wpedantic -std=c99

test: test.o sha-256.o

test.o: sha-256.h test.c

sha-256.o: sha-256.h sha-256.c

.PHONY: all
all: test
	./test

.PHONY: clean
clean:
	rm -f test *.o
