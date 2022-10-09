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

    int mapfd = shmget((key_t)234 , 0,0); 
    if(mapfd < 0)
        ERR_EXIT("shmget");
    STU* mapptr = (STU*)shmat(mapfd ,NULL , 0 ); 
    if(mapptr == NULL)
        ERR_EXIT("mmap");
    printf("name: %s , age: %d \n", mapptr->name , mapptr->age);
    memcpy(mapptr , "quit", 4);
    if((shmdt(mapptr)) < 0)
        ERR_EXIT("shmdt");
    printf("exit...\n");
    return 0;
}

        
