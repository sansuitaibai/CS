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


     int sockfd; 
     if((sockfd = socket(AF_INET, SOCK_STREAM , IPPROTO_TCP)) < 0)
         ERR_EXIT("listenfd");

     struct sockaddr_in srvaddr ;
     memset(&srvaddr , 0 , sizeof(srvaddr));
     srvaddr.sin_family = AF_INET;
     srvaddr.sin_port = htons(7878);
     srvaddr.sin_addr.s_addr =  inet_addr("127.0.0.1");

    struct sockaddr_in localaddr;
    memset(&localaddr , 0 , sizeof(localaddr));
    socklen_t locallen;
    if((connect(sockfd, (struct sockaddr*)&srvaddr,sizeof(srvaddr))) < 0)
        ERR_EXIT("connect");
    if((getsockname(sockfd , (struct sockaddr*)&localaddr, &locallen  )) <0)
        ERR_EXIT("getsockname");

    printf("连接成功：S(%s , %d) <-> D(%s , %d)\n ", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port),
                                                        inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port));
    char recvbuf[1024];
    char sendbuf[1024];
    int ret = 0;
    while( (fgets(sendbuf , sizeof(sendbuf), stdin)) != NULL ){
        
        ret = write(sockfd, sendbuf , strlen(sendbuf));

        if(ret < 0){
            ERR_EXIT("write");
        }
        ret = read(sockfd , recvbuf , sizeof(recvbuf));
        if(ret < 0){
            ERR_EXIT("read");
        }else if(ret == 0){
            printf("通信结束: S(%s , %d ) <-> (%s , %d)\n ", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port),
                                                             inet_ntoa(srvaddr.sin_addr ), ntohs(srvaddr.sin_port));
            break;
        }
        printf("(%s , %d ) -> (%s , %d): " , inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port),
                                             inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port));
        fputs(recvbuf , stdout);
        memset(recvbuf, 0 , sizeof(recvbuf));
        memset(sendbuf, 0 , sizeof(sendbuf));
    }

    close(sockfd);

    return 0;


}
