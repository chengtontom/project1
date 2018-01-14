// achieve_module.h
#ifndef __ACHIEVE_MODULE_H__
#define __ACHIEVE_MODULE_H__

#include <pthread.h>
#include "main.h"

#define HOST "money.cnn.com"    
#define PAGE "/"    
#define PORT 80    
#define USERAGENT "HTMLGET 1.0"    
#define XML_TEMP_FILE "./temp.xml"
#define _MAXLEN 1028
#define Uint32 unsigned int
#define UPDATE_TIME_HOUR 18
#define CHANGE_DATE_TO_INT(year, month, day) (1000000*(year) + 10000*(month) + 100*(day))
#define CHANGE_TIME_TO_INT(year, month, day, hour) (1000000*(year) + 10000*(month) + 100*(day) + (hour))
#define ACH_DB_TBL_NAME_MAX_LEN 64
#define BJ_TIMEZONE 8*3600 
#define ACH_MAX_RETRY_TIME 5

#define MAX_HOST_PAGE_NUM 1
#define MAX_TCP_BUFFER_LEN 128
#define TCP_PORT 7800
#define TCP_IP_ADDR "127.0.0.1"
#define TCP_CMD_ADD 0
#define TCP_CMD_DEL 1

#define DB_ACHIEVE_DATA_TEMP_LIST "DB_ACHIEVE_DATA_TEMP_LIST"

typedef struct _StrList {
    char *buffer;
    eStrType str_type;
    struct _StrList *next;
}StrList;
typedef struct _StrListHead {
    Uint32 list_len;
    StrList *next;
}StrListHead;

extern pthread_mutex_t achieve_mutex;
extern pthread_cond_t achieve_cond;
extern pthread_t achieve_pthread;
extern float achive_data_arr[ST_TYPE_MAX];

int create_tcp_socket();    
char *get_ip(char *host);    
char *build_get_query(char *host, char *page);    
void usage();    
Uint32 freadline(FILE *stream);
Uint32 save_xml_buffer(char *host, char *page);
void print_str_list(StrListHead* head);
Uint32 get_xml_line(StrListHead *str_list_head);
Uint32 add_str_list(StrListHead* head, char* p_str, Uint32 str_len, eStrType str_type);
void * achieve_thread_func(void *arg);
int is_new_update_time();
int send_data_to_tcp();
int list_float_arr_print_all(float* p_arr, int arr_lenth);
uint32_t ach_get_now();
int ach_db_init();
int ach_db_insert_entry(uint32_t type, float value);

#endif
