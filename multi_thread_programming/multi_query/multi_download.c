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

static const char *urls[] = {
  "http://www.microsoft.com",
  "http://www.opensource.org",
  "http://www.google.com",
  "http://www.yahoo.com",
  "http://www.ibm.com",
  "http://www.mysql.com",
  "http://www.oracle.com",
  "http://www.ripe.net",
  "http://www.iana.org",
  "http://www.amazon.com",
  "http://www.netcraft.com",
  "http://www.heise.de",
  "http://www.chip.de",
  "http://www.ca.com",
  "http://www.cnet.com",
  "http://www.news.com",
  "http://www.wikipedia.org",
  "http://www.dell.com",
  "http://www.hp.com",
  "http://www.cert.org",
  "http://www.mit.edu",
  "http://www.nist.gov",
  "http://www.ebay.com",
  "http://www.playstation.com",
  "http://www.uefa.com",
  "http://www.ieee.org",
  "http://www.apple.com",
  "http://www.symantec.com",
  "http://www.zdnet.com",
  "http://www.fujitsu.com",
  "http://www.supermicro.com",
  "http://www.hotmail.com",
  "http://www.ecma.com",
  "http://www.bbc.co.uk",
  "http://news.google.com",
  "http://www.foxnews.com",
  "http://www.msn.com",
  "http://www.wired.com",
  "http://www.sky.com",
  "http://www.usatoday.com",
  "http://www.cbs.com",
  "http://www.nbc.com",
  "http://slashdot.org",
  "http://www.techweb.com",
  "http://www.newslink.org",
  "http://www.un.org",
};

#define MAX 10 /* number of simultaneous transfers */
#define CNT sizeof(urls)/sizeof(char *) /* total number of transfers to do */

typedef struct{
	char *memory;
	size_t size;
}CURL_buffer;

typedef struct{
	CURL_buffer header;
	CURL_buffer body;
}CURL_data;

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
static void init(CURLM *cm, int i, CURL_data *result)
{
  CURL *eh = curl_easy_init();

  curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(eh, CURLOPT_WRITEDATA, (void *) &(result->body));
	curl_easy_setopt(eh, CURLOPT_WRITEHEADER, (void *) &(result->header));
	curl_easy_setopt(eh, CURLOPT_URL, urls[i]);
  curl_easy_setopt(eh, CURLOPT_PRIVATE, urls[i]);
  curl_easy_setopt(eh, CURLOPT_VERBOSE, 0L);

  curl_multi_add_handle(cm, eh);
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

  curl_global_init(CURL_GLOBAL_ALL);

  cm = curl_multi_init();

  /* we can optionally limit the total amount of connections this multi handle
     uses */
  curl_multi_setopt(cm, CURLMOPT_MAXCONNECTS, (long)MAX);

  for(C = 0; C < MAX; ++C) {
    init(cm, C,resp);
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
#ifdef WIN32
				Sleep(L);
#else
				sleep((unsigned int)L / 1000);
#endif
			}
			else {
				T.tv_sec = L/1000;
				T.tv_usec = (L%1000)*1000;

				if(0 > select(M + 1, &R, &W, &E, &T)) {
					fprintf(stderr, "E: select(%i,,,,%li): %i: %s\n",
							M + 1, L, errno, strerror(errno));
					return EXIT_FAILURE;
				}else{
					fprintf(stderr,"Start!!! [%d]\n",U);
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
				//fprintf(stderr,"%s\n",resp->body.memory);
				curl_multi_remove_handle(cm, e);
				curl_easy_cleanup(e);
			}
			else {
				fprintf(stderr, "E: CURLMsg (%d)\n", msg->msg);
			}
#if 0
			if(C < CNT) {
				init(cm, C++,resp);
				U++; /* just to prevent it from remaining at 0 if there are more
								URLs to get */
			}
#endif
		}
	}

  curl_multi_cleanup(cm);
  curl_global_cleanup();

  return EXIT_SUCCESS;
}
