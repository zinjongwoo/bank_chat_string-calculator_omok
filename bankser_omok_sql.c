/********************************************************************************************************/
#include "bank_head.h"
#define OPEN   0
#define SELECT 1
#define UPDATE 2
/********************************************************************************************************/

/********************************************************************************************************/
/* FUNCTION */
int     socket_set(int port);
int     socket_accept(int serv_sock);
char*   ltoa_ (long plLong, char* psAscii, int piLen);
long    atol_ (char* psAscii, int piLen);
void*   clnt_recv(void * arg);
void    send_message(char * message, int len, pthread_t tid);
void    error_handling(char * message);
void    log_set(char * message);
int     sql_handle(int handle, char * Account, char * Balance);
int     callback(void *data, int argc, char **argv, char **azColName);
/********************************************************************************************************/

/********************************************************************************************************/
/* VALUE */
int hb = 0;                 //hb회수
int clnt_number = 0;        //접속한 클라이언트 개수
int clnt_socks[MAX_ACCT];   //접속된 클라이언트 소켓 담을 구조체
pthread_mutex_t mutx;       //lock key
pthread_t thread[MAX_ACCT]; // recive thread
CUST_INFO cust[MAX_ACCT];   //고객정보
int custs_number = 0;       //메모리에 로드된 고객정보 개수
/********************************************************************************************************/

/********************************************************************************************************/
/* OMOK VALUE */
int  omok_sock[10] = {0};
int  table_sock[10][2] = {0,};
/********************************************************************************************************/

/********************************************************************************************************/
int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    pthread_t thread_temp;
    int current = 0;
    int j, rtn;

    sql_handle(OPEN, 0, 0);    

    if(argc!=2)
    {
        printf("사용법 : %s <port>\n", argv[0]);
        exit(1);
    }

    if(pthread_mutex_init(&mutx, NULL))
    {
        error_handling("mutex 초기화 에러남");
    }

    serv_sock=socket_set(atoi(argv[1])); //tcp socket생성

    while(1)
    {
        printf("accept wait....current link %d\n", clnt_number);
        clnt_sock = socket_accept(serv_sock);
        rtn = pthread_create(&thread_temp, NULL, clnt_recv, (void*)&clnt_sock);
        if(rtn != 0)
        {
            error_handling("thread create err");
        }
        pthread_mutex_lock(&mutx); //lock
        thread[clnt_number] = thread_temp;
        clnt_socks[clnt_number]=clnt_sock;
        clnt_number++;
        pthread_mutex_unlock(&mutx); //unlock
    }

    return 0;
}
/********************************************************************************************************/

/********************************************************************************************************/
int socket_set(int port)
{
    int serv_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    int clnt_addr_size;

    if((serv_sock=socket(PF_INET, SOCK_STREAM, 0)) < 0) //tcp socket생성
    {
        error_handling("socket creat err");
    }

    memset(&serv_addr, 0, sizeof(serv_addr)); //구조체 변수 초기화
    serv_addr.sin_family=AF_INET;  //ipv4
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);//모든주소네트워크 바이트 순서는 Big-Endian 방식host to netwok long
    serv_addr.sin_port=htons(port);
    
    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)    //해당소켓에 구조체변수에 정보를 구조체변수사이즈만큼 할당
    {
        error_handling("bind() error");
    }
    if(listen(serv_sock, 5) < 0)        //해당 소켓에 클라이언트 연결요청 기다림(동시 접속시 최대 대기인원 5)
    {
        error_handling("listen() error");
    }

    return serv_sock;
}
/********************************************************************************************************/

/********************************************************************************************************/
int socket_accept(int serv_sock)
{
    int clnt_sock;
    struct sockaddr_in clnt_addr;
    int clnt_addr_size = sizeof(clnt_addr);

    if((clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size)) < 0)//클라이언트 주소정보담을 구조체와 사이즈 및 소켓
    {
        printf("accept() error");
        return -1;
    }

    printf("새로운 연결, 클라이언트[%d] IP: %s \n", clnt_number, inet_ntoa(clnt_addr.sin_addr));
    return clnt_sock;
}
/********************************************************************************************************/
	
/********************************************************************************************************/
void * clnt_recv(void * arg)
{
    int clnt_sock=*((int*)arg);
    int str_len=0;
    int i = 0;
    char  data[BUF_SIZE];
    SEND_FMT  * message = (SEND_FMT*)data;
    int temp = 0;

    long cash = 0;
    long blc = 0;
    char ltoa[12];
    char blc_str[12];
    char cash_str[12];
    char acct_str[8];

    int acct;
  
    struct  pollfd  clnt_poll[1];
    clnt_poll[0].fd = clnt_sock;
    clnt_poll[0].events = POLLIN;
    int poll_rtn = 0;

    int table_temp;

    pthread_t tid = pthread_self();

    memset(data, 0x30, sizeof(SEND_FMT));
        
    while((str_len=read(clnt_sock, data, sizeof(SEND_FMT)))!=0)
    {
        printf("RECV FROM  CLIENT[SIZE:%d]:%s\n", sizeof(data), data);
        log_set(data);

        if(!memcmp(message->sTrCd, "LI", 2))
        {
            for(acct = 0; acct < MAX_ACCT; acct++)
            {
                if((memcmp(message->sAcct, cust[acct].sACCT, 8)==0) &&
                   (memcmp(message->sPw,   cust[acct].sPW, 8)  ==0))
                {
                    printf("SEND TO LI CLIENT[SIZE:%d]:%s\n", sizeof(data), data);
                    write(clnt_sock, data, str_len); //정상연결 잔고 업데이트 전송
                    log_set(data);
                    break;
                }
            }
            
            if(acct == MAX_ACCT)
            {
                memcpy(message->sTrCd, "ER", sizeof(message->sTrCd));
                printf("SEND TO ER CLIENT[SIZE:%d]:%s\n", sizeof(data), message);
                write(clnt_sock, message, str_len); 
                log_set(data);
                memset(data, 0x30, sizeof(SEND_FMT));
                continue;
            }
            break;
        }
    }

    memset(data, 0x30, sizeof(SEND_FMT));

    //int fcntl_flag = fcntl(clnt_sock, F_GETFL, 0);
    //fcntl(clnt_sock, F_SETFL, fcntl_flag | O_NONBLOCK);

    while(1)
    {
        poll_rtn = poll(clnt_poll, 1, 5000);
        if(poll_rtn == 0)
        {
            printf("TIME OUT 5 second...HB[%d]\n", hb);
            memset(data, 0x30, sizeof(SEND_FMT));

            memcpy(message->sTrCd, "HB", sizeof(message->sTrCd));
            printf("SEND TO HB CLIENT[SIZE:%d]:%s\n", sizeof(data), message);
            write(clnt_sock, message, sizeof(SEND_FMT)); 
            hb++;

        }
        memset(data, 0x30, sizeof(SEND_FMT));
        
        str_len = read(clnt_sock, data, sizeof(SEND_FMT));
        if(hb > 5)
        {
            printf("HB[%d:회]atm기 응답없음 경찰출동!!!\n", hb - 1);
            break;
        }

        if(str_len == 0)
        {
            printf("atm기 이상함 경찰출동!!!\n");
            break;
        }

        if(!memcmp(data, "LO", 2))
        {
            printf("RECV FROM  CLIENT[SIZE:%d]:%s\n", sizeof(data), data);
            log_set(data);
            printf("SEND TO LO CLIENT[SIZE:%d]:%s\n", sizeof(data), message);
            write(clnt_sock, message, sizeof(SEND_FMT));
            log_set(data);
            break;
        }

        else if(!memcmp(data, "JH", 2))
        {
            printf("RECV FROM  CLIENT[SIZE:%d]:%s\n", sizeof(data), data);
            log_set(data);
            memcpy(message->sBlc, cust[acct].sBLC, sizeof(cust[acct].sBLC));    
            printf("DBG memcpy:%s\n  cust:%s\n", message->sBlc, cust[acct].sBLC);
            printf("SEND TO JH CLIENT[SIZE:%d]:%s\n", sizeof(data), data);
            write(clnt_sock, data, sizeof(SEND_FMT));
            hb = 0;
        }
        
        else if(!memcmp(data, "PS", 2))
        {
            printf("RECV FROM  CLIENT[SIZE:%d]:%s\n", sizeof(data), data);
            log_set(data);
            blc = (long)atoi(cust[acct].sBLC);                                                    //잔고
            printf("계좌번호[%.*s]입금전:%ld\n", sizeof(cust[acct].sACCT), cust[acct].sACCT, blc);
            memcpy(cash_str, message->sCash, sizeof(cash_str));                                   //입금금액
            cash = atol_(cash_str, sizeof(cash_str));
            printf("계좌번호[%.*s]입금금액:%ld\n", sizeof(cust[acct].sACCT), cust[acct].sACCT, cash);
            blc += cash;                                                                          //잔고
            ltoa_(blc, ltoa, 12);
            printf("계좌번호[%.*s]입금후:%.*s\n", sizeof(cust[acct].sACCT), cust[acct].sACCT, sizeof(ltoa), ltoa);
            memcpy(message->sBlc, ltoa,  sizeof(message->sBlc));
            memset(message->sCash, 0x30, sizeof(message->sCash));
            printf("SEND TO PS CLIENT[SIZE:%d]:%s\n", sizeof(data), message);
            write(clnt_sock, data, sizeof(SEND_FMT)); 
            memcpy(cust[acct].sBLC, ltoa, sizeof(ltoa));                                   //입금금액
            printf("DBG->memcpy:%s\n", cust[acct].sBLC);
            hb = 0;
        }

        else if(!memcmp(data, "GE", 2))
        {
            printf("RECV FROM  CLIENT[SIZE:%d]:%s\n", sizeof(data), data);
            log_set(data);
            blc = (long)atoi(cust[acct].sBLC);                                                     //잔고
            printf("계좌번호[%.*s]출금전:%ld\n", sizeof(cust[acct].sACCT), cust[acct].sACCT, blc);
            memcpy(cash_str, message->sCash, sizeof(cash_str));                                    //출금금액
            cash = atol_(cash_str, sizeof(cash_str));
            printf("DBG memcpy e:%s, %ld\n", cash_str,cash);
            printf("계좌번호[%.*s]출금금액:%ld\n", sizeof(cust[acct].sACCT), cust[acct].sACCT, cash);

            if(blc < cash)
            {
                memcpy(message->sTrCd, "ER", sizeof(message->sTrCd));
                printf("SEND TO ER CLIENT[SIZE:%d]:%s\n", sizeof(data), message);
                write(clnt_sock, message, sizeof(SEND_FMT)); 
                printf("잔고[%ld] < 찾으려는돈[%ld], 날강도냐???\n", blc, cash);
            }

            else
            {
                blc -= cash;                                                                        //잔고
                ltoa_(blc, ltoa, sizeof(ltoa));
                memcpy(message->sBlc, ltoa,  sizeof(message->sBlc));
                memset(message->sCash, 0x30, sizeof(message->sCash));
       
                printf("계좌번호[%.*s]출금후:%.*s\n", sizeof(cust[acct].sACCT), cust[acct].sACCT, sizeof(ltoa), ltoa);
                printf("SEND TO GE CLIENT[SIZE:%d]:%s\n", sizeof(data), message);
                write(clnt_sock, message, sizeof(SEND_FMT));
                memcpy(cust[acct].sBLC, ltoa, sizeof(ltoa));                                   //입금금액
                printf("DBG->memcpy:%s\n", cust[acct].sBLC);
            }
            hb = 0;
        }

        else if(!memcmp(data, "HB", 2))
        {
            printf("RECV FROM  CLIENT[SIZE:%d]:%s\n", sizeof(data), data);
            printf("SEND TO HB CLIENT[SIZE:%d]:%s\n", sizeof(data), message);
            write(clnt_sock, message, sizeof(SEND_FMT)); 
            sleep(1);
            hb = 0;
        }

        else if(!memcmp(data, "ME", 2))
        {
            printf("RECV FROM  CLIENT[SIZE:%d]:%s\n", sizeof(data), data);
	    send_message((char *)message, str_len, tid); //자신을 뺀 모든 클라이언트에 전송
            hb = 0;
        }

	else if(!memcmp(data, "OM", 2))
	{
	    printf("RECV FROM  CLIENT[SIZE:%d]:%s\n", sizeof(data), data);
	    table_temp = atoi(message->sTable);
            for(i = 0; i < 10; i++)
            {
                if(i == table_temp)
                {
		    //if(table_sock[table_temp][omok_sock[table_temp]] > 0)
		    if(omok_sock[table_temp] > 1)
		    {
			memcpy(message->sTrCd, "OF", sizeof(message->sTrCd));
                        printf("SEND TO OE CLIENT[SIZE:%d]:%s\n", sizeof(data), message);
                        write(clnt_sock, message, sizeof(SEND_FMT)); 
			printf("table[%d] 자리에 만원임\n", table_temp);
                    }
		    else
		    {
		        table_sock[table_temp][omok_sock[table_temp]] = clnt_sock;
			printf("SEND TO OE CLIENT[SIZE:%d]:%s\n", sizeof(data), message);
			write(clnt_sock, message, sizeof(SEND_FMT));
			printf("table[%d] 자리에 손님들어옴\n", table_temp);
                    }
                    omok_sock[table_temp]++;
		    break;
		}
            }

	    if(i > 9)
	    {
                memcpy(message->sTrCd, "OE", sizeof(message->sTrCd));
                printf("SEND TO OE CLIENT[SIZE:%d]:%s\n", sizeof(data), message);
                write(clnt_sock, message, sizeof(SEND_FMT));
                printf("그런 table[%d] 없음\n", table_temp);
	    }

	    if(omok_sock[table_temp] == 1)
            {
                memcpy(message->sTrCd, "O1", sizeof(message->sTrCd));
                printf("SEND TO O1 CLIENT[SIZE:%d]:%s\n", sizeof(data), message);
                write(clnt_sock, message, sizeof(SEND_FMT)); 
            }

	    else if(omok_sock[table_temp] == 2)
            {
		memcpy(message->sTrCd, "O2", sizeof(message->sTrCd));
		printf("SEND TO O2 CLIENT[SIZE:%d]:%s\n", sizeof(data), message);
                write(clnt_sock, message, sizeof(SEND_FMT));

		memcpy(message->sTrCd, "O3", sizeof(message->sTrCd));
		for(i = 0; i < omok_sock[table_temp]; i++)
		{
                    printf("SEND TO O3 CLIENT[SIZE:%d]:%s\n", sizeof(data), message);
                    write(table_sock[table_temp][i], message, sizeof(SEND_FMT));
		}
	    }
            hb = 0;
	}

        else if((!memcmp(data, "OO", 2)) || (!memcmp(data, "OS", 2)))
	{
	    printf("RECV FROM  CLIENT[SIZE:%d]:%s\n", sizeof(data), data);
	    for(i = 0; i < omok_sock[table_temp]; i++)
            {
		if(table_sock[table_temp][i] != clnt_sock)
		{
                    write(table_sock[table_temp][i], message, sizeof(SEND_FMT)); 
		}
		if(!memcmp(data, "OO", 2))
		{
		    table_sock[table_temp][i] = 0;
		    if(i == omok_sock[table_temp] - 1)
		    {
			omok_sock[table_temp] = 0;
		    }
		}
            }
            hb = 0;
	}

        log_set(data);
    }

    for(i = 0; i < omok_sock[table_temp]; i++)
    {
        table_sock[table_temp][i] = 0;
    }

    omok_sock[table_temp] = 0;

    pthread_mutex_lock(&mutx);
    
    sql_handle(UPDATE, (char *)cust[acct].sACCT, (char *)cust[acct].sBLC); //SQL UPDATE
    
    for(i=0; i<clnt_number; i++)   // remove disconnected client
    {
        if(clnt_sock==clnt_socks[i])
        {
            temp = i;                          //tid index temp
            for( ; i<clnt_number-1;i++)
            {
                clnt_socks[i]=clnt_socks[i+1];
		thread[i]=thread[i+1];
            }
            break;
        }
    }

    clnt_number--;    
    pthread_mutex_unlock(&mutx);

    close(clnt_sock);
    pthread_detach(tid);
    pthread_exit(0);

    return 0;
}
/********************************************************************************************************/

/********************************************************************************************************/
void log_set(char * message)
{
    FILE * pfile;
    pfile = fopen("log.txt", "at");
    if(pfile == NULL)
    {
        printf("log file fopen err");
        return;
    }

    fputs(message, pfile); // 로그파일에 대화내용 기록
    fputc('\n', pfile);
    fclose(pfile);
    return;
}
/********************************************************************************************************/

/********************************************************************************************************/
void send_message(char * message, int len, pthread_t tid)   // send to all
{
	int i;
	//pthread_t tid = pthread_self();
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_number; i++)
	{
	    if(tid == thread[i])
            {
                continue;
	    }
	    write(clnt_socks[i], message, len);
            printf("SEND TO ME CLIENT[SIZE:%d]:%s\n", len, message);
	}

	pthread_mutex_unlock(&mutx);
}
/********************************************************************************************************/
 
/********************************************************************************************************/
void error_handling(char * message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
/********************************************************************************************************/

/********************************************************************************************************/
char*   ltoa_ (long plLong, char* psAscii, int piLen)
{
    memset(psAscii, 0x30, piLen);

    for ( piLen--; piLen >= 0 ; piLen-- )
    {
        *(psAscii+piLen) = plLong % 10 + '0' ;
        plLong /= 10 ;
    }

    return (psAscii);

}
/********************************************************************************************************/

/********************************************************************************************************/
long    atol_ (char* psAscii, int piLen)
{
    int     i;
    long    k;

    k = 0 ;

    for ( i = 0 ; i < piLen ; i++)
    {
        if ( psAscii[i] < '0' || psAscii[i] > '9' ) break;
        k = k * 10 + psAscii[i] - '0' ;
    }

    return (k);

} 
/********************************************************************************************************/

/********************************************************************************************************/
int sql_handle(int handle, char * pAccount, char * pBalance)
{
    sqlite3 *db;
    char *err_msg = 0;
    int rc;
    char * sql_open;
    char * sql_select;
    char sql_update[50] = {0};
    char * sql_print = "SELECT rowid, * FROM person;";

    char Account[8];
    char Balance[12];
    
    rc = sqlite3_open("account.db", &db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
   }

    if(handle == OPEN)
    {
        sql_open = "CREATE TABLE IF NOT EXISTS person(name TEXT, account TEXT PRIMARY KEY, passwd TEXT, balance TEXT);"
                    "INSERT OR IGNORE INTO person VALUES('Alice', '00000000', '00000000', '000000000000');"
                    "INSERT OR IGNORE INTO person VALUES('Bob',   '00000001', '00000001', '000001000000');"
                    "INSERT OR IGNORE INTO person VALUES('Tom',   '00000002', '00000002', '000002000000');"
                    "INSERT OR IGNORE INTO person VALUES('Jerry', '00000003', '00000003', '000003000000');";

        rc = sqlite3_exec(db, sql_open, 0, 0, &err_msg);

        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "Failed to execute SQL query: %s\n", err_msg);
            sqlite3_free(err_msg);
            sqlite3_close(db);
            return 1;
        }

        rc = sqlite3_exec(db, sql_print, callback, 0, &err_msg);
    }

    else if(handle == SELECT)
    {
        sprintf(sql_select, "SELECT * FROM person WHERE account='%s';", Account);
        
        rc = sqlite3_exec(db, sql_select, callback, 0, &err_msg);

        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "Failed to execute SQL query: %s\n", err_msg);
            sqlite3_free(err_msg);
            sqlite3_close(db);
            return 1;
        }
    }

    else if(handle == UPDATE)
    {
        memcpy(Account, pAccount, sizeof(Account));
        memcpy(Balance, pBalance, sizeof(Balance));
        printf("SQL DBG:%s\n%s\n", Account, Balance);
        
        sprintf(sql_update, "UPDATE person SET balance='%.*s' WHERE account='%.*s';", sizeof(Balance), Balance, sizeof(Account), Account);

        rc = sqlite3_exec(db, sql_update, 0, 0, &err_msg);

        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "Failed to execute SQL query: %s\n", err_msg);
            sqlite3_free(err_msg);
            sqlite3_close(db);
            return 1;
        }

        rc = sqlite3_exec(db, sql_print, callback, 0, &err_msg);
    }
    sqlite3_close(db);

    return 0;
}
/********************************************************************************************************/

/********************************************************************************************************/
int callback(void *data, int argc, char **argv, char **azColName)
{
    int i;
    for (i = 0; i < argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        if(!strcmp(azColName[i], "rowid"))
        {
            memcpy(cust[custs_number].sIDX, argv[i], sizeof(cust[custs_number].sIDX));
            printf("cust[custs_number].sIDX = %s\n", cust[custs_number].sIDX);
        }

        else if(!strcmp(azColName[i], "account"))
        {
            memcpy(cust[custs_number].sACCT, argv[i], sizeof(cust[custs_number].sACCT));
            printf("cust[custs_number].sACCT = %s\n", cust[custs_number].sACCT);
        }
        else if(!strcmp(azColName[i], "passwd"))
        {
            memcpy(cust[custs_number].sPW, argv[i], sizeof(cust[custs_number].sPW));
            printf("cust[custs_number].sPW = %s\n", cust[custs_number].sPW);
        }
        else if(!strcmp(azColName[i], "balance"))
        {
            memcpy(cust[custs_number].sBLC, argv[i], sizeof(cust[custs_number].sBLC));
            printf("cust[custs_number].sBLC = %s\n", cust[custs_number].sBLC);
        }
    }
    custs_number++;
    printf("\n");
    return 0;
}
/********************************************************************************************************/
