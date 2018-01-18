#include <stdio.h>    
#include <stdlib.h>    
#include <netdb.h>    
#include <string.h>  
#include <unistd.h>  
#include <sys/socket.h>    
#include <arpa/inet.h>  
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "achieve_module.h"
#include "main.h"
#include "db_lib.h"

#define ACH_DB_TBL_NAME_MAX_LEN DB_TBL_NAME_MAX_LEN

static char a_line[_MAXLEN];
float achive_data_arr[ST_TYPE_MAX];
float achive_tmp_data_arr[ST_TYPE_MAX];
int g_now_date;
int g_sockfd;


pthread_mutex_t achieve_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t achieve_cond = PTHREAD_COND_INITIALIZER;
pthread_t achieve_pthread;

char g_host_page[MAX_HOST_PAGE_NUM][2][30] = 
    {{"money.cnn.com","/"},
     {"www.livepriceofgold.com","/china-gold-price.html"},
     {"www.livepriceofgold.com","/silver-price/china.html"},
     {"money.cnn.com", "/data/commodities/index.html"},
     {"money.cnn.com", "/data/world_markets/americas/"},
     {"money.cnn.com", "/data/world_markets/europe/"},
     {"money.cnn.com", "/data/world_markets/asia/"}};

uint32_t g_host_type_range[MAX_HOST_PAGE_NUM][2] = 
    {{EX_US_EURO, EX_US_CHN},
     {AU_G_CHN, AU_G_CHN},
     {AG_G_CHN, AG_G_CHN},
     {BRENT_CRUDE_US, ME_CU_US},
     {MK_DOW, MK_SP},
     {MK_FTSE, MK_DAX},
     {MK_HS, MK_SSE}};

Uint32 save_xml_buffer(char *host, char *page)
{
    int sock;    
    int tmpres;    
    char *ip;    
    char *get;    
    char buf[BUFSIZ+1];    
    FILE *fp_xml_file;
    struct sockaddr_in *remote;    
    
    if((sock = create_tcp_socket()) < 0) {
        PRINT_LOG("sock is NULL\n");
        return RES_FAIL;
    }  
    if((ip = get_ip(host)) == NULL) {
        PRINT_LOG("IP is NULL\n");
        return RES_FAIL;
    }
    PRINT_LOG("IP is %s\n", ip);   
    remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));    
    remote->sin_family = AF_INET;    
    tmpres = inet_pton(AF_INET, ip, (void *)(&(remote->sin_addr.s_addr)));    
    if( tmpres < 0)
    {    
        PRINT_LOG("Can't set remote->sin_addr.s_addr");    
        return RES_FAIL;    
    }else if(tmpres == 0)    
    {    
        PRINT_LOG("%s is not a valid IP address\n", ip);    
        return RES_FAIL; 
    }    
    remote->sin_port = htons(PORT);    
     
    if(connect(sock, (struct sockaddr *)remote, sizeof(struct sockaddr))< 0){    
        PRINT_LOG("Could not connect");    
        return RES_FAIL; 
    }    
    get = build_get_query(host, page);    
    PRINT_LOG("Query is:\n<<START>>\n%s<<END>>\n", get);    
     
    //Send the query to the server    
    int sent = 0;    
    while(sent < strlen(get))    
    {    
        tmpres = send(sock, get+sent, strlen(get)-sent, 0);    
        if(tmpres == -1){    
            PRINT_LOG("Can't send query");    
            return RES_FAIL;
        }    
        sent += tmpres;    
    }    
    //now it is time to receive the page    
    memset(buf, 0, sizeof(buf));    
    int htmlstart = 0;    
    char * htmlcontent;   
    if((fp_xml_file = fopen(XML_TEMP_FILE, "w+")) == NULL)
    {
        PRINT_LOG("XMP TEMP FILE OPEN FAIL");  
        return RES_FAIL;
    }

    struct timeval timeout={5,0};//5s
    if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout)) == -1)
    {
        PRINT_LOG("%s : setsockopt fail\n", __FUNCTION__);
        return RES_FAIL;
    }
    
    while((tmpres = recv(sock, buf, BUFSIZ, 0)) > 0){    
        if(htmlstart == 0)    
        {    
            /* Under certain conditions this will not work. 
            * If the \r\n\r\n part is splitted into two messages 
            * it will fail to detect the beginning of HTML content 
            */    
            htmlcontent = strstr(buf, "\r\n\r\n");    
            if(htmlcontent != NULL){    
                htmlstart = 1;    
                htmlcontent += 4;    
            }    
        }else{    
            htmlcontent = buf;    
        }    
        if(htmlstart){    
            fprintf(fp_xml_file,"%s\n",htmlcontent);    
        }    
        memset(buf, 0, tmpres);    
    } 
    
    if(tmpres < 0)    
    {    
        PRINT_LOG("Error receiving data");
        return RES_FAIL;
    }    
    free(get);    
    free(remote);    
    free(ip);    
    close(sock);  
    fclose(fp_xml_file);
    return RES_OK;
}       
     
int create_tcp_socket()    
{    
    int sock;  
    if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){    
        PRINT_LOG("Can't create TCP socket");    
    }
    return sock;    
}    
     
     
char *get_ip(char *host)    
{    
    struct hostent *hent;    
    int iplen = 15; //XXX.XXX.XXX.XXX    
    char *ip = (char *)malloc(iplen+1);    
    memset(ip, 0, iplen+1);    
    if((hent = gethostbyname(host)) == NULL)    
    {    
        PRINT_LOG("Can't get IP");    
        return NULL;    
    }    
    if(inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, 16) ==NULL)    
    {    
        PRINT_LOG("Can't resolve host");    
        return NULL;    
    }
    return ip;    
}    
     
char *build_get_query(char *host, char *page)    
{    
    char *query;    
    char *getpage = page;    
    char *tpl = "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";    
    if(getpage[0] == '/'){    
        getpage = getpage + 1;    
        // fprintf(stderr,"Removing leading \"/\", converting %s to %s\n", page, getpage);    
    }    
    // -5 is to consider the %s %s %s in tpl and the ending \0    
    query = (char*)malloc(strlen(host)+strlen(getpage)+strlen(USERAGENT)+strlen(tpl)-5);    
    sprintf(query, tpl, getpage, host, USERAGENT);    
    return query;    
}  

Uint32 freadline(FILE *stream)
{    
    Uint32 count = 0;
    while((count < _MAXLEN) && ((a_line[count] = getc(stream)) != '\n')) {
        if(a_line[count] == EOF){
            a_line[count] = '\0';
            return count;
        }
        count ++;
    }
    
    a_line[count] = '\0';
    count++;
    return count;
}

eStrType parse_file_line(char* match_str)
{
    char *p_str;
    if(strstr(match_str,"span class=\"column quote-dollar\" title=\"Euro\"")!=NULL)
    {
        return EX_US_EURO;
    }
    else if(strstr(match_str,"span class=\"column quote-dollar\" title=\"British Pound\"")!=NULL)
    {
        return EX_US_UK;
    }
    else if(strstr(match_str,"span class=\"column quote-dollar\" title=\"Japanese Yen\"")!=NULL)
    {
        return EX_US_JPN;
    }
    else if(strstr(match_str,"span class=\"column quote-dollar\" title=\"Canadian Dollar\"")!=NULL)
    {
        return EX_US_CAN;
    }
    else if(strstr(match_str,"span class=\"column quote-dollar\" title=\"Chinese Yuan\"")!=NULL)
    {
        return EX_US_CHN;
    }
    else if((p_str = strstr(match_str,"Gold Rate per Gram in CNY"))!=NULL)
    {
        strcpy(a_line, p_str);
        return AU_G_CHN;
    }
    else if((p_str = strstr(match_str,"Silver Price per Gram in CNY"))!=NULL)
    {
        strcpy(a_line, p_str);
        return AG_G_CHN;
    }
    else if((p_str = strstr(match_str,"last_BZZH8"))!=NULL && 
                (p_str = strstr(p_str,">")) != NULL)
    {
        strcpy(a_line, p_str);
        return BRENT_CRUDE_US;
    }
    else if((p_str = strstr(match_str,"last_NGG18"))!=NULL && 
                (p_str = strstr(p_str,">")) != NULL)
    {
        strcpy(a_line, p_str);
        return NA_GAS_US;
    }
    else if((p_str = strstr(match_str,"last_GCG8"))!=NULL && 
                (p_str = strstr(p_str,">")) != NULL)
    {
        strcpy(a_line, p_str);
        return ME_AU_US;
    }
    else if((p_str = strstr(match_str,"last_SIH8"))!=NULL && 
                (p_str = strstr(p_str,">")) != NULL)
    {
        strcpy(a_line, p_str);
        return ME_AG_US;
    }
    else if((p_str = strstr(match_str,"last_PLJ8"))!=NULL && 
                (p_str = strstr(p_str,">")) != NULL)
    {
        strcpy(a_line, p_str);
        return ME_PT_US;
    }
    else if((p_str = strstr(match_str,"last_HGH8"))!=NULL && 
                (p_str = strstr(p_str,">")) != NULL)
    {
        strcpy(a_line, p_str);
        return ME_CU_US;
    }
    else if((p_str = strstr(match_str,"last_599362"))!=NULL && 
                (p_str = strstr(p_str,">")) != NULL)
    {
        strcpy(a_line, p_str);
        return MK_DOW;
    }
    else if((p_str = strstr(match_str,"last_575769"))!=NULL && 
                (p_str = strstr(p_str,">")) != NULL)
    {
        strcpy(a_line, p_str);
        return MK_SP;
    }
    else if((p_str = strstr(match_str,"last_572009"))!=NULL && 
                (p_str = strstr(p_str,">")) != NULL)
    {
        strcpy(a_line, p_str);
        return MK_FTSE;
    }
    else if((p_str = strstr(match_str,"last_585994"))!=NULL && 
                (p_str = strstr(p_str,">")) != NULL)
    {
        strcpy(a_line, p_str);
        return MK_CAC;
    }
    else if((p_str = strstr(match_str,"last_569857"))!=NULL && 
                (p_str = strstr(p_str,">")) != NULL)
    {
        strcpy(a_line, p_str);
        return MK_DAX;
    }
    else if((p_str = strstr(match_str,"last_568838"))!=NULL && 
                (p_str = strstr(p_str,">")) != NULL)
    {
        strcpy(a_line, p_str);
        return MK_HS;
    }
    else if((p_str = strstr(match_str,"last_576473"))!=NULL && 
                (p_str = strstr(p_str,">")) != NULL)
    {
        strcpy(a_line, p_str);
        return MK_NK;
    }
    else if((p_str = strstr(match_str,"last_586621"))!=NULL && 
                (p_str = strstr(p_str,">")) != NULL)
    {
        strcpy(a_line, p_str);
        return MK_SSE;
    }
    return ST_TYPE_MAX;
}

static float parse_str_float(char* buffer)
{
    float res = 0;
    sscanf(buffer, "%*[^0-9]%f", &res);
    return res;
    
}

static float parse_str_mk_dot_float(char* buffer)
{
    float res = 0;
    sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*1s%f", &res);
    return res;
}

static void parse_str_del_specific_char(char* buffer, char word)
{
    int i, j = 0;
    for(i = 0; i+j < strlen(buffer); i++) {
        if(buffer[i+j] == word) {
            j++;
        }
        buffer[i] = buffer[i+j];
    }
    buffer[i] = '\0';
}

static bool check_parse_temp_value(float temp_val)
{
    if(temp_val == 0)
    {
        return false;
    }
    return true;
}

int str_parse_to_data_arr()
{
    int rc = RES_OK;
    FILE *fp_xml_file;
    if((fp_xml_file = fopen(XML_TEMP_FILE, "r")) == NULL)
    {
        return RES_FAIL;
    }
    float temp_val = 0;
    eStrType st_type = ST_TYPE_MAX;
    Uint32 res_count = 0;
    char temp_str[1024] = {0};
    int next_line = 0;
    do {
        memset(a_line, 0, _MAXLEN);
        res_count = freadline(fp_xml_file);
        if(next_line)
        {
            next_line = 0;
            sprintf(temp_str, "%s%s", temp_str, a_line);
            strcpy(a_line, temp_str);
            memset(temp_str, 0, 1024);
        }
        st_type = parse_file_line(a_line);
        if(st_type == ST_TYPE_MAX)
        {
            continue;
        }
        else if(st_type == EX_US_EURO || 
            st_type == EX_US_UK || 
            st_type == EX_US_JPN || 
            st_type == EX_US_CAN || 
            st_type == EX_US_CHN ||
            st_type == AU_G_CHN ||
            st_type == AG_G_CHN)
        {
            PRINT_LOG("%s : %s\n", __FUNCTION__, a_line);
            temp_val = parse_str_float(a_line);
            if(check_parse_temp_value(temp_val))
            {
                achive_tmp_data_arr[st_type] = temp_val;
            }
        }
        else if(st_type == BRENT_CRUDE_US || st_type == NA_GAS_US || st_type == ME_AU_US ||
            st_type == ME_AG_US || st_type == ME_PT_US ||st_type == ME_CU_US ||
            st_type == MK_DOW || st_type == MK_SP ||st_type == MK_FTSE||
            st_type == MK_CAC || st_type == MK_DAX ||st_type == MK_HS||
            st_type == MK_NK || st_type == MK_SSE)
        {
            parse_str_del_specific_char(a_line, ',');
            PRINT_LOG("%s : %s\n", __FUNCTION__, a_line);
            temp_val = parse_str_float(a_line);
            if(check_parse_temp_value(temp_val))
            {
                achive_tmp_data_arr[st_type] = temp_val;
            }
        }
    }while(res_count > 0);
    fclose(fp_xml_file);
    return rc;
}

int ach_check_temp_arr_ready(int page_id)
{
    int i, res = RES_OK;
    for(i = g_host_type_range[page_id][0]; i <= g_host_type_range[page_id][1]; i++) {
        if(achive_tmp_data_arr[i] == 0) {
            res = RES_FAIL;
        }
        else {
            achive_data_arr[i] = achive_tmp_data_arr[i];
        }
    }
    return res;
}

int save_data_to_db()
{
    int i, res = RES_OK;
    for( i = 0; i < ST_TYPE_MAX; i++)
    {
        if(check_parse_temp_value(achive_data_arr[i])) {
            res = ach_db_insert_entry(i, achive_data_arr[i]);
        }
    }
    return res;
}

void achieve_main_run()
{ 
    time_t t_now = time(NULL);
    PRINT_LOG("%s start run : %s\n", __FUNCTION__, ctime(&t_now));
    
    int i = 0;
    int retry_time;
    for( i = 0; i < MAX_HOST_PAGE_NUM; i++)
    {
        retry_time = 0;
        if(retry_time < ACH_MAX_RETRY_TIME) {
            if(save_xml_buffer(g_host_page[i][0], g_host_page[i][1]) != RES_OK) {
                retry_time++;
                sleep(retry_time);
                printf("%s : host id[%d], retry times %d\n", __FUNCTION__, i, retry_time);
            }   
            str_parse_to_data_arr();
            if(ach_check_temp_arr_ready(i) != RES_OK) {
                retry_time++;
                sleep(retry_time);
                printf("%s : host id[%d], retry times %d\n", __FUNCTION__, i, retry_time);
            }
        }
        sleep(ACH_PER_HOST_INTERVAL);
    }
    

    save_data_to_db();
    send_data_to_tcp();
    memset(achive_tmp_data_arr, 0, sizeof(achive_tmp_data_arr));
}

int recover_list_from_db()
{
    int res = RES_OK;
    return res;
}

void * achieve_thread_func(void *arg)
{
    recover_list_from_db();
    while(1)
    {
        pthread_mutex_lock(&achieve_mutex);
        pthread_cond_wait(&achieve_cond, &achieve_mutex);
        PRINT_LOG("%s :func start\n", __FUNCTION__);
        achieve_main_run();
        pthread_mutex_unlock(&achieve_mutex);
    }
}

int tcp_socket_init()
{
    struct sockaddr_in host_addr;
    int on = 1;
    int res = 0;
    if((g_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        PRINT_ERROR_LOG("%s :socket fail \n",__FUNCTION__);
        return RES_FAIL;
    }
    
    memset(&host_addr, 0, sizeof(host_addr));
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(TCP_PORT);
    host_addr.sin_addr.s_addr = inet_addr(TCP_IP_ADDR);
    if(connect(g_sockfd, (struct sockaddr*)&host_addr, sizeof(host_addr))<0)
    {
        PRINT_ERROR_LOG("%s :connect fail \n",__FUNCTION__);
        close(g_sockfd);
        return RES_FAIL;  
    }  
    res = setsockopt (g_sockfd, IPPROTO_TCP, TCP_NODELAY, (void *) &on, sizeof (on)); //Solve the port takes up
    if(res < 0)
    {
        PRINT_ERROR_LOG("%s :setsockopt TCP_NODELAY fail \n",__FUNCTION__);
    }
    return RES_OK;
}

int tcp_send_packet(int cmd, int key, float value)
{
    signal(SIGPIPE, SIG_IGN);
    char buffer[MAX_TCP_BUFFER_LEN] = {0};
    sprintf(buffer, "%d:%d:%.3f#", cmd, key, value);
    if(g_sockfd == 0)
    {
        if(tcp_socket_init() != RES_OK)
        {
            PRINT_ERROR_LOG("%s :tcp_socket_init fail \n",__FUNCTION__);
            return RES_FAIL;
        }
    }
    if(send(g_sockfd, buffer, strlen(buffer), 0) < 0)
    {
        if(tcp_socket_init() != RES_OK)
        {
            PRINT_ERROR_LOG("%s :tcp_socket_init fail \n",__FUNCTION__);
            return RES_FAIL;
        }
        // retry one time
        if(send(g_sockfd, buffer, strlen(buffer), 0) < 0)
        {
            PRINT_ERROR_LOG("%s :retry send fail \n",__FUNCTION__);
            return RES_FAIL;
        }
    }
    printf("%s \n", buffer);
    return RES_OK;
}


int send_data_to_tcp()
{
    int i = 0, res = RES_OK;
    for(i = 0; i < ST_TYPE_MAX; i++)
    {
        res = tcp_send_packet(TCP_CMD_ADD, i, achive_data_arr[i]);
        CHECK_RES(res);
    }
    return res;
}

int list_float_arr_print_all(float* p_arr, int arr_lenth)
{
    int i;
    for(i = 0; i < arr_lenth; i++)
    {
        printf("%d : %f\n", i, p_arr[i]);
    }
    return RES_OK;
}

int ach_get_table_name(uint32_t type, char* tbl_name) {
    if(type == EX_US_EURO) {
        sprintf(tbl_name, "%s", "tbl_ex_us_euro");
    }
    else if(type == EX_US_UK) {
        sprintf(tbl_name, "%s", "tbl_ex_us_uk");
    }
    else if(type == EX_US_JPN) {
        sprintf(tbl_name, "%s", "tbl_ex_us_jpn");
    }
    else if(type == EX_US_CAN) {
        sprintf(tbl_name, "%s", "tbl_ex_us_can");
    }
    else if(type == EX_US_CHN) {
        sprintf(tbl_name, "%s", "tbl_ex_us_chn");
    }
    else if(type == AU_G_CHN) {
        sprintf(tbl_name, "%s", "tbl_au_g_chn");
    }
    else if(type == AG_G_CHN) {
        sprintf(tbl_name, "%s", "tbl_ag_g_chn");
    }
    else if(type == BRENT_CRUDE_US) {
        sprintf(tbl_name, "%s", "tbl_brent_crude_us");
    }
    else if(type == NA_GAS_US) {
        sprintf(tbl_name, "%s", "tbl_ns_gas_us");
    }
    else if(type == ME_AU_US) {
        sprintf(tbl_name, "%s", "tbl_me_au_us");
    }
    else if(type == ME_AG_US) {
        sprintf(tbl_name, "%s", "tbl_me_ag_us");
    }
    else if(type == ME_PT_US) {
        sprintf(tbl_name, "%s", "tbl_me_pt_us");
    }
    else if(type == ME_CU_US) {
        sprintf(tbl_name, "%s", "tbl_me_cu_us");
    }
    else if(type == MK_DOW) {
        sprintf(tbl_name, "%s", "tbl_mk_dow");
    }
    else if(type == MK_SP) {
        sprintf(tbl_name, "%s", "tbl_mk_sp");
    }
    else if(type == MK_FTSE) {
        sprintf(tbl_name, "%s", "tbl_mk_ftse");
    }
    else if(type == MK_CAC) {
        sprintf(tbl_name, "%s", "tbl_mk_cac");
    }
    else if(type == MK_DAX) {
        sprintf(tbl_name, "%s", "tbl_mk_dax");
    }
    else if(type == MK_HS) {
        sprintf(tbl_name, "%s", "tbl_mk_hs");
    }
    else if(type == MK_NK) {
        sprintf(tbl_name, "%s", "tbl_mk_nk");
    }
    else if(type == MK_SSE) {
        sprintf(tbl_name, "%s", "tbl_mk_sse");
    }
    return RES_OK;
}

int ach_db_init() 
{
    int i;
    char tbl_name[ACH_DB_TBL_NAME_MAX_LEN];
    if(g_test_mode) {
        db_create_datadase(CT_TEST_DB_NAME);
        strcpy(g_using_db_name ,CT_TEST_DB_NAME);
    }
    else {
        db_create_datadase(CT_DEFAULT_DB_NAME);
        strcpy(g_using_db_name ,CT_DEFAULT_DB_NAME);
    }

    for (i = 0; i < ST_TYPE_MAX; i++) {
        ach_get_table_name(i, tbl_name);
        db_create_table(tbl_name, DB_TBL_TYPE_TIME_VALUE);
    }
    return 0;
}

int ach_db_insert_entry(uint32_t type, float value)
{
    int rc;
    char tbl_name[ACH_DB_TBL_NAME_MAX_LEN];
    ach_get_table_name(type, tbl_name);
    db_date_time_value_t db_data;
    db_data.key.time = ach_get_now();
    db_data.value = value;
    rc = db_insert_entry(tbl_name, DB_TBL_TYPE_TIME_VALUE, &db_data);
    DB_CHECK_RC(rc);
    return RES_OK;
}

uint32_t ach_get_now()
{
    struct tm *time_now;
    time_t t_now;
    uint32_t get_now_time;
    t_now = time(NULL) + BJ_TIMEZONE;
    time_now = gmtime(&t_now);
    get_now_time = CHANGE_TIME_TO_INT(time_now->tm_year + 1900, time_now->tm_mon + 1, time_now->tm_mday, time_now->tm_hour);
    return get_now_time;
}