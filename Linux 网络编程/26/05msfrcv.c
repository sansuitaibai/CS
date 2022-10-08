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

int main(int argc , char* argv[]){
    int opt = -1;
    int flag = 0;
    int type = 0;
    while(1){
        opt = getopt(argc , argv , "nt:");
        if(opt == -1)
            break;
        if(opt == '?')
            ERR_EXIT("getopt");
        switch(opt){
            case 'n':
                flag |= IPC_NOWAIT;
                break;
            case 't':
                type = atoi(optarg);
                break;
        }
    }
    int msgfd ;
    msgfd = msgget(1234 ,0);
    if(msgfd < 0)
        ERR_EXIT("msgget");
    struct msgbuf* ptrmsg;
    ptrmsg = (struct msgbuf*)malloc(sizeof(long) + MSGMAX);
    if((msgrcv(msgfd , ptrmsg ,MSGMAX, type , flag)) < 0)
        ERR_EXIT("msgsnd");
    printf("type = %d ,msglen = %ld ,  msg = %s \n", type ,strlen(ptrmsg->mtext) , ptrmsg->mtext);
    struct msqid_ds ds;
    memset(&ds , 0, sizeof(ds));
    if((msgctl(msgfd, IPC_STAT , &ds) ) < 0)
        ERR_EXIT("msgctl");
    printf("msgfd = %d \n", msgfd);
    printf("id = %d \n", (int)ds.msg_perm.__key); 
    printf("current bytes = %d \n",(int)ds.__msg_cbytes);
    printf("queue msgnum = %d \n", (int)ds.msg_qnum);
    printf("queue MSGMAX = %d \n", (int)ds.msg_qbytes);
    return 0;

}
        
