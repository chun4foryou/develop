cc = gcc

TARGET = hello

$(TARGET) : hello.o
	$(cc) -o $@ $^

hello.o : hello.c
	$(cc) -c $@ $^

clean :
	rm -rf $(TARGET)
