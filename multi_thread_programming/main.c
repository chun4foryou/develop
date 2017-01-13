#include <stdio.h> 
#include <unistd.h> 
#include <pthread.h> 
#include  "./fifo_queue.h"

int ncount;    // 쓰레드간 공유되는 자원

struct my_list*  mt = NULL;

pthread_mutex_t  mutex = PTHREAD_MUTEX_INITIALIZER; // 쓰레드 초기화

// 쓰레드 함 수 1
void* enqueue(void *data)
{
	int i;
	int count=0;

	while(1){
		sleep(1);
		pthread_mutex_lock(&mutex); // 잠금을 생성한다.
		list_add_element(mt, count++);
		pthread_mutex_unlock(&mutex); // 잠금을 해제한다.
	}
}

// 쓰레드 함수 2
void* do_loop2(void *data)
{
	int i;

	// 잠금을 얻으려고 하지만 do_loop 에서 이미 잠금을 
	// 얻었음으로 잠금이 해제될때까지 기다린다.  
	while(1){
		if(mt->size >1){
			pthread_mutex_lock(&mutex); // 잠금을 생성한다.
			list_remove_element(mt);
			list_print(mt);
			pthread_mutex_unlock(&mutex); // 잠금을 해제한다.
		}
	}
}    

int main()
{
	int thr_id;
	pthread_t p_thread[2];
	int status;
	int a = 1;

	mt = list_new();

	printf("test\n");
	ncount = 0;
	thr_id = pthread_create(&p_thread[0], NULL, enqueue, (void *)&a);
	sleep(1);
	thr_id = pthread_create(&p_thread[1], NULL, do_loop2, (void *)&a);

	pthread_join(p_thread[0], (void *) &status);
	pthread_join(p_thread[1], (void *) &status);

	status = pthread_mutex_destroy(&mutex);
	printf("code  =  %d\n", status);
	printf("programing is end");
	return 0;
}
