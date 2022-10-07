/* ************************************************************************
> File Name:     rlimit.c
> Author:        sansuitaibai
> email:         2657302908@qq.com
> Created Time:  2022年10月07日 16时47分37秒 CST
> Description:   
 ************************************************************************/
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m) \
        do \
        { \
             perror(m); \
             exit(EXIT_FAILURE);    \
        } while (0);

//仅仅改变当前进程最大文件描述符的限制
int main(void)
{
    //man getrlimit
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) <0)
        ERR_EXIT("getrlimit");

    printf("%d\n", (int)rl.rlim_max);

    rl.rlim_cur = 2048;
    rl.rlim_max = 2048;

    if (setrlimit(RLIMIT_NOFILE, &rl) <0)
        ERR_EXIT("setrlimit");

    if (getrlimit(RLIMIT_NOFILE, &rl) <0)
        ERR_EXIT("getrlimit");

    printf("%d\n", (int)rl.rlim_max);
    return 0;
}
