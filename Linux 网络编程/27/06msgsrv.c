/* ************************************************************************
> File Name:     01msgget.c
> Author:        sansuitaibai
> email:         2657302908@qq.com
> Created Time:  2022年10月08日 17时20分09秒 CST
> Description:   
 ************************************************************************/

#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<string.h>
#include<errno.h>
#include<stdio.h>
#include<stdlib.h>

#define ERR_EXIT(m) \
    do{ \
        perror(m); \
        exit(EXIT_FAILURE); \
    }while(0)

#define MSGMAX 8192

struct msgbuf{

    long mtype ;
    char mtext[MSGMAX];

};

void echo_srv(int msgfd){
    struct msgbuf recvbuf;
    memset(&recvbuf , 0 , sizeof(recvbuf));
    int  type = 1;
    int ret = -1;
    
    while(1){
        ret = msgrcv(msgfd , &recvbuf , MSGMAX, 1 , 0);
        if( ret <  0 ){
            if(errno == EINTR)
                continue;
            exit(EXIT_FAILURE); 
        }
        type  = *((int*)recvbuf.mtext);
        recvbuf.mtype = type; 
        printf("recving from client: ");
        fputs(recvbuf.mtext + 4 , stdout);
        if((msgsnd(msgfd , &recvbuf , ret  , 0 ) )<0 )
            ERR_EXIT("msgsnd");
        memset(recvbuf.mtext , 0 , sizeof(recvbuf.mtext));
        recvbuf.mtype = 1;
    }

}
    

int main(int argc , char* argv[]){
    int msgfd = msgget(1234 , 0666 | IPC_CREAT | IPC_EXCL);
    if(msgfd < 0)
        ERR_EXIT("msgget");
    echo_srv(msgfd);
    return 0;

}
        
