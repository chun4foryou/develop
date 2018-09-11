CC = gcc

CFLAGS = $(DEFS) -ggdb -O2 -Wall -I./

fseek : fseek.o
	$(CC) $(CFLAGS) -o $@ $^ 

clean :
	rm -rf fseek *.o fseek_struct_test  fseek_test.txt
