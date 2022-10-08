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


int main(){
    int msgfd ;
    msgfd = msgget(1234 , 0666 | IPC_CREAT | IPC_EXCL );
    if(msgfd < 0)
        ERR_EXIT("msgget");
    printf("msgget is success\n");
    
    










    



    
    return 0;

}
        
