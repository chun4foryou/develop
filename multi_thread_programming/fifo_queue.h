typedef struct url_info{
	char url[128];
	int port;
	int idx;
}URL_INFO;


struct bucket
{
	void *data;
	struct bucket* next;
};


struct fifo_list
{
	struct bucket* head;     // 처음 데이터
	struct bucket* tail;     // 마지막 데이터 
	int bucket_data_size;    // fifo data 크기 
	unsigned int size;       // fifo_queue의 데이터 크기
	unsigned int queue_size; // 최대 queue 크기 
};

struct fifo_list* create_fifo_queue(unsigned int max_size,unsigned int data_size);
struct fifo_list* enqueue( struct fifo_list*,void* data);
struct fifo_list* deqeue( struct fifo_list* s , void *data);
struct fifo_list* fifo_free( struct fifo_list* );
struct fifo_list* remove_element( struct fifo_list*);

