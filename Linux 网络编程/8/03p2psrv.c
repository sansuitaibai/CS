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

void do_service(int connfd, struct sockaddr_in srvaddr , struct                 sockaddr_in peeraddr){
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
        memset(recvbuf , 0 , sizeof(recvbuf));

    }

}

void handle_sigusr1(int sig){
    printf("通信结束： %d\n", sig);
    exit(EXIT_SUCCESS);
}

int main(){

     int listenfd;
     if(( listenfd = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP)) < 0)
         ERR_EXIT("socket");
     int on = 1;
     if(( setsockopt(listenfd , SOL_SOCKET , SO_REUSEADDR , &on , sizeof(on)) ) < 0)
         ERR_EXIT("setsockopt");
     struct sockaddr_in srvaddr;
     memset(&srvaddr ,0 , sizeof(srvaddr) );
     srvaddr.sin_family = AF_INET;
     srvaddr.sin_port = htons(9898);
     srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
     if(( bind(listenfd , (struct sockaddr*)&srvaddr , sizeof(srvaddr) ) ) < 0)
         ERR_EXIT("bind");
     if((listen(listenfd , SOMAXCONN)) < 0)
         ERR_EXIT("listen");
     struct sockaddr_in peeraddr;
     socklen_t peerlen;
     memset(&peeraddr , 0 , sizeof(peeraddr));
     int connfd ;
     pid_t pid ;
        if((connfd =  accept(listenfd , (struct sockaddr*)&peeraddr , &peerlen)) < 0)
            ERR_EXIT("accept");
        printf("连接成功： S(%s , %d) <-> D(%s , %d)\n", inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port),
                                                         inet_ntoa(peeraddr.sin_addr),ntohs(peeraddr.sin_port));
        pid = fork();
        if(pid < 0){
            ERR_EXIT("fork");
        }else if(pid >  0){
            close(listenfd);
            do_service( connfd ,srvaddr ,peeraddr);
            printf("通信结束： S(%s , %d) <-> D(%s , %d)\n", inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port),
                                                             inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
            kill(pid , SIGUSR1);
            close(connfd);
            exit(EXIT_SUCCESS);
        }else{
            close(listenfd);
            signal(SIGUSR1 , handle_sigusr1);
            char sendbuf[1024];
            memset(sendbuf , 0 , sizeof(sendbuf));
            while(fgets(sendbuf , sizeof(sendbuf), stdin)!= NULL){
                write(connfd , sendbuf , strlen(sendbuf));

            }
        }
     


    return 0;
}
