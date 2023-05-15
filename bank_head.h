#include <stdio.h>                                         
#include <stdlib.h>                                        
#include <unistd.h>                                        
#include <string.h>                                        
#include <arpa/inet.h>                                     
#include <sys/socket.h>                                    
#include <netinet/in.h>                                    
#include <pthread.h>                                       
#include <sys/types.h>                                     
#include <poll.h>                                          
#include <fcntl.h>                                         
#include <signal.h>
#include <ncurses.h>
/**********************************************************/
/* DEFINE */                                               
#define BUF_SIZE 100                                       
#define MAX_ACCT 100                                       
/**********************************************************/
                                                           
/**********************************************************/
/* STRUCT 100 byte*/                                       
typedef struct                                             
{                                                          
    char    sTrCd[ 2];  //MS, LI, LO, HB, JH, PS, GE, ERR  
    char    sSeq [10];                                     
    char    sAcct[ 8];                                     
    char    sPw  [ 8];                                     
    char    sBlc [12];                                     
    char    sCash[12];                                     
    char    sDate[ 8];                                     
    char    sTime[ 6];                                     
    char    sIp  [15]; 
    char    sTable[ 2];    
    char    sDol [ 1];
    char    sRow [ 2];
    char    sCol [ 2];
    char    sHuman[1];
    char    sRes  [1];
    char    sNull[ 9];                                     
    char    sFill[ 1];                                     
}SEND_FMT;                                                 
                                                           
/* STRUCT 31 byte*/                                        
typedef struct                                             
{                                                          
    char    sIDX [ 2];                                     
    char    sACCT[ 8];                                     
    char    sPW  [ 8];                                     
    char    sBLC [12];                                     
    char    sFill[ 1];                                     
}CUST_INFO;                                                

/**********************************************************/

/*inline int     socket_set(int port);
inline int     socket_set_connect(char * ip, int port);
inline void    log_set(char * message);
inline void    error_handling(char * message);
inline char *  ltoa_(long plLong, char * psAscii, int piLen);
inline long    atol_(char * psAscii, int piLen);*/
