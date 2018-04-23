CC=gcc

DEFS =  -D_REENTRANT -DLINUX
INC = -I./ -I./library -I./library/curl/include -I./library/openssl/
CFLAGS = -Wall -ggdb $(INC) $(DEFS)
LIB = -L./library/curl/lib -lcurl \
			-L./library/openssl/lib -lssl -lcrypto \
			-lrt -ldl

all: curl_multi multi_download

curl_multi: curl_multi.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIB)

multi_download: multi_download.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIB)

clean:
	rm -rf *.o curl_multi multi_download 
