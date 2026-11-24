CC = gcc
CFLAGS = -std=gnu17 -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined

test: test_dynarray
	./test_dynarray

test_dynarray: test_dynarray.o dynarray.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c dynarray.h
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o test_dynarray

.PHONY: test clean
