#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdbool.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>   
#include <sys/time.h>   
#include <fcntl.h>   
#include <sys/ioctl.h>   
#include <unistd.h>   


#define RES_OK      0
#define RES_FAIL    1

extern FILE* log_fd;
extern FILE* error_log_fd;
extern uint32_t g_test_mode;

#define PRINT_LOG(format, ...) fprintf(log_fd, format, ##__VA_ARGS__)
#define PRINT_ERROR_LOG(format, ...) fprintf(error_log_fd, format, ##__VA_ARGS__)

#define CHECK_RES(res) if(res != RES_OK) {PRINT_ERROR_LOG("%s res FAIL\n", __FUNCTION__);return res;}
#define FD_LOG_PATH "./log.log"

typedef enum _eStrType{
    EX_US_EURO = 0,
    EX_US_UK,
    EX_US_JPN,
    EX_US_CAN,
    EX_US_CHN,
    BOND_TEN_US,// 10 year us 
    AU_G_CHN, // gold g
    AG_G_CHN, // silver g
    BRENT_CRUDE_US,
    NA_GAS_US,
    ME_AU_US, // gold               //10
    ME_AG_US, // silver             
    ME_PT_US, // platinum 
    ME_CU_US, // copper 
    MK_DOW,
    MK_SP,
    MK_FTSE,
    MK_CAC,
    MK_DAX,
    MK_HS,
    MK_NK,                          // 20
    MK_SSE,                         
    ST_TYPE_MAX
}eStrType;
#endif
