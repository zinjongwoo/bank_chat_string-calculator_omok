/********************************************************************************************************/
#include "bank_head.h"
/********************************************************************************************************/

/********************************************************************************************************/
/* DEFINE */
#define NAME_SIZE 20
#define ARRAY_SIZE 100
#define BOARD_SIZE 15
/********************************************************************************************************/
	
/********************************************************************************************************/
/* FUNCTION */
void * send_message(void * arg);
void * recv_message(void * arg);
void   error_handling(char * message);
int    socket_set_connect(char * ip, int port);
void   print_sigint(int  signo);
void   signal_handler();
/* Calculator FUNCTION */
void   bracketOperation();
int    fundamentalOperation(char  localValue[], int localCount);
void   printInputArray();
void   cal();

/* 5Mok FUNCTION */
void   init_board();
int    check_win(int row, int col);
void   omok(int sock);
void   init_curses();
int    get_move(char player, int * prow, int * pcol);
void   draw_board();
/********************************************************************************************************/

/********************************************************************************************************/
/* Login VALUE */
char name[NAME_SIZE]="[DEFAULT]";
char act[9];
char pw[9];
struct timespec ltTimeSpec;
struct tm*      ltpTm;
int login = 0;
/********************************************************************************************************/

/********************************************************************************************************/
/*Calculator VALUE */
//INFIX를 POSTFIX로 표기하여 계산하는방식으로 구현
//중위표기법(infix)->후위표기법(postfix)

//'0', '9', '+', '-', '*', '/', '(', ')' 확인핸들러
int handle;

//괄호 확인하는 핸들러
int guarhandle;

//곱하기나누기 확인하는 핸들러
int divmulhandle;

// 입력 값의 최대 사이즈 지정
//#define ARRAY_SIZE 200

// 입력 받은 수식이 저장되는 배열
char input[ARRAY_SIZE] = {0};

// 입력 받은 수식의 길이를 보관하는 변수
int inputLength = 0;
/********************************************************************************************************/

/********************************************************************************************************/
/*Omok VALUE */
//오목판
char board[BOARD_SIZE][BOARD_SIZE];

//입장할 테이블
char table[3];

//사용할 돌
char dol;

//게임 시작체크 0테이블 입장 1 시작
char start =  -1;

//승부 체크 w/l  win/loss
char win_loss = 0;

//우선순위 체크 y/n  y내차례 n상대차례
char turn = 0;
/********************************************************************************************************/

/********************************************************************************************************/
int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void * thread_result;
	if(argc!=4) 
	{
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		printf("임시계정ACCOUNT ID : 00000000\n PW : 00000000\n", argv[0]);
		exit(1);
	}
        
        sprintf(name, "[%s]", argv[3]);
	signal_handler();
	
        sock = socket_set_connect(argv[1], atoi(argv[2]));
	
	pthread_create(&snd_thread, NULL, send_message, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_message, (void*)&sock);
	pthread_join(snd_thread, &thread_result);
	pthread_join(rcv_thread, &thread_result);
	return 0;
}
/********************************************************************************************************/

/********************************************************************************************************/
int socket_set_connect(char * ip, int port)
{
    int sock;
    struct sockaddr_in serv_addr;

    sock=socket(PF_INET, SOCK_STREAM, 0);
    if(sock==-1)
    {
        error_handling("socket() errer");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(ip);
    serv_addr.sin_port=htons(port);

    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
    {
        error_handling("connect() error");
    }
    return sock;
}
/********************************************************************************************************/

/********************************************************************************************************/
void * send_message(void * arg)   // send thread main
{
    int sock=*((int*)arg);
    char   message[100];
    char   cash[13];
    char   sdate[8];
    char   stime[8];
    char   check = 0;
    int    row, col, i;
    SEND_FMT * s_message = (SEND_FMT *)message;
    while(1)
    {
        memset(message, 0x30, sizeof(SEND_FMT));
        if(login == 0)
        {
            printf("input acct:");
            fgets(act, 9 , stdin);
            while(getchar()!='\n');
            printf("input pw:");
            fgets(pw, 9 , stdin);
            while(getchar()!='\n');
            printf("이름:%s님\n계좌:%s\n비번:%s\n", name, act, pw);

            memcpy(s_message->sTrCd, "LI", 2);
            memcpy(s_message->sAcct, act, 8);
            memcpy(s_message->sPw, pw, 8);            

            clock_gettime(CLOCK_REALTIME, &ltTimeSpec);
            ltpTm = localtime(&ltTimeSpec.tv_sec);
            sprintf(sdate, "%04d%02d%02d", ltpTm->tm_year+1900, ltpTm->tm_mon+1, ltpTm->tm_mday);
            sprintf(stime, "%02d%02d%02d",     ltpTm->tm_hour, ltpTm->tm_min, ltpTm->tm_sec);
            memcpy(s_message->sDate, sdate, sizeof(s_message->sDate));
            memcpy(s_message->sTime, stime, sizeof(s_message->sTime));
            memset(s_message->sFill, 'E', sizeof(s_message->sFill));
            printf("SEND LI TO BANK[SIZE:%d]:%s\n", sizeof(message), message);
            
            write(sock, message, sizeof(SEND_FMT));
            sleep(1);
        }

        else if(login == 1) 
        {          
            while(1) 
            {
                memset(message, 0x30, BUF_SIZE);
                printf("\n종료:Q, 조회:J, 채팅:M, 입금:P, 출금:G, 계산기:C, 오목게임:5:");
                check = getchar();
                while(getchar()!='\n');

                memcpy(s_message->sAcct, act, 8);
                memcpy(s_message->sPw, pw, 8);

                clock_gettime(CLOCK_REALTIME, &ltTimeSpec);
                ltpTm = localtime(&ltTimeSpec.tv_sec);
                sprintf(sdate, "%04d%02d%02d", ltpTm->tm_year+1900, ltpTm->tm_mon+1, ltpTm->tm_mday);
                sprintf(stime, "%02d%02d%02d",     ltpTm->tm_hour, ltpTm->tm_min, ltpTm->tm_sec);
                memcpy(s_message->sDate, sdate, sizeof(s_message->sDate));
                memcpy(s_message->sTime, stime, sizeof(s_message->sTime));
                memset(s_message->sFill, 'E',   sizeof(s_message->sFill));

                if(check == 'q'||check == 'Q') 
                {
                    memcpy(s_message->sTrCd, "LO", 2);

                    printf("\nSEND LO TO BANK[SIZE:%d]:%s\n", sizeof(message), message);
            
                    write(sock, message, sizeof(SEND_FMT));
                }
            
                else if(check == 'j'||check == 'J') 
                {
                    memcpy(s_message->sTrCd, "JH", 2);

                    printf("\nSEND JH TO BANK[SIZE:%d]:%s\n", sizeof(message), message);
            
                    write(sock, message, sizeof(SEND_FMT));
                }

                else if(check == 'p'||check == 'P') 
                {
                    printf("\ninput cash 12byte:");

                    fgets(cash, 13, stdin);

                    while(getchar()!='\n');
                    memcpy(s_message->sTrCd, "PS", 2);
                    memcpy(s_message->sCash, cash, 12);

                    printf("\nSEND PS TO BANK[SIZE:%d]:%s\n", sizeof(message), message);
            
                    write(sock, message, sizeof(SEND_FMT));
                }
                else if(check == 'g'||check == 'G') 
                {
                    printf("\ninput cash 12byte:");

                    fgets(cash, 13, stdin);

                    while(getchar()!='\n');
                    memcpy(s_message->sTrCd, "GE", 2);
                    memcpy(s_message->sCash, cash, 12);
                    printf("\nSEND GE TO BANK[SIZE:%d]:%s\n", sizeof(message), message);
                    write(sock, message, sizeof(SEND_FMT));
                }
                else if(check == 'm'||check == 'M') 
                {
                    memcpy(message, "ME:", 3);
		    printf("Input Message: ");
                    fgets(message + 3, BUF_SIZE - 3, stdin);
                    write(sock, message, sizeof(SEND_FMT));
                }
		else if(check == 'c'||check == 'C')
	        {
                    cal();
		}
                else if(check == '5')
		{
		    memcpy(message, "OM", 2);
                    printf("테이블 선택[00~09]:");
		    fgets(table, 3, stdin);
		    while(getchar()!='\n');
                    printf("사용할 돌 선택[0~Z]:");
		    dol = (char)getchar();
		    while(getchar()!='\n');
		    memcpy(s_message->sTable, table, 2);
		    memcpy(s_message->sDol,   &dol, 1);
                    write(sock, message, sizeof(SEND_FMT));
		    while(1)
		    {
			sleep(3);
		        if(start == 0)
	                {
			    printf("\n상대 기다리는중...\n");
			    continue;
			}
			else if(start == 1)
			{
			    memcpy(message, "OS:", 3);
			    init_board();
			    init_curses();
                            draw_board();

			    win_loss = 0;
			    while(1)
		            {
				sleep(1);
				if(win_loss == 'l')
				{
				    for(i = 5; i > 0; i--)
			            {
				        mvprintw(BOARD_SIZE+1, 0, "you loss!, [%d]초뒤 게임종료", i);
				        refresh();
					sleep(1);
				    }
				    endwin();
				    break;
				}
				if(turn == 'y')
				{
                                    mvprintw(BOARD_SIZE+3, 0, "사용법:방향키로 조절, enter키로 확정");
				    if (get_move(dol, &row, &col))
			            {
                                        mvprintw(BOARD_SIZE+2, 0, "내가 놓은 자리[row:%d][col:%d]", row, col);
				        turn = 'n';

					sprintf(s_message->sRow, "%d", row);
                                        sprintf(s_message->sCol, "%d", col);

				        write(sock, message, sizeof(SEND_FMT));
					refresh();
				        check = check_win(row, col);
				        if(check == 0)
			                {
					    for(i = 5; i > 0; i--)
				            {
					        mvprintw(BOARD_SIZE+1, 0, "you wins!, [%d]초뒤 게임종료", i);
					        refresh();
						sleep(1);
					    }
                                            memcpy(message, "OO", 2);
                                            write(sock, message, sizeof(SEND_FMT));
					    endwin();
				            break;
					}
			 	    }
			        }
			    }
                        }
			start = -1;
			break;
		    }
		}
            }
        }
    }
}
/********************************************************************************************************/

/********************************************************************************************************/
void * recv_message(void * arg)   // read thread main
{
    int sock=*((int*)arg);
    int str_len = 0;
    int check = 0;
    char   message[BUF_SIZE];
    SEND_FMT * s_message = (SEND_FMT *)message;
    memset(message, 0x00, sizeof(SEND_FMT));

    int row, col;

    while(1)
    {
        if(login == 0)
        {
            memset(message, 0x00, sizeof(SEND_FMT));
            read(sock, message, sizeof(SEND_FMT));    
            if(!memcmp(s_message->sTrCd, "LI", 2))
            {
                printf("login success\n");
                printf("\nRECV FROM BANK[SIZE:%d]:%s\n", sizeof(message), message);
                login = 1 ;
            }
            else if(!memcmp(s_message->sTrCd, "ER", 2))
            {
                check++;
                printf("\n%d회 오류\n", check);
                printf("\nRECV FROM BANK[SIZE:%d]:%s\n", sizeof(message), message);
                sleep(1);
                if(check == 5)
                {
                    printf("\n%d회 오류, 은행방문하세요!!!\n", check);
                    close(sock);
                    exit(0);
                }
            }
        }

        if(login == 1)
        {
            while(1)
            {
                memset(message, 0x00, sizeof(SEND_FMT));
                //read(sock, message, sizeof(SEND_FMT));
                str_len=read(sock, message, sizeof(SEND_FMT));
                if(str_len <= 0) 
                {
                    printf("\n은행에 강도가 쳐들어옴!!!\n");
                    close(sock);
                    exit(0);
                }

                message[str_len]=0;
                if(!memcmp(s_message->sTrCd, "ER", 2))
                {
                    printf("\nRECV FROM BANK[SIZE:%d]:%s\n", sizeof(message), message);
                }

                else if(!memcmp(s_message->sTrCd, "LO", 2))
                {
                    printf("\nRECV FROM BANK[SIZE:%d]:%s\n", sizeof(message), message);
                    close(sock);
                    exit(0);
                }

                else if(!memcmp(s_message->sTrCd, "JH", 2))
                {
                    printf("\nRECV FROM BANK[SIZE:%d]:%s\n", sizeof(message), message);
                    printf("\n현재 잔액:%.*s\n", sizeof(s_message->sBlc), s_message->sBlc);
                }

                else if(!memcmp(s_message->sTrCd, "PS", 2))
                {
                    printf("\nRECV FROM BANK[SIZE:%d]:%s\n", sizeof(message), message);
                    printf("\n입금후 잔액:%.*s\n", sizeof(s_message->sBlc), s_message->sBlc);
                }

                else if(!memcmp(s_message->sTrCd, "GE", 2))
                {
                    printf("\nRECV FROM BANK[SIZE:%d]:%s\n", sizeof(message), message);
                    printf("\n출금후 잔액:%.*s\n", sizeof(s_message->sBlc), s_message->sBlc);
                }

                else if(!memcmp(s_message->sTrCd, "HB", 2))
                {
                    write(sock, message, sizeof(SEND_FMT));
                }

		else if(!memcmp(s_message->sTrCd, "OM", 2))
                {
		    start = 0;
                }

		else if(!memcmp(s_message->sTrCd, "O1", 2))
                {
                    printf("\n먼저 입장 선수...\n");
                    turn = 'y';
                }

		else if(!memcmp(s_message->sTrCd, "O2", 2))
                {
                    printf("\n나중에 입장 후수...\n");
                    turn = 'n';
                }

		else if(!memcmp(s_message->sTrCd, "O3", 2))
                {
                    printf("\n게임 스타트...\n");
                    start = 1;
                }

		else if(!memcmp(s_message->sTrCd, "OE", 2))
		{    
		    start = -1;
		    printf("\n해당테이블 없음\n");
		}

		else if(!memcmp(s_message->sTrCd, "OF", 2))
		{    
		    start = -1;
		    printf("\n해당테이블 만원임\n");
		}

		else if(!memcmp(s_message->sTrCd, "OS", 2))
		{
		    row = atoi(s_message->sRow);
                    col = atoi(s_message->sCol);
                    board[row][col] = *(s_message->sDol);
		    mvprintw(row, col*2, "%c", board[row][col]);
                    mvprintw(BOARD_SIZE+2, 0, "상대가 놓은 자리[row:%d][col:%d] ", row, col);
                    refresh();

		    check = check_win(row, col);
                    if(check == 0)
                    {
			win_loss = 'l';
                    }
		    turn = 'y';
		}

		else if(!memcmp(s_message->sTrCd, "OO", 2))
		{
		    printf("\n상대가 나감\n");
		    start = -1;
		}

                else
                {
                    printf("\nRECV FROM BANK[SIZE:%d]:%s\n", sizeof(message), message);
                }
            }  
        }
    }
}
/********************************************************************************************************/

/********************************************************************************************************/
void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
/********************************************************************************************************/

/********************************************************************************************************/
void printInputArray()
{
    int i;
    for (i =0; i < inputLength; i++)
    {
        printf("%c", input[i]);
    }
    printf("\n");
}
/********************************************************************************************************/

/********************************************************************************************************/
// 사칙 연산의 결과를 int형으로 반환하는 함수이다. 파라미터로 사칙연산 수식과, 그 수식의 길이를 받는다.
int fundamentalOperation(char localValue[ARRAY_SIZE], int localCount)
{
    int  INT[ARRAY_SIZE] = {0};           // 수를 저장하는 공간
    int  INTidx = 0;                         // v배열에 들어가는 갯수를 계산하기 위한 변수
    char SYMBOL[ARRAY_SIZE] = {0};      // 수식기호를 저장하는 공간
    int  SYMBOLidx = 0;                     // con배열에 들어가는 갯수를 계산하기 위한 변수
    int  INTbuf = 0;                         // 문자열 배열로 깨져있는 수를 조합하는 공간(이후 저장하는 공간으로 이동)
    int  i, j, tmp, result;
    divmulhandle = 0;       // 곱하기나 나누기를 가지고 있는지 확인하기 위한 변수
    for (j = 0; j < localCount; j++)
    {
        if ('0' <= localValue[j] && localValue[j] <= '9')       // 숫자일 경우 sqr에 저장한다.
        {
            INTbuf *= 10;                                               // 기존에 들어있던 sqr 값은 10을 곱하여 한칸 밀어주고 값이 없으면 그냥0이다.
            INTbuf += localValue[j] - '0';                            // 새로운 숫자를 더해준다. 캐릭터형이기 때문에 '0'을 빼서 원래 값을 구한다.
        }

        else if (localValue[j] == '-' || localValue[j] == '+')   // - + 연산자일 경우
        {
            INT[INTidx] = INTbuf;      // 기존에 구한 숫자를 v배열에 추가하고,
            INTbuf = 0;            // sqr을 초기화한다.
            INTidx++;             // 배열의 크기를 계산하기 위한 vIdx는 1을 더해준다.

            if (divmulhandle == 1)
            { // 곱하기나 나누기를 가지고 있음을 확인한 상태라면
                tmp = 0;
                if (SYMBOL[SYMBOLidx-1] == '*')
                {
                    printf("곱셈 계산");
                    tmp = INT[INTidx-2] * INT[INTidx-1];
                }

                else if (SYMBOL[SYMBOLidx-1] == '/')
                {
                    printf("나눗셈 계산");
                    tmp = INT[INTidx-2] / INT[INTidx-1];
                }
                printf(" %d\n", tmp);
                divmulhandle = 0;
                // 계산한 값들을 배열에서 제외해준다.
                SYMBOL[SYMBOLidx-1] = 0;
                SYMBOLidx--;

                INT[INTidx-1] = 0;
                INT[INTidx-2] = 0;
                INTidx -= 2;

                INT[INTidx] = tmp;
                INTidx ++;
            }

            SYMBOL[SYMBOLidx] = localValue[j]; // 수식도 배열에 추가한다.
            printf("con[%d] : %c\n", SYMBOLidx, SYMBOL[SYMBOLidx]);
            SYMBOLidx++;

        }
        else if (localValue[j] == '*' || localValue[j] == '/')
        { // * / 연산자일 경우,
            INT[INTidx] = INTbuf;      // 기존에 구한 숫자를 v배열에 추가하고,
            INTbuf = 0;            // sqr을 초기화한다.
            INTidx++;             // 배열의 크기를 계산하기 위한 vIdx는 1을 더해준다.

            if (divmulhandle == 1)
            { // 기존에 곱셈이나 나눗셈 수식을 받아둔게 있다면
                tmp = 0;
                if (SYMBOL[SYMBOLidx-1] == '*')
                {
                    printf("곱셈 계산");
                    tmp = INT[INTidx-2] * INT[INTidx-1];
                }
                else if (SYMBOL[SYMBOLidx-1] == '/')
                {
                    printf("나눗셈 계산");
                    tmp = SYMBOL[SYMBOLidx-2] / SYMBOL[SYMBOLidx-1];
                }
                printf(" %d\n", tmp);
                SYMBOL[SYMBOLidx-1] = 0;
                SYMBOLidx--;

                INT[INTidx-1] = 0;
                INT[INTidx-2] = 0;
                INTidx -= 2;

                INT[INTidx] = tmp;
                INTidx ++;
            }

            SYMBOL[SYMBOLidx] = localValue[j]; // 수식도 배열에 추가한다.
            SYMBOLidx++;

            divmulhandle = 1; // 곱하기나 나누기를 입력받았음을 표현해둔다.
        }

        if (j == localCount-1)
        { // 수식을 다 입력받은 상태로, 곱하기나 나누기를 입력 받은 상태라면
            INT[INTidx] = INTbuf;      // 기존에 구한 숫자를 v배열에 추가하고,
            INTbuf = 0;            // sqr을 초기화한다.
            INTidx++;             // 배열의 크기를 계산하기 위한 vIdx는 1을 더해준다.

            if (divmulhandle == 1)
            {
                tmp = 0;
                if (SYMBOL[SYMBOLidx-1] == '*')
                {
                    printf("곱셈 계산");
                    tmp = INT[INTidx-2] * INT[INTidx-1];
                }
                else if (SYMBOL[SYMBOLidx-1] == '/')
                {
                    printf("나눗셈 계산 %d, %d", INT[INTidx-2], INT[INTidx-1]);
                    tmp = INT[INTidx-2] / INT[INTidx-1];
                }
                printf(" %d\n", tmp);
                divmulhandle = 0;
                // 계산한 값들을 배열에서 제외해준다.
                SYMBOL[SYMBOLidx-1] = 0;
                SYMBOLidx--;

                INT[INTidx-1] = 0;
                INT[INTidx-2] = 0;
                INTidx -= 2;

                INT[INTidx] = tmp;
                INTidx ++;
            }
        }
    }

    result = INT[0];
    for (i =0; i < SYMBOLidx; i++)
    {
        if (SYMBOL[i] == '+')
        {
            result += INT[i+1];
        }
        else if (SYMBOL[i] == '-')
        {
            result -= INT[i+1];
        }
    }
    printf("result : %d", result);
    return result;
}
/********************************************************************************************************/

/********************************************************************************************************/
// 전역변수 input에 있는 수식의 가장 먼저 만나고 가장 깊은 괄호 하나를 풀어내고,
// input에 수식을 재정리하는 함수이다.
void bracketOperation()
{
    // 가장 깊은 괄호문에 있는 수식을 찾는다.
    // 괄호가 열릴 때 마다 +1, 닫힐 때 마다 -1
    // 더 이상 내부에 괄호가 없을 때(닫는 괄호를 만날 때) 계산 시작
    // 1순위 -> 괄호 내부에 있는 수식
    // 2순위 -> 곱셈/나눗셈
    // 3순위 -> 덧셈/뺄셈
    int newForm[ARRAY_SIZE] = {0};
    int newFormIdx = 0;
    char localValue[ARRAY_SIZE] = {0};
    int localCount = 0;
    int localStart = 0;
    int localEnd = 0;
    int localResult = 0;
    int i, j, k;
    for (i = 0; i < inputLength; i++)
    {
        if (input[i] == '(')
        {
            // deepLevel++;
            memset(&localValue[0], 0, sizeof(localValue));
            localCount = 0;
            localStart = i;
        }

        else if (input[i] == ')')
        {
            // deepLevel--;
            localResult = fundamentalOperation(localValue, localCount);
            printf("cal : %d\n", localResult);
            localEnd = i;
            for (j = 0; j < ARRAY_SIZE; j++)
            {
                if (j < localStart)
                {
                    newForm[newFormIdx] = input[j];
                    newFormIdx++;
                }
                else if (j == localStart)
                {
                    char localResultArr[11] = {0};
                    sprintf(localResultArr, "%d", localResult);
                    for (k = 0; k < 11; k++)
                    {
                        if (localResultArr[k] != 0)
                        {
                            newForm[newFormIdx] = localResultArr[k];
                            newFormIdx++;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                else if (j > localEnd && input[j] != 0)
                {
                    newForm[newFormIdx] = input[j];
                    newFormIdx++;
                }
            }
            break;
        }
        else
        {
            localValue[localCount] = input[i];
            localCount++;
            // 숫자 혹은 수식
        }
    }
    memset(&input[0], 0, sizeof(input));
    inputLength = 0;

    for (i = 0; i < ARRAY_SIZE; i++)
    {
        if (newForm[i] == 0)
        {
            break;
        }
        else
        {
            input[i] = newForm[i];
            inputLength++;
        }
    }
}
/********************************************************************************************************/

/********************************************************************************************************/
void cal()
{
    int i, result;
    while(1)
    {
        printf("수식을 입력하세요. 종료를 원하시면 q를 입력하세요.\n");
        printf("입력 : ");
        scanf("%s", input);
        if (input[0] == 'q' || input[0] == 'Q')
        {
            // q라는 값을 입력받았는지 확인하고, 입력받았으면 while문을 벗어난다.
            break;
        }

        else
        {
            // 입력한 수식의 길이를 알아낸다.
            // strlen을 사용하여 길이를 구해도 되지만 사용자가space를 입력할수도 있기 때문에 for문으로 체크함
            for (i = 0; i < ARRAY_SIZE; i++)
            {
                if (input[i] == 0x00)
                {
                    inputLength = i;
                    break;
                }
            }

            // 수식을 한 번 출력하고,
            printInputArray();
            // 배열 입력 값 검사.
            // 0~9와 *-+/()만 입력 가능하도록 필터링한다.
            handle = 0;
            for (i = 0; i < inputLength; i++)
            {
                if ((
                    (input[i] >= '0') ||
                    (input[i] <= '9') ||
                    (input[i] == '+') ||
                    (input[i] == '-') ||
                    (input[i] == '*') ||
                    (input[i] == '/') ||
                    (input[i] == '(') ||
                    (input[i] == ')')
                   ) == 1)
                {
                    continue;
                }
                else
                {
                    // 숫자와 괄호, 그리고 사칙연산 이외의 값을 입력받으면 -1로 변환
                    handle = -1;
                }
            }

            // -1일 경우(엉뚱한 값을 입력받았을 경우)
            if (handle == -1)
            {
                printf("잘못된 입력 값이 있습니다.\n"); // 메시지 출력
            }
            else
            {
                // 괄호를 다 풀어낼 때 까지 무한반복한다.
                while (1)
                {
                    guarhandle = 0;
                    for (i = 0; i < inputLength; i++)
                    {
                        // 괄호를 가지고 있는지 확인하고,
                        if (input[i] == '(')
                        {
                            // 있으면 1로 변경
                            guarhandle = 1;
                            break;
                        }
                    }
                    if (guarhandle == 1)// 1일 경우(괄호를 가지고 있을 경우)
                    {
                        bracketOperation(); // 괄호 하나를 풀어내고
                        printInputArray(); // 결과를 출력
                    }
                    else
                    { // 괄호가 없을 경우, 반복문을 벗어난다.
                        break;
                    }
                }
                // 기타 잔여 사칙연산을 수행한다.
                result = fundamentalOperation(input, inputLength);
                printf("결과는 %d입니다.\n", result); // 결과 출력
            }
        }
        // 배열의 모든 데이터를 0으로 초기화
        memset(&input[0], 0, sizeof(input));
        inputLength = 0;
    }
    printf("종료합니다.\n");
    return ;
}
/********************************************************************************************************/

/********************************************************************************************************/
//보드 초기화
void init_board()
{
    int i, j;
    for (i = 0; i < BOARD_SIZE; i++)
    {
        for (j = 0; j < BOARD_SIZE; j++)
        {
            board[i][j] = '_';
        }
    }
}
/********************************************************************************************************/

/********************************************************************************************************/
//승부체크
int check_win(int row, int col)
{
    char check_win_dol = board[row][col];
    int i, j;
    int count = 0;

    //가로 방향으로
    for (j = 0; j < BOARD_SIZE; j++)
    {

        if (board[row][j] == check_win_dol)
        {
            count++;
        }
        else
        {
            count = 0;
        }
        if (count == 5)
        {
            return 0;
        }
    }

    // 세로 방향으로
    count = 0;
    for (i = 0; i < BOARD_SIZE; i++)
    {
        if (board[i][col] == check_win_dol)
        {
            count++;
        }
        else
        {
            count = 0;
        }
        if (count == 5)
        {
            return 0;
        }
    }

    // 왼쪽으로 휘어진 방향으로(\)
    count = 0;
    i = row, j = col;
    while (i > 0 && j > 0)
    {
        i--;
        j--;
    }
    while (i < BOARD_SIZE && j < BOARD_SIZE)
    {
        if (board[i][j] == check_win_dol)
        {
            count++;
        }
        else
        {
            count = 0;
        }
        if (count == 5)
        {
            return 0;
        }
        i++;
        j++;
    }

    // 오른쪽으로 휘어진 방향으로(/)
    count = 0;
    i = row;
    j = col;
    while (i > 0 && j < BOARD_SIZE - 1)
    {
        i--;
        j++;
    }
    while (i < BOARD_SIZE && j >= 0)
    {
        if (board[i][j] == check_win_dol)
        {
            count++;
        }
        else
        {
            count = 0;
        }
        if (count == 5)
        {
            return 0;
        }
        i++;
        j--;
    }

    return -1; // 5돌 연속 아닐경우
}
/********************************************************************************************************/

/********************************************************************************************************/
void print_sigint(int  signo)
{
    if (signo == SIGINT)
    {
        printf( "ctrl + c 탐지됨\n");
    }
    else if (signo == SIGSTOP)
    {
        printf( "외주에서 중지됨\n");
    }
    else if (signo == SIGTSTP)
    {
        printf( "ctrl + z 탐지됨\n");
    }
    else if (signo == SIGSEGV)
    {
        printf( "메모리 주소 참조 실패\n");
    }
    else if (signo == SIGBUS)
    {
        printf( "메모리 접근 에러\n");
    }
    else if (signo == SIGTERM)
    {
        printf( "프로세스 종료\n");
    }
    else if (signo == SIGPIPE)
    {
        printf( "bank socket close\n");
    }
    return;
}
/********************************************************************************************************/

/********************************************************************************************************/
void signal_handler()
{
    signal(SIGINT,  print_sigint);
    signal(SIGSTOP, print_sigint); 
    signal(SIGTSTP, print_sigint); 
    signal(SIGSEGV, print_sigint);
    signal(SIGBUS,  print_sigint);
    signal(SIGPIPE, print_sigint);
    signal(SIGQUIT, print_sigint);
    signal(SIGTERM, print_sigint);
    return;
}
/********************************************************************************************************/

/********************************************************************************************************/
void init_curses()
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
}
/********************************************************************************************************/

/********************************************************************************************************/
void draw_board()
{
    int i,j;
    clear();
    for (i = 0; i < BOARD_SIZE; i++)
    {
        for (j = 0; j < BOARD_SIZE; j++)
        {
            mvprintw(i, j*2, "+");
        }
    }
    refresh();
}
/********************************************************************************************************/

/********************************************************************************************************/
int get_move(char player, int * prow, int * pcol)
{
    int row = 0, col = 0;
    while (1)
    {
        //mvprintw(BOARD_SIZE+1, 0, "Player %c's turn: ", player);
        //refresh();
        int ch = getch();
        if(board[row][col] == '_')
        {
            mvprintw(row, col*2, "%c", '+');
        }
        if (ch == KEY_UP)
        {
            row--;
            if(board[row][col] == '_')
            {
                mvprintw(row, col*2, "%c", player);
                refresh();
            }
        }
        else if (ch == KEY_DOWN)
        {
            row++;
            if(board[row][col] == '_')
            {
                mvprintw(row, col*2, "%c", player);
                refresh();
            }
        }
        else if (ch == KEY_LEFT)
        {
            col--;
            if(board[row][col] == '_')
            {
                mvprintw(row, col*2, "%c", player);
                refresh();
            }
        }
        else if (ch == KEY_RIGHT)
        {
            col++;
            if(board[row][col] == '_')
            {
                mvprintw(row, col*2, "%c", player);
                refresh();
            }
        }
        else if (ch == '\n' && board[row][col] == '_' && row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE) // 위치 유효성
        {
            board[row][col] = player;
            mvprintw(row, col*2, "%c", player);
            refresh();
	    *pcol = col;
	    *prow = row;
            return 1;
        }
        row = (row + BOARD_SIZE) % BOARD_SIZE;
        col = (col + BOARD_SIZE) % BOARD_SIZE;
    }
}
/********************************************************************************************************/
