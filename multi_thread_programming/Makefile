CC=gcc

DEFS= -ggdb -Wall
LIB= -lpthread

all: test

test:
	gcc ${DEFS} -o multi fifo_queue.c  main.c ${LIB}

clean:
	rm -rf multi
