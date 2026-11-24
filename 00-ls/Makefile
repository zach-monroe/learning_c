CC = gcc
CFLAGS = -std=gnu17 -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined
DYN = ../01-dynarray

ls: ls.o $(DYN)/dynarray.o
	$(CC) $(CFLAGS) $^ -o $@

ls.o: ls.c $(DYN)/dynarray.h
	$(CC) -c $(CFLAGS) $< -o $@

$(DYN)/dynarray.o: $(DYN)/dynarray.c $(DYN)/dynarray.h
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf *.o ls
.PHONY: clean
