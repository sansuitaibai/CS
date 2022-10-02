#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

#define ERR_EXIT(m) \
    do{ \
        perror(m); \
        exit(EXIT_FAILURE); \
    }while(0);

void do_service(int connfd, struct sockaddr_in srvaddr , struct sockaddr_in peeraddr){
    int ret = 0;
    char recvbuf[1024];
    while(1){

        ret = read(connfd , recvbuf, sizeof(recvbuf));
        if(ret < 0)
            ERR_EXIT("read");
        if(ret == 0){
            break;
        }
        printf("D(%s , %d )->S(%s , %d):", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port) , 
                                           inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port));
        fputs(recvbuf,stdout);
        write(connfd , recvbuf ,strlen(recvbuf));
        memset(recvbuf , 0 , sizeof(recvbuf));

    }

}


void handle_sigusr1(int sig){
    printf("通信结束： sig = %d\n", sig);
    exit(EXIT_SUCCESS);
}

int main(){

     int sockfd;
     if(( sockfd = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP)) < 0)
         ERR_EXIT("socket");
     struct sockaddr_in srvaddr;
     memset(&srvaddr ,0 , sizeof(srvaddr) );
     srvaddr.sin_family = AF_INET;
     srvaddr.sin_port = htons(9898);
     srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
     struct sockaddr_in localaddr;
     socklen_t locallen;
     memset(&localaddr , 0 , sizeof(localaddr));
     if(( connect(sockfd , (struct sockaddr*)&srvaddr , sizeof(srvaddr))) < 0 )
         ERR_EXIT("connect");

     if((getsockname(sockfd , (struct sockaddr*)&localaddr, &locallen)) < 0)
         ERR_EXIT("getsockname");

     printf("连接成功： S(%s , %d) <-> D(%s ,%d)\n", inet_ntoa(localaddr.sin_addr),ntohs(localaddr.sin_port),
                                                      inet_ntoa(srvaddr.sin_addr), srvaddr.sin_port);
     char recvbuf[1024];
     char sendbuf[1024];
     memset(recvbuf , 0 , sizeof(recvbuf));
     memset(sendbuf , 0 , sizeof(sendbuf));
     int ret = 0;
     pid_t pid = fork();
     if(pid < 0)
         ERR_EXIT("fork");
     if(pid > 0 ){
         while(1){
                ret = read(sockfd , recvbuf , sizeof(recvbuf));
                if(ret < 0){
                    ERR_EXIT("read");
                }else if(ret == 0){
                    printf("通信结束： S(%s , %d) <-> D(%s , %d)\n", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port),
                                                                     inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port));
                    kill(pid , SIGUSR1);
                    close(sockfd);
                    exit(EXIT_SUCCESS);
                 }
                printf("D(%s , %d) -> S(%s , %d):", inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port), 
                                             inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port));
                fputs(recvbuf , stdout);
                memset(recvbuf, 0 , sizeof(recvbuf));
         }
           
     }else{
                signal(SIGUSR1 , handle_sigusr1);
                while(fgets(sendbuf , sizeof(sendbuf), stdin) != NULL){
                   write(sockfd , sendbuf , strlen(sendbuf)); 
                   }
     }

    return 0;
}
