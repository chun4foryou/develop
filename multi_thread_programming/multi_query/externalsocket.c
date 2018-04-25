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
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/
/* <DESC>
 * An example demonstrating how an application can pass in a custom
 * socket to libcurl to use. This example also handles the connect itself.
 * </DESC>
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#define close closesocket
#else
#include <sys/types.h>        /*  socket types              */
#include <sys/socket.h>       /*  socket definitions        */
#include <netinet/in.h>
#include <arpa/inet.h>        /*  inet (3) functions         */
#include <unistd.h>           /*  misc. Unix functions      */
#endif

#include <errno.h>

/* The IP address and port number to connect to */
#define IPADDR "127.0.0.1"
#define PORTNUM 80

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

typedef struct{
	char *memory;
	size_t size;
}CURL_buffer;

typedef struct{
	CURL_buffer header;
	CURL_buffer body;
}CURL_data;

typedef struct query_nfo{
  char ip[128];
  int  port;
  char url[1024];
}QueryInfo;

/**
* @brief response 데이터를 메모리에 write한다. 
*
* @param contents
* @param size
* @param nmemb
* @param userp
*
* @return 
*/
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
//  fprintf(stderr,"%s \n",(char*)contents);

	return realsize;
}

static int closecb(void *clientp, curl_socket_t item)
{
  (void)clientp;
  printf("libcurl wants to close %d now\n", (int)item);
  return 0;
}

static curl_socket_t opensocket(void *clientp,
                                curlsocktype purpose,
                                struct curl_sockaddr *address)
{
  curl_socket_t sockfd;
  (void)purpose;
  (void)address;
  sockfd = *(curl_socket_t *)clientp;
  /* the actual externally set socket is passed in via the OPENSOCKETDATA
     option */
  return sockfd;
}

static int sockopt_callback(void *clientp, curl_socket_t curlfd,
                            curlsocktype purpose)
{
  (void)clientp;
  (void)curlfd;
  (void)purpose;
  /* This return code was added in libcurl 7.21.5 */
  return CURL_SOCKOPT_ALREADY_CONNECTED;
}

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

int curl_process (CURL_data *resp, QueryInfo *url_info, char *data, int dsize,char *error_log,int log_size)
{
  CURL *curl = NULL ;
  CURLcode res;
  struct sockaddr_in servaddr;  /*  socket address structure  */
  curl_socket_t sockfd;
  char send_query[1024]={0,};
  int next_ssl=0;

  while (1){
    if( curl != NULL){
      curl_easy_cleanup(curl);
    }
    curl = curl_easy_init();
    if(curl) {
      /*
       * Note that libcurl will internally think that you connect to the host
       * and port that you specify in the URL option.
       */
      if( url_info->port == 80 ){
        snprintf(send_query,sizeof(send_query),"http://%s/%s", url_info->ip, url_info->url);
      } else {
        snprintf(send_query,sizeof(send_query),"https://%s:%d/%s", url_info->ip,url_info->port, url_info->url);
      }
      fprintf(stderr,"%s\n",send_query);
      curl_easy_setopt(curl, CURLOPT_URL,send_query);

      /* Create the socket "manually" */
      sockfd = socket(AF_INET, SOCK_STREAM, 0);
      if(sockfd == CURL_SOCKET_BAD) {
        printf("Error creating listening socket.\n");
        return 3;
      }

      memset(&servaddr, 0, sizeof(servaddr));
      servaddr.sin_family = AF_INET;
      servaddr.sin_port   = htons(url_info->port);

      servaddr.sin_addr.s_addr = inet_addr(url_info->ip);
      if(INADDR_NONE == servaddr.sin_addr.s_addr)
        return 2;

      if(connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) ==
          -1) {
        close(sockfd);
        printf("client error: connect: %s\n", strerror(errno));
        return 1;
      }


      if( dsize > 0 ){
        /* Now specify the POST data */ 
        curl_easy_setopt(curl, CURLOPT_POST,1L);
        /* 문자열이 아닌것을 보내기 위해서 크기를 지정해 줘야한다 */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, dsize);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS,data);
      }
      /* no progress meter please */
      curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

      /* send all data to this function  */
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &(resp->body));
      curl_easy_setopt(curl, CURLOPT_WRITEHEADER, (void *) &(resp->header));

      /* call this function to get a socket */
      curl_easy_setopt(curl, CURLOPT_OPENSOCKETFUNCTION, opensocket);
      curl_easy_setopt(curl, CURLOPT_OPENSOCKETDATA, &sockfd);

      /* call this function to close sockets */
      curl_easy_setopt(curl, CURLOPT_CLOSESOCKETFUNCTION, closecb);
      curl_easy_setopt(curl, CURLOPT_CLOSESOCKETDATA, &sockfd);

      /* call this function to set options for the socket */
      curl_easy_setopt(curl, CURLOPT_SOCKOPTFUNCTION, sockopt_callback);
      curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

      /* Set SSL Version */
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);    
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
      if ( next_ssl == 0) {
        curl_easy_setopt(curl, CURLOPT_SSLVERSION,CURL_SSLVERSION_DEFAULT);
      } else if (next_ssl == 1 ){
        curl_easy_setopt(curl, CURLOPT_SSLVERSION,CURL_SSLVERSION_SSLv3);
      }

      res = curl_easy_perform(curl);
      if (res != CURLE_OK && next_ssl == 1)
      {
        snprintf(error_log, log_size,"%s",curl_easy_strerror(res));
        break;
      } else if (res != CURLE_OK && next_ssl < 1) {
        close(sockfd);
        next_ssl++;
      } else if (res == CURLE_OK) {
        break;
      } else {
        snprintf(error_log, log_size,"%s",curl_easy_strerror(res));
        break;
      }
    }
  }//end of while

  if( curl != NULL){
    curl_easy_cleanup(curl);
    close(sockfd);
  }

  return 0;
}


QueryInfo url_list[] = {
  {"175.113.83.132",4000,"sniper.atx?cmd=config&order=config_rule_compile&category=1500"},
  {"175.113.83.131",4000,"sniper.atx?cmd=config&order=config_rule_compile&category=1501"},
};

void* send_curl_query(void *_idx)
{
  CURL_data *resp= NULL;
  char error_log[1024]={0, };
  FILE *fp = NULL;
  char file_name[128]={0, };
  int idx= *(int*)_idx;

  snprintf(file_name, sizeof(file_name), "./save_file/save_file_%d.txt",idx);
  unlink(file_name);
  fp = fopen(file_name,"w");

  resp = (CURL_data *)malloc(sizeof(CURL_data));
  memset(resp,0,sizeof(CURL_data));

  curl_process (resp, &url_list[idx], NULL, 0 ,error_log,sizeof(error_log));
  fprintf(fp,"[%3d]==================================\n",idx);
  fprintf(fp,"%s \n",resp->body.memory);
  fclose(fp);
  clear_data_ptr(resp);
  fprintf(stderr,"Wrrite OK[%d]\n",idx);
  free(_idx);
  return NULL;
}

int main(int argc , char** argv) {
  pthread_t thread_t;
  int i = 0 ;
  int *_idx=NULL;
  core();

  curl_global_init(CURL_GLOBAL_ALL);
  while (1 ){
    fprintf(stderr,"Start Query Send\n");
    for(i = 0; i < 2; i++){
      _idx = (int*)malloc(sizeof(int));
      *_idx = i; 
      if (pthread_create (&thread_t, NULL, send_curl_query, _idx) < 0);
      pthread_detach (thread_t);
    }
    sleep(10);
  }
  curl_global_cleanup();
  return 0;
}
