CC=gcc

LIB= -lpthread

all: test

test:
	gcc -o multi fifo_queue.c  main.c ${LIB}

clean:
	rm -rf multi
