CC = gcc

CFLAGS = -g -Wall -O2

all : pipe named_pipe_client named_pipe_server\
			message_queue_constructer message_queue_consumer\
			share_memory share_memory_view share_memory_clear memory_map

pipe : pipe.o
	$(CC) $(CFLAGS) -o $@ $^

named_pipe_client : named_pipe_client.o 
	$(CC) $(CFLAGS) -o $@ $^

named_pipe_server : named_pipe_server.o 
	$(CC) $(CFLAGS) -o $@ $^

message_queue_constructer : message_queue_constructer.o 
	$(CC) $(CFLAGS) -o $@ $^

message_queue_consumer : message_queue_consumer.o 
	$(CC) $(CFLAGS) -o $@ $^

share_memory: share_memory.o
	gcc $(CFLAGS) -o $@ $^

share_memory_view: share_memory_view.o
	gcc $(CFLAGS) -o $@ $^

share_memory_clear: share_memory_clear.o
	gcc $(CFLAGS) -o $@ $^

memory_map: memory_map.o
	gcc $(CFLAGS) -o $@ $^

clean :
	rm -rf pipe named_pipe_client named_pipe_server *.o named_pipe_file\
		message_queue_constructer message_queue_consumer share_memory\
		share_memory_view share_memory_clear


