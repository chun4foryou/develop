/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2017, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/
/* <DESC>
 * Source code using the multi interface to download many
 * files, with a capped maximum amount of simultaneous transfers.
 * </DESC>
 * Written by Michael Wallner
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#ifndef WIN32
#  include <unistd.h>
#endif
#include <curl/multi.h>

typedef struct{
	char *memory;
	size_t size;
}CURL_buffer;

typedef struct{
	CURL_buffer header;
	CURL_buffer body;
}CURL_data;


static const char *urls[] = {
	"https://www.naver.com",
	"https://www.googl.com"
};

#define MAX 10 /* number of simultaneous transfers */
#define CNT sizeof(urls)/sizeof(char *) /* total number of transfers to do */

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	CURL_buffer *mem = (CURL_buffer *)userp;

	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL) {
		/* out of memory! */
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

#if 0
static size_t WriteMemoryCallback(char *d, size_t n, size_t l, void *p)
{
	/* take care of the data here, ignored in this example */
	fprintf(stderr,"%s",d);
	(void)d;
	(void)p;
	return n*l;
}
#endif

static void init(CURLM *cm, int i)
{
  CURL *curl = curl_easy_init();

  fprintf(stderr,"%s\n",urls[i]);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &(result->body));
  curl_easy_setopt(curl, CURLOPT_WRITEHEADER, (void *) &(result->header));
  curl_easy_setopt(curl, CURLOPT_URL, urls[i]);
  curl_easy_setopt(curl, CURLOPT_PRIVATE, urls[i]);
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

  curl_multi_add_handle(cm, curl);
}

static int core ()
{
  struct rlimit rl;


	if (getrlimit (RLIMIT_NOFILE, &rl) == -1){
		fprintf(stderr,"error, getrlimit rlimit_nofile\n");
		return -1;
	}
	rl.rlim_cur = rl.rlim_max;
	setrlimit (RLIMIT_NOFILE, &rl);

  if (getrlimit (RLIMIT_CORE, &rl) == -1){
		fprintf(stderr,"error, getrlimit rlimit_core\n");
		return -1;
	}
	rl.rlim_cur = rl.rlim_max;
  setrlimit (RLIMIT_CORE, &rl);

	fprintf(stderr,"Core file Enable !!!!\n");
	return 0;
}


int main(void)
{
	CURLM *cm;
	CURLMsg *msg;
	long L;
	unsigned int C = 0;
	int M, Q, U = -1;
	fd_set R, W, E;
	struct timeval T;
  CURL_data *resp=NULL;
	core();

	curl_global_init(CURL_GLOBAL_ALL);
  resp = (CURL_data *)malloc(sizeof(CURL_data));
  memset(resp,0,sizeof(CURL_data));

	cm = curl_multi_init();

	/* we can optionally limit the total amount of connections this multi handle
	 *      uses */
	curl_multi_setopt(cm, CURLMOPT_MAXCONNECTS, (long)MAX);

	for(C = 0; C < CNT; ++C) {
		init(cm, C);
	}

	while(U) {
		curl_multi_perform(cm, &U);

		if(U) {
			FD_ZERO(&R);
			FD_ZERO(&W);
			FD_ZERO(&E);

			if(curl_multi_fdset(cm, &R, &W, &E, &M)) {
				fprintf(stderr, "E: curl_multi_fdset\n");
				return EXIT_FAILURE;
			}

			if(curl_multi_timeout(cm, &L)) {
				fprintf(stderr, "E: curl_multi_timeout\n");
				return EXIT_FAILURE;
			}
			if(L == -1)
				L = 100;

			if(M == -1) {
				sleep((unsigned int)L / 1000);
			}
			else {
				T.tv_sec = L/1000;
				T.tv_usec = (L%1000)*1000;

				if(0 > select(M + 1, &R, &W, &E, &T)) {
					fprintf(stderr, "E: select(%i,,,,%li): %i: %s\n",
							M + 1, L, errno, strerror(errno));
					return EXIT_FAILURE;
				}
			}
		}

		while((msg = curl_multi_info_read(cm, &Q))) {
			if(msg->msg == CURLMSG_DONE) {
				char *url;
				CURL *e = msg->easy_handle;
				curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &url);
				fprintf(stderr, "R: %d - %s <%s>\n",
						msg->data.result, curl_easy_strerror(msg->data.result), url);
				curl_multi_remove_handle(cm, e);
				curl_easy_cleanup(e);
			}
			else {
				fprintf(stderr, "E: CURLMsg (%d)\n", msg->msg);
			}
			if(C < CNT ) {
				init(cm, C++);
				U++; /* just to prevent it from remaining at 0 if there are more
								URLs to get */
			}
		}
	}
	clear_data_ptr(resp);           

	curl_multi_cleanup(cm);
	curl_global_cleanup();

	return EXIT_SUCCESS;
}


#if 0
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	CURL_buffer *mem = (CURL_buffer *)userp;

	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL) {
		/* out of memory! */
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

/*
static size_t read_callback(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	curl_off_t nread;
	size_t retcode;
	size_t realsize = size * nmemb;
	
	file_buf.buf = realloc(file_buf.buf, file_buf.size + realsize + 1);
	retcode = fread(file_buf.buf, size, nmemb, stream);
	
	nread = (curl_off_t)retcode;
	//memcpy(&(file_buf.buf[file_buf.size]), ptr, realsize);
	file_buf.buf[file_buf.size] = 0;
	file_buf.size += realsize;

	return retcode;
}
*/

void clear_data_ptr(CURL_data *result)
{
	if(result->header.memory != NULL && result->header.size > 0){	
		free(result->header.memory);
		result->header.size = 0;
	}
	if(result->body.memory != NULL && result->body.size > 0){	
		free(result->body.memory);
		result->body.size = 0;
	}
	return;
}

int curl_process (CURL_data *result, char* send_query,char *data, int dsize,char *error_log,int log_size)
{
	CURLcode res;
	CURL *curl;

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();

	if (!curl)
	{
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		return ERROR_CURL_INIT;
	}

	clear_data_ptr(result);           
	result->header.memory = malloc(1);
	result->header.size = 0;          
	result->body.memory = malloc(1);  
	result->body.size = 0;            

	/* Now specify the POST data */ 
	curl_easy_setopt(curl, CURLOPT_URL, send_query);    

	if( dsize > 0 ){
		/* Now specify the POST data */ 
		curl_easy_setopt(curl, CURLOPT_POST,1L);
		/* 문자열이 아닌것을 보내기 위해서 크기를 지정해 줘야한다 */
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, dsize);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS,data);
	}
	//response처리 callback 등록
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &(result->body));
	curl_easy_setopt(curl, CURLOPT_WRITEHEADER, (void *) &(result->header));
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);    
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(curl, CURLOPT_SSLVERSION,CURL_SSLVERSION_DEFAULT);

	res = curl_easy_perform(curl);

	if (res != CURLE_OK)
	{
		snprintf(error_log, log_size,"%s",curl_easy_strerror(res));
		/* sslv3 로 다시 처리 */
		curl_easy_setopt(curl, CURLOPT_SSLVERSION,CURL_SSLVERSION_SSLv3);
		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			snprintf(error_log, log_size,"%s",curl_easy_strerror(res));
			sleep(1);
			return ERROR_CURL_SEND;
		}
		return 0;
	}

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	return 0;
}

#endif
