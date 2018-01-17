#include <stdio.h>  
#include <sys/socket.h>    
#include <arpa/inet.h>    
#include <stdlib.h>    
#include <netdb.h>    
#include <string.h>  
#include <pthread.h> 
#include <sys/time.h> 
#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>

#include "main.h"
#include "achieve_module.h"
#include "db_lib.h" 

#define gettid() syscall(__NR_gettid)

struct itimerval timer_tick;

uint32_t main_thread_id ;
uint32_t g_test_mode;
FILE* log_fd;
FILE* error_log_fd;


void recv_sigalrm(int sig_id)
{
    if(main_thread_id != (uint32_t)gettid())
    {
        return;
    }
    PRINT_LOG("%s : %u start\n", __FUNCTION__, (uint32_t)gettid());
    pthread_mutex_lock(&achieve_mutex);
    pthread_cond_signal(&achieve_cond);  
    pthread_mutex_unlock(&achieve_mutex);   
}

void timer_init()
{
    // Initialize struct  
    memset(&timer_tick, 0, sizeof(timer_tick));  
  
    // Timeout to run function first time    
    timer_tick.it_value.tv_sec = 1;  // sec  
    timer_tick.it_value.tv_usec = 0; // micro sec.    
    // Interval time to run function  
    if(g_test_mode) {
        timer_tick.it_interval.tv_sec = 60;
    }
    else {
        timer_tick.it_interval.tv_sec = 300;
    }
    timer_tick.it_interval.tv_usec = 0;  
}

int test_func_print_all()
{
    printf("1 : print temp list data\n");
    return RES_OK;
}

char* readline(FILE* f)
{
    char* line = (char*) calloc(1, sizeof(char) );;
    char c;
    int len = 0;
   
    while ( (c = fgetc(f) ) != EOF && c != '\n')
    {
        line = (char*) realloc(line, sizeof(char) * (len + 2) );
        line[len++] = c;
        line[len] = '\0';
    }
    return line;
}

char* select_read()
{
    int nread;
    ioctl(0,FIONREAD,&nread);
    if(nread == 0)   
    {   
        return NULL;
    }
    char* buffer = (char*) malloc(nread);;   
    nread = read(0,buffer,nread);   
    buffer[nread] = '\0';   
    return buffer;
}

int test_func()
{
    char *temp_cmd = NULL;
    int func_id = 0, arg1 = 0, arg2 = 0, arg3 = 0;
    // temp_cmd = readline(stdin);
    temp_cmd = select_read();
    if(temp_cmd == NULL)
        return RES_OK;
    printf("%s\n", temp_cmd);
    sscanf(temp_cmd, "%d %d %d %d", &func_id, &arg1, &arg2, &arg3);
    printf("func %d arg %d %d %d\n", func_id, arg1, arg2, arg3);
    switch(func_id)
    {
        case(0):
        {
            test_func_print_all();
            break;
        }
        case(1):
        {
            list_float_arr_print_all(achive_data_arr, ST_TYPE_MAX);
            break;
        }
    }
    free(temp_cmd);
    return RES_OK;
}

int log_init()
{
    error_log_fd = stdout;
    if ((log_fd = fopen(FD_LOG_PATH, "a+")) == NULL)
    {
        printf("open log fail ...\n");fflush(stdout);
        log_fd = stdout;
    }
    else
    {
        setlinebuf(log_fd);
    }
    return 0;
}

int main(int argc, char **argv)    
{
    // block Signal tcp send fail quit
    signal(SIGPIPE, SIG_IGN);
    
    main_thread_id = (uint32_t)gettid();
    if((argc == 2) && (!strcmp(argv[1],"test"))) {
        g_test_mode = 1;
        printf("test mode start \n");
    }
    else {
        printf("%s : %u start\n", __FUNCTION__, (uint32_t)gettid());
    }
    log_init();
    timer_init();
    if(ach_db_init() != RES_OK)
    {
        printf("%s : db_init fail !\n",__FUNCTION__);
    }


    pthread_create(&achieve_pthread, NULL, achieve_thread_func, NULL);
    signal(SIGALRM, recv_sigalrm);
    setitimer(ITIMER_REAL, &timer_tick, NULL);

    // main thread wait forever
    printf("wait command \n");
    while(1) {
        test_func();
    }
    return 0;
}  