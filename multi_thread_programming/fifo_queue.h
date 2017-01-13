struct my_struct
{
	int num;
	struct my_struct* next;
};


struct my_list
{
	struct my_struct* head;
	struct my_struct* tail;
	int size;
};


struct my_list* list_add_element( struct my_list*, const int);
struct my_list* list_remove_element( struct my_list*);


struct my_list* list_new(void);
struct my_list* list_free( struct my_list* );

void list_print( const struct my_list* );
void list_print_element(const struct my_struct* );

