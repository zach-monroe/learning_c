CC = gcc
CFLAGS = -std=gnu17 -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined
ls: ls.o
	$(CC) $(CFLAGS)  $< -o $@
ls.o: ls.c 
	$(CC) -c $(CFLAGS)  $< -o $@
clean: 
	rm -rf *.o ls
.PHONY: clean
