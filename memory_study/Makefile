CC=gcc

CFLAGS= -Wall -ggdb
DEFS= -I./ 

all: share_memory share_memory_view share_memory_clear

share_memory: share_memory.o
	gcc $(CFLAGS) -o $@ $^

share_memory_view: share_memory_view.o
	gcc $(CFLAGS) -o $@ $^

share_memory_clear: share_memory_clear.o
	gcc $(CFLAGS) -o $@ $^

clean:
	rm -rf *.o share_memory share_memory_view share_memory_clear

