#ifndef _DB_LIB_H_
#define _DB_LIB_H_

#include <mysql.h>
#include <stdio.h>
#include <stdint.h>

#define CT_DEFAULT_DB_NAME "ct_db"
#define CT_TEST_DB_NAME "ct_test_db"
#define DB_TBL_NAME_MAX_LEN 64
#define CT_DB_LIB_DEBUG_LOG(format, ...)  printf(format, ##__VA_ARGS__);
#define DB_CHECK_RC(rc) \
    do {\
        if(rc < 0) {\
            printf("%s:%d rc < 0\n",__FUNCTION__, __LINE__);\
            return rc;\
        }\
    }while(0);

#define DB_DATA_NEXT(data, type) \
    do {\
        if(type == DB_TBL_TYPE_TIME_VALUE) {\
            (data) = ((db_date_time_value_key_t*)(data)) + 1;\
        }\
    }while(0);
        
enum {
    DB_TBL_TYPE_TIME_VALUE,
};

typedef struct {
    uint32_t time; //ymdh
}db_date_time_value_key_t;

typedef struct {
    db_date_time_value_key_t key; //ymdh
    float value;
}db_date_time_value_t;

extern char g_using_db_name[DB_TBL_NAME_MAX_LEN];

int db_connect(char *usr, char *pwd, char *db_name);
int db_disconnect();
int db_create_datadase(char *name);
int db_create_table(char *name, uint32_t table_type);
int db_insert_entry(char *name, uint32_t table_type, void* data);
int db_update_entry(char *name, uint32_t table_type, void* data);
int db_delete_entry(char *name, uint32_t table_type, void* key);
int db_get_key_entry(char *name, uint32_t table_type, void* key, void* data);
int db_get_last_one_entry(char *name, uint32_t table_type, void* data);
#endif
