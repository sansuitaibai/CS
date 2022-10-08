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

void echo_cli(int msgfd){
    struct msgbuf recvbuf;
    struct msgbuf sendbuf;
    memset(&sendbuf , 0 , sizeof(sendbuf));
    memset(&recvbuf , 0 , sizeof(recvbuf));
    int type  = getpid();
    sendbuf.mtype = 1;
    *(int*)sendbuf.mtext = type;
    int ret = -1;
    
    while(1){
        if((fgets(sendbuf.mtext + 4 , sizeof(sendbuf.mtext) - 4 , stdin)) == NULL){
            ERR_EXIT("fgets");
        }
        if( (msgsnd(msgfd , &sendbuf , strlen(sendbuf.mtext + 4) + 4 , 0)) < 0)
            ERR_EXIT("msgsnd");
        memset(sendbuf.mtext + 4 , 0 , sizeof(sendbuf.mtext) - 4);
        ret = msgrcv(msgfd , &recvbuf , MSGMAX, type , 0);
        if( ret <  0 ){
            if(errno == EINTR)
                continue;
            exit(EXIT_FAILURE);
        }
        printf("recv from srv: ");
        fputs(recvbuf.mtext + 4 , stdout);
        memset(&recvbuf , 0 , sizeof(recvbuf));
    }

}
    

int main(int argc , char* argv[]){
    int msgfd = msgget(1234 ,0); 
    if(msgfd < 0)
        ERR_EXIT("msgget");
    echo_cli(msgfd);
    return 0;

}
        
