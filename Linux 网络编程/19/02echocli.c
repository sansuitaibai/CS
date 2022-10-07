#include<unistd.h>
#include<fcntl.h>
#include<poll.h>
#include<sys/select.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/epoll.h>

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include<vector>
using namespace std;

#define ERR_EXIT(m) \
    do{ \
        perror(m); \
        exit(EXIT_FAILURE); \
    }while(0)

void do_client(int sockfd , struct sockaddr_in srvaddr , struct sockaddr_in peeraddr){
    if(sockfd < 0){
     ERR_EXIT("sockfd < 0");
    }
    
    char recvbuf[1024];
    char sendbuf[1024];
    memset(recvbuf , 0 , sizeof(recvbuf));
    memset(sendbuf , 0 ,sizeof(sendbuf));
    int ret = -1;
    socklen_t addrlen ;
    if((connect(sockfd , (struct sockaddr*)&srvaddr, sizeof(srvaddr))) < 0)
        ERR_EXIT("connect");
    addrlen = sizeof(peeraddr);
    if((getsockname(sockfd , (struct sockaddr*)&peeraddr, &addrlen)) < 0)
        ERR_EXIT("getsockname");
    while((fgets(sendbuf , sizeof(sendbuf), stdin)) != NULL){
        sendto(sockfd , sendbuf , strlen(sendbuf), 0 , NULL , 0);     
        ret = recvfrom(sockfd , recvbuf , sizeof(recvbuf), 0 , NULL , NULL);
        if(ret < 0){
            if(errno == EINTR)
                continue;
            printf("通信结束：C(%s , %d) <-> S(%s , %d)\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port),
                                                            inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port));
            close(sockfd);
            return ;
        }
         
        printf("S(%s , %d) ->  ", inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port));
        fputs(recvbuf , stdout);
    }
    close(sockfd);
}



int main(){
    int sockfd ;
    if((sockfd =  socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP)) < 0)
        ERR_EXIT("socket");
    struct sockaddr_in srvaddr;
    struct sockaddr_in localaddr;
    memset(&srvaddr , 0 , sizeof(srvaddr));
    memset(&localaddr , 0 , sizeof(localaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(6767);
    srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    do_client(sockfd , srvaddr , localaddr);


    return 0;
}

















