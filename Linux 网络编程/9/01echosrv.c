#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<signal.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>


#define ERR_EXIT(m)\
    do{ \
        perror(m); \
        exit(EXIT_FAILURE); \
    }while(0)

#define BUF_SIZE 10

ssize_t readn(int fd, void *buf, size_t count){

    if(buf == NULL || count < 0)
        ERR_EXIT("readn arg");
    char* bufp = (char*)buf;
    size_t nleft = count ; 
    ssize_t ret ;
    while(nleft > 0){
        ret  = read(fd , bufp , nleft);
        if(ret == -1){
            if(errno == EINTR)
                continue;
            return -1;
        }else if(ret == 0)
            return count - nleft;    
        bufp += ret;
        nleft -= ret;
    }
    return count - nleft;
}

ssize_t writen(int fd, const void *buf, size_t count){
    if(buf == NULL || count <0)
        ERR_EXIT("writen arg");
    ssize_t ret;
    size_t nleft = count;
    char* bufp = (char*)buf;
    while(nleft > 0){
        ret = write(fd , bufp, nleft);
        if(ret == -1){
            if(errno == EINTR)
                continue;
            return -1;
        }else if(ret == 0)
            continue;
       bufp += ret;
       nleft -= ret;
    }
    return count - nleft;

}

void do_service(int connfd , struct sockaddr_in srvaddr , struct sockaddr_in peeraddr){
    char recvbuf[BUF_SIZE];
    int ret = 0;
    memset(recvbuf , 0 , sizeof(recvbuf));
    while(1){
        ret = readn(connfd , recvbuf , sizeof(recvbuf));
        if(ret == -1)
            ERR_EXIT("readn");
        if(ret == 0){
            printf("close client(%s , %d)\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
            close(connfd);
            break;
        }

        printf("client(%s , %d) -> service(%s , %d): ", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port),
                                                        inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port));
        fputs(recvbuf , stdout);
        if(ret == BUF_SIZE)
            printf("\n");
        writen(connfd , recvbuf , sizeof(recvbuf));
    }
}

int main(){

    int listenfd;
    if((listenfd = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP)) < 0 )
        ERR_EXIT("listen");
    int on = 1;
    if((setsockopt(listenfd , SOL_SOCKET , SO_REUSEADDR , &on  ,sizeof(on))) < 0)
        ERR_EXIT("setsockopt");
    struct sockaddr_in srvaddr ;
    struct sockaddr_in peeraddr;
    int connfd;
    memset(&srvaddr , 0 , sizeof(srvaddr));
    memset(&peeraddr , 0 , sizeof(peeraddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(8787);
    srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(( bind(listenfd , (struct sockaddr*)&srvaddr , sizeof(srvaddr) ) ) < 0)
        ERR_EXIT("bind");
    if(listen(listenfd , SOMAXCONN) < 0)
        ERR_EXIT("listen");
    pid_t pid ;
    socklen_t addrlen ; 
    while(1){
        if((connfd = accept(listenfd , (struct sockaddr*)&peeraddr , &addrlen)) < 0)
            ERR_EXIT("accept");
        printf("连接成功： service(%s , %d) <-> client(%s , %d)\n", inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port),
                                                                    inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
        pid = fork();
        if(pid == -1){
            ERR_EXIT("fork");
        }else if(pid == 0){
            close(listenfd);
            do_service(connfd , srvaddr , peeraddr);
            exit(EXIT_SUCCESS);
        }else{
            close(connfd);
        }
    }

 return 0;
}
































