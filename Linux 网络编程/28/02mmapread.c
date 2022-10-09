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

#define ERR_EXIT(m) \
    do{ \
        perror(m); \
        exit(EXIT_FAILURE); \
    }while(0)

typedef struct STU{

    int age ;
    char name[4];

}STU;

int  main(int argc , char* argv[]){
    if(argc != 2){
        fprintf(stderr , "argc is invalid");
        exit(EXIT_FAILURE);
    }
    int mapfd = open(argv[1] , O_RDWR);
    STU* mapptr = (STU*)mmap(NULL ,sizeof(STU)*5  , PROT_READ | PROT_WRITE, MAP_SHARED , mapfd , 0 ); 
    if(mapptr == NULL)
        ERR_EXIT("mmap");
    for(int i = 0 ; i < 10 ; ++i){
        printf("name: %s , age: %d\n", (mapptr+i)->name , (mapptr + i)->age);
    }
    munmap(mapptr , sizeof(STU)*5); 
    printf("exit...\n");
    return 0;
}

        
