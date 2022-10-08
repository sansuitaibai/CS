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

#define MSGMAX 6

struct msgbuf{

    long mtype ;
    char mtext[MSGMAX];

};

int main(int argc , char* argv[]){
    if(argc != 3){
        fprintf(stderr ,"输入的参数不对: 项目名 长度 类型\n");
        exit(EXIT_FAILURE);
    }
    int len = atoi(argv[1]);
    int type = atoi(argv[2]);
    int msgfd ;
    msgfd = msgget(1234 ,0);
    if(msgfd < 0)
        ERR_EXIT("msgget");
      
    struct msgbuf* ptrmsg;
    ptrmsg = malloc(sizeof(long) + len);
    ptrmsg->mtype = type;
    if((msgsnd(msgfd , ptrmsg , len , 0)) < 0)
        ERR_EXIT("msgsnd");
    struct msqid_ds ds;
    memset(&ds , 0, sizeof(ds));
    if((msgctl(msgfd, IPC_STAT , &ds) ) < 0)
        ERR_EXIT("msgctl");
    printf("id = %d \n", (int)ds.msg_perm.__key); 
    printf("current bytes = %d \n",(int)ds.__msg_cbytes);
    printf("queue msgnum = %d \n", (int)ds.msg_qnum);
    printf("queue MSGMAX = %d \n", (int)ds.msg_qbytes);

    return 0;

}
        
