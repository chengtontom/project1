#include "db_lib.h"

char g_db_cmd[256];
char g_using_db_name[DB_TBL_NAME_MAX_LEN];
MYSQL *g_db_conn;

#define db_ct_default_connect() db_connect("root","ct",g_using_db_name)

int db_connect(char *usr, char *pwd, char *db_name) {
    if(g_db_conn) {
        return 0;
    }
  g_db_conn = mysql_init(NULL);
  if (g_db_conn == NULL) {
      CT_DB_LIB_DEBUG_LOG("Error %u: %s\n", mysql_errno(g_db_conn), mysql_error(g_db_conn));
      return -1;
  }
  if (mysql_real_connect(g_db_conn, "localhost", usr, 
          pwd, db_name, 0, NULL, 0) == NULL) {
      CT_DB_LIB_DEBUG_LOG("Error %u: %s\n", mysql_errno(g_db_conn), mysql_error(g_db_conn));
      return -1;
  }   
}

int db_disconnect() {
    mysql_close(g_db_conn);
    g_db_conn = NULL;
    return 0;
}

int db_run_cmd(MYSQL *conn,char *cmd)
{
    int rc = 0;
    CT_DB_LIB_DEBUG_LOG("%s\n", cmd);
    if((rc = mysql_query(conn, cmd)) < 0) {
        CT_DB_LIB_DEBUG_LOG("Error %u: %s\n", mysql_errno(g_db_conn), mysql_error(g_db_conn));
    }
    return rc;
}

int db_parse_one_result(MYSQL_ROW row, uint32_t type, void* data)
{
    if(type == DB_TBL_TYPE_TIME_VALUE) {
        db_date_time_value_t* p_data = (db_date_time_value_t*)data;
        sscanf(row[0], "%u" , &(p_data->key.time));
        sscanf(row[1], "%f" , &(p_data->value));
    }

    return 0;
}

int db_parse_result(MYSQL *conn, uint32_t type, void* data, uint32_t n)
{
    MYSQL_ROW row;
    uint32_t i;
    MYSQL_RES *result = mysql_store_result(conn);
    if(!result) {
        return -1;
    }
    while ((row = mysql_fetch_row(result)) && (i < n)) {
        db_parse_one_result(row, type, data);
        DB_DATA_NEXT(data, type);
    }
    mysql_free_result(result);
    return 0;
}

int db_create_datadase(char *name)
{
    int rc = 0;
    rc = db_connect("root", "ct", NULL);
    DB_CHECK_RC(rc);
    sprintf(g_db_cmd, "CREATE DATABASE %s", name);
    rc = db_run_cmd(g_db_conn, g_db_cmd);
    DB_CHECK_RC(rc);
    db_disconnect();
    return rc;
}

int db_create_table(char *name, uint32_t table_type)
{
    int rc = 0;
    rc = db_ct_default_connect();
    DB_CHECK_RC(rc);
    if(table_type == DB_TBL_TYPE_TIME_VALUE) {
        sprintf(g_db_cmd, "CREATE TABLE %s(time INT, value DECIMAL(12,3), PRIMARY KEY(time))", name);
    }
    else {
        return -1;
    }
    rc = db_run_cmd(g_db_conn, g_db_cmd);
    DB_CHECK_RC(rc);
    db_disconnect();
    return rc;
}

int db_insert_entry(char *name, uint32_t table_type, void* data)
{
    int rc = 0;
    rc = db_ct_default_connect();
    DB_CHECK_RC(rc);
    if(table_type == DB_TBL_TYPE_TIME_VALUE) {
        db_date_time_value_t* p_data = (db_date_time_value_t*)data;
        sprintf(g_db_cmd, "INSERT INTO %s VALUE('%u', '%f')", 
            name, p_data->key.time, p_data->value);
    }
    else {
        return -1;
    }
    rc = db_run_cmd(g_db_conn, g_db_cmd);
    DB_CHECK_RC(rc);
    db_disconnect();
}

int db_update_entry(char *name, uint32_t table_type, void* data)
{
    int rc = 0;
    rc = db_ct_default_connect();
    DB_CHECK_RC(rc);
    if(table_type == DB_TBL_TYPE_TIME_VALUE) {
        db_date_time_value_t* p_data = (db_date_time_value_t*)data;
        sprintf(g_db_cmd, "UPDATE %s SET value = '%f' WHERE time = '%u'" , 
            name, p_data->value, p_data->key.time);
    }
    else {
        return -1;
    }
    rc = db_run_cmd(g_db_conn, g_db_cmd);
    DB_CHECK_RC(rc);
    db_disconnect();
}

int db_delete_entry(char *name, uint32_t table_type, void* key)
{
    int rc = 0;
    rc = db_ct_default_connect();
    DB_CHECK_RC(rc);
    if(table_type == DB_TBL_TYPE_TIME_VALUE) {
        db_date_time_value_key_t* p_key = (db_date_time_value_key_t*)key;
        sprintf(g_db_cmd, "DELETE FROM %s WHERE time = '%u' ", 
            name, p_key->time);
    }
    else {
        return -1;
    }
    rc = db_run_cmd(g_db_conn, g_db_cmd);
    DB_CHECK_RC(rc);
    db_disconnect();
}

int db_get_key_entry(char *name, uint32_t table_type, void* key, void* data)
{
    int rc = 0;
    rc = db_ct_default_connect();
    DB_CHECK_RC(rc);
    if(table_type == DB_TBL_TYPE_TIME_VALUE) {
        db_date_time_value_key_t* p_key = (db_date_time_value_key_t*)key;
        sprintf(g_db_cmd, "SELECT * FROM %s WHERE time = '%u' ", 
            name, p_key->time);
    }
    else {
        return -1;
    }
    rc = db_run_cmd(g_db_conn, g_db_cmd);
    DB_CHECK_RC(rc);
    rc = db_parse_result(g_db_conn, table_type, data, 1);
    DB_CHECK_RC(rc);
    db_disconnect();
}

int db_get_last_one_entry(char *name, uint32_t table_type, void* data)
{
    int rc = 0;
    rc = db_ct_default_connect();
    DB_CHECK_RC(rc);
    if(table_type == DB_TBL_TYPE_TIME_VALUE) {
        sprintf(g_db_cmd, "SELECT * FROM %s ORDER BY time DESC LIMIT 1", name);
    }
    else {
        return -1;
    }
    rc = db_run_cmd(g_db_conn, g_db_cmd);
    DB_CHECK_RC(rc);
    rc = db_parse_result(g_db_conn, table_type, data, 1);
    DB_CHECK_RC(rc);
    db_disconnect();
}

