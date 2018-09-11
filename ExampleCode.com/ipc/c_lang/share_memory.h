#ifndef __SHARE_MEMORY_H__
#define __SHARE_MEMORY_H__

#define SHM_INFO_COUNT 30

typedef struct _shm_info{
	char         str_ip[40];
	unsigned int int_ip;
	unsigned int int_id;
}SHM_INFOS;

#endif//__SHARE_MEMORY_H__
