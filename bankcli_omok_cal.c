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
void   print_board();
int    check_valid_move(int row, int col);
int    check_win(int row, int col);
void   omok(int sock);
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
//INFIX�� POSTFIX�� ǥ���Ͽ� ����ϴ¹������ ����
//����ǥ���(infix)->����ǥ���(postfix)

//'0', '9', '+', '-', '*', '/', '(', ')' Ȯ���ڵ鷯
int handle;

//��ȣ Ȯ���ϴ� �ڵ鷯
int guarhandle;

//���ϱ⳪���� Ȯ���ϴ� �ڵ鷯
int divmulhandle;

// �Է� ���� �ִ� ������ ����
//#define ARRAY_SIZE 200

// �Է� ���� ������ ����Ǵ� �迭
char input[ARRAY_SIZE] = {0};

// �Է� ���� ������ ���̸� �����ϴ� ����
int inputLength = 0;
/********************************************************************************************************/

/********************************************************************************************************/
/*Omok VALUE */
//������
char board[BOARD_SIZE][BOARD_SIZE];

//������ ���̺�
char table[3];

//����� ��
char dol;

//���� ����üũ 0���̺� ���� 1 ����
char start =  -1;

//�º� üũ w/l  win/loss
char win_loss = 0;

//�켱���� üũ y/n  y������ n�������
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
    int    row, col;
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
            printf("�̸�:%s��\n����:%s\n���:%s\n", name, act, pw);

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
                printf("\n����:Q, ��ȸ:J, ä��:M, �Ա�:P, ���:G, ����:C, �������:5:");
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
                    printf("���̺� ����[00~09]:");
		    fgets(table, 3, stdin);
		    while(getchar()!='\n');
                    printf("����� �� ����[0~Z]:");
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
			    printf("\n��� ��ٸ�����...\n");
			    continue;
			}
			else if(start == 1)
			{
			    memcpy(message, "OS:", 3);
			    init_board();
			    win_loss = 0;
			    print_board();
			    while(1)
		            {
				sleep(1);
				if(win_loss == 'l')
				{
			            printf("you Loss, ��������\n");
				    break;
				}
				if(turn == 'y')
				{
			            printf("������\n");
			            printf("��(0~14)�Է�: ");
                                    scanf("%d", &row);
				    while(getchar()!='\n');
                                    printf("��(0[A]~14[O])�Է�: ");
                                    scanf("%d", &col);
				    while(getchar()!='\n');
                                    check =  check_valid_move(row, col);
                                    if(check < 0)
                                    {
                                        printf("�߸��� ��ġ�� %d\n", check);
                                        continue;
                                    }
				    board[row][col] = dol;

                                    sprintf(s_message->sRow, "%d", row);
                                    sprintf(s_message->sCol, "%d", col);

				    turn = 'n';
				    write(sock, message, sizeof(SEND_FMT));
				    check = check_win(row, col);
                                    print_board();
				    if(check == 0)
			            {
                                        print_board();
				        printf("You Win, ��������\n");
                                        memcpy(message, "OO", 2);
                                        write(sock, message, sizeof(SEND_FMT));
				        break;
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
                printf("\n%dȸ ����\n", check);
                printf("\nRECV FROM BANK[SIZE:%d]:%s\n", sizeof(message), message);
                sleep(1);
                if(check == 5)
                {
                    printf("\n%dȸ ����, ����湮�ϼ���!!!\n", check);
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
                    printf("\n���࿡ ������ �ĵ���!!!\n");
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
                    printf("\n���� �ܾ�:%.*s\n", sizeof(s_message->sBlc), s_message->sBlc);
                }

                else if(!memcmp(s_message->sTrCd, "PS", 2))
                {
                    printf("\nRECV FROM BANK[SIZE:%d]:%s\n", sizeof(message), message);
                    printf("\n�Ա��� �ܾ�:%.*s\n", sizeof(s_message->sBlc), s_message->sBlc);
                }

                else if(!memcmp(s_message->sTrCd, "GE", 2))
                {
                    printf("\nRECV FROM BANK[SIZE:%d]:%s\n", sizeof(message), message);
                    printf("\n����� �ܾ�:%.*s\n", sizeof(s_message->sBlc), s_message->sBlc);
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
                    printf("\n���� ���� ����...\n");
                    turn = 'y';
                }

		else if(!memcmp(s_message->sTrCd, "O2", 2))
                {
                    printf("\n���߿� ���� �ļ�...\n");
                    turn = 'n';
                }

		else if(!memcmp(s_message->sTrCd, "O3", 2))
                {
                    printf("\n���� ��ŸƮ...\n");
                    start = 1;
                }

		else if(!memcmp(s_message->sTrCd, "OE", 2))
		{    
		    start = -1;
		    printf("\n�ش����̺� ����\n");
		}

		else if(!memcmp(s_message->sTrCd, "OF", 2))
		{    
		    start = -1;
		    printf("\n�ش����̺� ������\n");
		}

		else if(!memcmp(s_message->sTrCd, "OS", 2))
		{
                    printf("\nRECV FROM BANK[SIZE:%d]:%s\n", sizeof(message), message);
		    row = atoi(s_message->sRow);
                    col = atoi(s_message->sCol);
                    printf("\ncol row\n");
                    board[row][col] = *(s_message->sDol);
                    printf("\ndol\n");
                    print_board();
		    check = check_win(row, col);
                    if(check == 0)
                    {
			win_loss = 'l';
                    }
		    turn = 'y';
		}

		else if(!memcmp(s_message->sTrCd, "OO", 2))
		{
		    printf("\n��밡 ����\n");
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
    for (int i =0; i < inputLength; i++)
    {
        printf("%c", input[i]);
    }
    printf("\n");
}
/********************************************************************************************************/

/********************************************************************************************************/
// ��Ģ ������ ����� int������ ��ȯ�ϴ� �Լ��̴�. �Ķ���ͷ� ��Ģ���� ���İ�, �� ������ ���̸� �޴´�.
int fundamentalOperation(char localValue[ARRAY_SIZE], int localCount)
{
    int  INT[ARRAY_SIZE] = {0};           // ���� �����ϴ� ����
    int  INTidx = 0;                         // v�迭�� ���� ������ ����ϱ� ���� ����
    char SYMBOL[ARRAY_SIZE] = {0};      // ���ı�ȣ�� �����ϴ� ����
    int  SYMBOLidx = 0;                     // con�迭�� ���� ������ ����ϱ� ���� ����
    int  INTbuf = 0;                         // ���ڿ� �迭�� �����ִ� ���� �����ϴ� ����(���� �����ϴ� �������� �̵�)
    divmulhandle = 0;       // ���ϱ⳪ �����⸦ ������ �ִ��� Ȯ���ϱ� ���� ����
    for (int j = 0; j < localCount; j++)
    {
        if ('0' <= localValue[j] && localValue[j] <= '9')       // ������ ��� sqr�� �����Ѵ�.
        {
            INTbuf *= 10;                                               // ������ ����ִ� sqr ���� 10�� ���Ͽ� ��ĭ �о��ְ� ���� ������ �׳�0�̴�.
            INTbuf += localValue[j] - '0';                            // ���ο� ���ڸ� �����ش�. ĳ�������̱� ������ '0'�� ���� ���� ���� ���Ѵ�.
        }

        else if (localValue[j] == '-' || localValue[j] == '+')   // - + �������� ���
        {
            INT[INTidx] = INTbuf;      // ������ ���� ���ڸ� v�迭�� �߰��ϰ�,
            INTbuf = 0;            // sqr�� �ʱ�ȭ�Ѵ�.
            INTidx++;             // �迭�� ũ�⸦ ����ϱ� ���� vIdx�� 1�� �����ش�.

            if (divmulhandle == 1)
            { // ���ϱ⳪ �����⸦ ������ ������ Ȯ���� ���¶��
                int tmp = 0;
                if (SYMBOL[SYMBOLidx-1] == '*')
                {
                    printf("���� ���");
                    tmp = INT[INTidx-2] * INT[INTidx-1];
                }

                else if (SYMBOL[SYMBOLidx-1] == '/')
                {
                    printf("������ ���");
                    tmp = INT[INTidx-2] / INT[INTidx-1];
                }
                printf(" %d\n", tmp);
                divmulhandle = 0;
                // ����� ������ �迭���� �������ش�.
                SYMBOL[SYMBOLidx-1] = 0;
                SYMBOLidx--;

                INT[INTidx-1] = 0;
                INT[INTidx-2] = 0;
                INTidx -= 2;

                INT[INTidx] = tmp;
                INTidx ++;
            }

            SYMBOL[SYMBOLidx] = localValue[j]; // ���ĵ� �迭�� �߰��Ѵ�.
            printf("con[%d] : %c\n", SYMBOLidx, SYMBOL[SYMBOLidx]);
            SYMBOLidx++;

        }
        else if (localValue[j] == '*' || localValue[j] == '/')
        { // * / �������� ���,
            INT[INTidx] = INTbuf;      // ������ ���� ���ڸ� v�迭�� �߰��ϰ�,
            INTbuf = 0;            // sqr�� �ʱ�ȭ�Ѵ�.
            INTidx++;             // �迭�� ũ�⸦ ����ϱ� ���� vIdx�� 1�� �����ش�.

            if (divmulhandle == 1)
            { // ������ �����̳� ������ ������ �޾Ƶа� �ִٸ�
                int tmp = 0;
                if (SYMBOL[SYMBOLidx-1] == '*')
                {
                    printf("���� ���");
                    tmp = INT[INTidx-2] * INT[INTidx-1];
                }
                else if (SYMBOL[SYMBOLidx-1] == '/')
                {
                    printf("������ ���");
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

            SYMBOL[SYMBOLidx] = localValue[j]; // ���ĵ� �迭�� �߰��Ѵ�.
            SYMBOLidx++;

            divmulhandle = 1; // ���ϱ⳪ �����⸦ �Է¹޾����� ǥ���صд�.
        }

        if (j == localCount-1)
        { // ������ �� �Է¹��� ���·�, ���ϱ⳪ �����⸦ �Է� ���� ���¶��
            INT[INTidx] = INTbuf;      // ������ ���� ���ڸ� v�迭�� �߰��ϰ�,
            INTbuf = 0;            // sqr�� �ʱ�ȭ�Ѵ�.
            INTidx++;             // �迭�� ũ�⸦ ����ϱ� ���� vIdx�� 1�� �����ش�.

            if (divmulhandle == 1)
            {
                int tmp = 0;
                if (SYMBOL[SYMBOLidx-1] == '*')
                {
                    printf("���� ���");
                    tmp = INT[INTidx-2] * INT[INTidx-1];
                }
                else if (SYMBOL[SYMBOLidx-1] == '/')
                {
                    printf("������ ��� %d, %d", INT[INTidx-2], INT[INTidx-1]);
                    tmp = INT[INTidx-2] / INT[INTidx-1];
                }
                printf(" %d\n", tmp);
                divmulhandle = 0;
                // ����� ������ �迭���� �������ش�.
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

    int result = INT[0];
    for (int i =0; i < SYMBOLidx; i++)
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
// �������� input�� �ִ� ������ ���� ���� ������ ���� ���� ��ȣ �ϳ��� Ǯ���,
// input�� ������ �������ϴ� �Լ��̴�.
void bracketOperation()
{
    // ���� ���� ��ȣ���� �ִ� ������ ã�´�.
    // ��ȣ�� ���� �� ���� +1, ���� �� ���� -1
    // �� �̻� ���ο� ��ȣ�� ���� ��(�ݴ� ��ȣ�� ���� ��) ��� ����
    // 1���� -> ��ȣ ���ο� �ִ� ����
    // 2���� -> ����/������
    // 3���� -> ����/����
    int newForm[ARRAY_SIZE] = {0};
    int newFormIdx = 0;
    char localValue[ARRAY_SIZE] = {0};
    int localCount = 0;
    int localStart = 0;
    int localEnd = 0;
    int localResult = 0;
    for (int i = 0; i < inputLength; i++)
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
            for (int j = 0; j < ARRAY_SIZE; j++)
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
                    for (int k = 0; k < 11; k++)
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
            // ���� Ȥ�� ����
        }
    }
    memset(&input[0], 0, sizeof(input));
    inputLength = 0;

    for (int i = 0; i < ARRAY_SIZE; i++)
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
    while(1)
    {
        printf("������ �Է��ϼ���. ���Ḧ ���Ͻø� q�� �Է��ϼ���.\n");
        printf("�Է� : ");
        scanf("%s", input);
        if (input[0] == 'q' || input[0] == 'Q')
        {
            // q��� ���� �Է¹޾Ҵ��� Ȯ���ϰ�, �Է¹޾����� while���� �����.
            break;
        }

        else
        {
            // �Է��� ������ ���̸� �˾Ƴ���.
            // strlen�� ����Ͽ� ���̸� ���ص� ������ ����ڰ�space�� �Է��Ҽ��� �ֱ� ������ for������ üũ��
            for (int i = 0; i < ARRAY_SIZE; i++)
            {
                if (input[i] == 0x00)
                {
                    inputLength = i;
                    break;
                }
            }

            // ������ �� �� ����ϰ�,
            printInputArray();
            // �迭 �Է� �� �˻�.
            // 0~9�� *-+/()�� �Է� �����ϵ��� ���͸��Ѵ�.
            handle = 0;
            for (int i = 0; i < inputLength; i++)
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
                    // ���ڿ� ��ȣ, �׸��� ��Ģ���� �̿��� ���� �Է¹����� -1�� ��ȯ
                    handle = -1;
                }
            }

            // -1�� ���(������ ���� �Է¹޾��� ���)
            if (handle == -1)
            {
                printf("�߸��� �Է� ���� �ֽ��ϴ�.\n"); // �޽��� ���
            }
            else
            {
                // ��ȣ�� �� Ǯ� �� ���� ���ѹݺ��Ѵ�.
                while (1)
                {
                    guarhandle = 0;
                    for (int i = 0; i < inputLength; i++)
                    {
                        // ��ȣ�� ������ �ִ��� Ȯ���ϰ�,
                        if (input[i] == '(')
                        {
                            // ������ 1�� ����
                            guarhandle = 1;
                            break;
                        }
                    }
                    if (guarhandle == 1)// 1�� ���(��ȣ�� ������ ���� ���)
                    {
                        bracketOperation(); // ��ȣ �ϳ��� Ǯ���
                        printInputArray(); // ����� ���
                    }
                    else
                    { // ��ȣ�� ���� ���, �ݺ����� �����.
                        break;
                    }
                }
                // ��Ÿ �ܿ� ��Ģ������ �����Ѵ�.
                int result = fundamentalOperation(input, inputLength);
                printf("����� %d�Դϴ�.\n", result); // ��� ���
            }
        }
        // �迭�� ��� �����͸� 0���� �ʱ�ȭ
        memset(&input[0], 0, sizeof(input));
        inputLength = 0;
    }
    printf("�����մϴ�.\n");
    return ;
}
/********************************************************************************************************/

/********************************************************************************************************/
//���� �ʱ�ȭ
void init_board()
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            board[i][j] = '_';
        }
    }
}
/********************************************************************************************************/

/********************************************************************************************************/
//���� ���
void print_board()
{
    system("clear");
    printf("\n  ");
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        printf(" %c ", 'A' + i);
    }
    printf("\n");
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        printf("%2d", i);
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            printf(" %c|", board[i][j]);
        }
        printf("\n");
    }
}
/********************************************************************************************************/

/********************************************************************************************************/
//���� ��ȿ�� üũ
int check_valid_move(int row, int col)
{
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE)
    {
        return -1; //���� ���� �ʰ���
    }
    if (board[row][col] != '_')
    {
        return -1; //�ش� ��ġ�� ����������
    }
    return 0;
}
/********************************************************************************************************/

/********************************************************************************************************/
//�º�üũ
int check_win(int row, int col)
{
    char check_win_dol = board[row][col];
    int count = 0;

    //���� ��������
    for (int j = 0; j < BOARD_SIZE; j++)
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

    // ���� ��������
    count = 0;
    for (int i = 0; i < BOARD_SIZE; i++)
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

    // �������� �־��� ��������(\)
    count = 0;
    int i = row, j = col;
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

    // ���������� �־��� ��������(/)
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

    return -1; // 5�� ���� �ƴҰ��
}
/********************************************************************************************************/

/********************************************************************************************************/
void print_sigint(int  signo)
{
    if (signo == SIGINT)
    {
        printf( "ctrl + c Ž����\n");
    }
    else if (signo == SIGSTOP)
    {
        printf( "���ֿ��� ������\n");
    }
    else if (signo == SIGTSTP)
    {
        printf( "ctrl + z Ž����\n");
    }
    else if (signo == SIGSEGV)
    {
        printf( "�޸� �ּ� ���� ����\n");
    }
    else if (signo == SIGBUS)
    {
        printf( "�޸� ���� ����\n");
    }
    else if (signo == SIGTERM)
    {
        printf( "���μ��� ����\n");
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
