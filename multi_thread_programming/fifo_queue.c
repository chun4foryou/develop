/* This Queue implementation of singly linked list in C implements 3 
 * operations: add, remove and print elements in the list.  Well, actually, 
 * it implements 4 operations, lats one is list_free() but free() should not 
 * be considered the operation but  a mandatory practice like brushing 
 * teeth every morning, otherwise you will end up loosing some part of 
 * your body(the software) Its is the modified version of my singly linked 
 * list suggested by Ben from comp.lang.c . I was using one struct to do 
 * all the operations but Ben added a 2nd struct to make things easier and 
 * efficient.
 *
 * I was always using the strategy of searching through the list to find the
 *  end and then addd the value there. That way list_add() was O(n). Now I 
 * am keeping track of tail and always use  tail to add to the linked list, so 
 * the addition is always O(1), only at the cost of one assignment.
 *
 *
 * VERISON 0.5
 *
 */

#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  "./fifo_queue.h"

struct fifo_list* create_fifo_queue(unsigned int max_size,unsigned int data_size)
{
	struct fifo_list* p = malloc( 1 * sizeof(struct fifo_list));

	if( NULL == p )
	{
		fprintf(stderr, "LINE: %d, malloc() failed\n", __LINE__);
	}

	p->queue_size =  max_size;
	p->bucket_data_size = data_size;
	p->head = p->tail = NULL;

	return p;
}

/* Will always return the pointer to fifo_list */
// 동적데이터의 포인터를 저장한다.
struct fifo_list* enqueue(struct fifo_list* s, void *enq_data)
{
	struct bucket *p = NULL;
	void *new_data = NULL;

	if( NULL == s )
	{
		printf("Queue not initialized\n");
		free(p);
		return s;
	}

	if( s->size ==  s->queue_size ){
		fprintf(stderr, "IN %s, %s: FIFO Queue is FULL",__FILE__,__FUNCTION__);
		return s;
	}

	p = malloc( 1 * sizeof(struct bucket));
	new_data = malloc(1 * (s->bucket_data_size));
	memset(new_data,0,s->bucket_data_size);

	if( p ==  NULL || new_data == NULL)
	{
		fprintf(stderr, "IN %s, %s: malloc() failed\n", __FILE__,__FUNCTION__);
		return s; 
	}

	memcpy((void*)new_data, enq_data, s->bucket_data_size);
	p->data = new_data;
	p->next = NULL;

	if( NULL == s->head && NULL == s->tail )
	{
		/* printf("Empty list, adding p->num: %d\n\n", p->num);  */
		s->head = s->tail = p;
		s->size++;
		return s;
	}
	else if( NULL == s->head || NULL == s->tail )
	{
		fprintf(stderr, "There is something seriously wrong with your assignment of head/tail to the list\n");
		free(p);
		return NULL;
	}
	else
	{
		/* printf("List not empty, adding element to tail\n"); */
		s->tail->next = p;
		s->tail = p;
		s->size++;
	}

	return s;
}


/* This is a queue and it is FIFO, so we will always remove the first element */
struct fifo_list* deqeue( struct fifo_list* s , void *data)
{
	struct bucket* h = NULL;
	struct bucket* p = NULL;

	if( NULL == s )
	{
		printf("List is empty\n");
		return s;
	}
	else if( NULL == s->head && NULL == s->tail )
	{
		printf("Well, List is empty\n");
		return s;
	}
	else if( NULL == s->head || NULL == s->tail )
	{
		printf("There is something seriously wrong with your list\n");
		printf("One of the head/tail is empty while other is not \n");
		return s;
	}

	h = s->head;
	p = h->next;
	memcpy((void*)data, h->data, s->bucket_data_size);
	free(h->data);
	free(h);
	s->head = p;
	if( NULL == s->head )  s->tail = s->head;   /* The element tail was pointing to is free(), so we need an update */

	s->size--;
	return s;
}



/* This is a queue and it is FIFO, so we will always remove the first element */
struct fifo_list* remove_element( struct fifo_list* s )
{
	struct bucket* h = NULL;
	struct bucket* p = NULL;

	if( NULL == s )
	{
		printf("List is empty\n");
		return s;
	}
	else if( NULL == s->head && NULL == s->tail )
	{
		printf("Well, List is empty\n");
		return NULL;
	}
	else if( NULL == s->head || NULL == s->tail )
	{
		printf("There is something seriously wrong with your list\n");
		printf("One of the head/tail is empty while other is not \n");
		return s;
	}

	h = s->head;
	p = h->next;
	free(h->data);
	free(h);
	s->head = p;
	if( NULL == s->head )  s->tail = s->head;   /* The element tail was pointing to is free(), so we need an update */

	s->size--;
	return s;
}

struct fifo_list* fifo_free( struct fifo_list* s )
{
	while( s->head )
	{
		remove_element(s);
	}
	free(s);

	return NULL;
}


