CC = gcc

CFLAGS = -g -Wall -I./ -I/usr/include/python2.7 

LIB = -lpthread -lpython2.7

all:
	$(CC) $(CFLAGS) -o api_server api_server.c $(LIB)

clean:
	rm -rf *.pyc
