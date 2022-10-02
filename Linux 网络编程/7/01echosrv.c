#include<unistd.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<errno.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<stdio.h>
#include<stdlib.h>

#define ERR_EXIT(m) \
    do{ \
        perror(m); \
        exit(EXIT_FAILURE); \
    }while(0);

int main(){


     int listenfd; 
     if((listenfd = socket(AF_INET, SOCK_STREAM , IPPROTO_TCP)) < 0)
         ERR_EXIT("listenfd");
     int on = 1;
     if((setsockopt(listenfd , SOL_SOCKET , SO_REUSEADDR ,  &on , sizeof(on))) < 0)
         ERR_EXIT("setsockopt");

     struct sockaddr_in srvaddr ;
     memset(&srvaddr , 0 , sizeof(srvaddr));
     srvaddr.sin_family = AF_INET;
     srvaddr.sin_port = htons(7878);
     srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);

     if((bind(listenfd , (struct sockaddr*)&srvaddr , sizeof(srvaddr) )) < 0)
         ERR_EXIT("bind");

     if((listen(listenfd , SOMAXCONN)) < 0)
         ERR_EXIT("listen");

    struct sockaddr_in peeraddr;
    socklen_t peerlen ;
    memset(&peeraddr , 0 , sizeof(peeraddr));
    int connfd ;
    if(( connfd = accept(listenfd , (struct sockaddr* )&connfd, &peerlen) ) < 0)
        ERR_EXIT("accept");
    struct sockaddr_in localaddr;
    socklen_t locallen;
    if((getsockname(connfd , (struct sockaddr*)&localaddr, &locallen  )) <0)
        ERR_EXIT("getsockname");

    printf("连接成功：S(%s , %d) <-> D(%s , %d)\n ", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port),
                                                        inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
    char recvbuf[1024];
    int ret = 0;
    while(1){
        
        ret = read(connfd, recvbuf ,sizeof(recvbuf));
        if(ret < 0){
            if(ret == -1 && errno == EINTR)
                continue;
            ERR_EXIT("read");
        }else if(ret == 0){
            printf("通信结束：S(%s , %d) <-> D(%s , %d)\n ", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port),
                                                        inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
            break;
        }
        printf("(%s , %d ) -> (%s , %d): " , inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port),
                                             inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port));
        fputs(recvbuf , stdout);
        write(connfd , recvbuf ,strlen(recvbuf));
        memset(recvbuf, 0 , sizeof(recvbuf));
    }

    close(connfd);
    close(listenfd);

    return 0;
}
