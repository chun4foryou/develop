CC = gcc

CFLAGS = $(DEFS) -ggdb -O2 -Wall -I./

all : fseek rewind

fseek : fseek.o
	$(CC) $(CFLAGS) -o $@ $^ 

rewind : rewind.o
	$(CC) $(CFLAGS) -o $@ $^ 

clean :
	rm -rf fseek *.o fseek_struct_test  fseek_test.txt rewind
