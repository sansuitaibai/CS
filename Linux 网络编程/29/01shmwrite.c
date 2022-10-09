/* ************************************************************************
> File Name:     01mmapwrite.c
> Author:        sansuitaibai
> email:         2657302908@qq.com
> Created Time:  2022年10月09日 16时08分58秒 CST
> Description:   
 ************************************************************************/
#include <sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define ERR_EXIT(m) \
    do{ \
        perror(m); \
        exit(EXIT_FAILURE); \
    }while(0)

typedef  struct STU{

    int age ;
    char name[80];

}STU;

int  main(int argc , char* argv[]){

    int mapfd = shmget((key_t)234 , sizeof(STU),  0666 | IPC_CREAT); 
    if(mapfd  == -1)
        ERR_EXIT("shmget");
    STU* mapptr = (STU*)shmat(mapfd ,NULL , 0 ); 
    if(mapptr == NULL)
        ERR_EXIT("mmap");
    mapptr->age = 99;
    memcpy(mapptr->name , "zhangsan", 8);
    printf("initialized over...\n");
    while(1){
        if( (memcmp(mapptr , "quit" , 0) ) == 0)
            break;
    }
    sleep(10);
    if((shmdt(mapptr)) < 0)
        ERR_EXIT("shmdt");
    shmctl(mapfd , IPC_RMID , NULL);
    printf("exit...\n");
    return 0;
}

        
