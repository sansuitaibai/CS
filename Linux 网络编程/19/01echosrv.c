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

void do_service(int sockfd , struct sockaddr_in srvaddr , struct sockaddr_in peeraddr){
    if(sockfd < 0){
     ERR_EXIT("sockfd < 0");
    }
    char recvbuf[1024];
    memset(recvbuf , 0 , sizeof(recvbuf));
    int ret = -1;
    socklen_t addrlen ;
    while(1){
        addrlen = sizeof(peeraddr);
        ret = recvfrom(sockfd , recvbuf , sizeof(recvbuf), 0 , (struct sockaddr*)&peeraddr , &addrlen);
        if(ret <0){
            if(errno == EINTR)
                continue;
            close(sockfd);
            return ;
        }
        printf("C(%s , %d) -> S(%s , %d): ", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port),
                                             inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port));
        fputs(recvbuf , stdout);
        sendto(sockfd,recvbuf , strlen(recvbuf), 0 , (struct sockaddr*)&peeraddr , sizeof(peeraddr));
    }


}



int main(){
    int sockfd ;
    if((sockfd =  socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP)) < 0)
        ERR_EXIT("socket");
    struct sockaddr_in srvaddr;
    struct sockaddr_in peeraddr;
    memset(&srvaddr , 0 , sizeof(srvaddr));
    memset(&peeraddr , 0 , sizeof(peeraddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(6767);
    srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if((bind(sockfd, (struct sockaddr*)&srvaddr , sizeof(srvaddr))) < 0)
        ERR_EXIT("bind");
    do_service(sockfd , srvaddr , peeraddr);


    return 0;
}

















