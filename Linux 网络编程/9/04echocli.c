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

struct packet{

    int len ;
    char buf[1024];


};

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
        writen(connfd , recvbuf , sizeof(recvbuf));
    }
}

int main(){

    int sockfd;
    if((sockfd = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP)) < 0 )
        ERR_EXIT("listen");
    struct sockaddr_in srvaddr ;
    struct sockaddr_in localaddr;
    memset(&srvaddr , 0 , sizeof(srvaddr));
    memset(&localaddr , 0 , sizeof(localaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(8787);
    srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    socklen_t addrlen ; 
    if((connect(sockfd ,(struct sockaddr*)&srvaddr , sizeof(srvaddr))) < 0)
        ERR_EXIT("connect");
    if((getsockname(sockfd , (struct sockaddr*)&localaddr, &addrlen)) < 0)
        ERR_EXIT("getsockname");
    printf("连接成功：client(%s , %d) <-> service(%s , %d)\n", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port),
                                                               inet_ntoa(srvaddr.sin_addr),   ntohs(srvaddr.sin_port));
    struct packet recvbuf;
    struct packet sendbuf;
    memset(&sendbuf , 0 , sizeof(sendbuf));
    memset(&recvbuf , 0 , sizeof(recvbuf));
    int ret = 0;
    int n =0;
    while((fgets(sendbuf.buf , sizeof(sendbuf.buf), stdin)) != NULL){
        n = strlen(sendbuf.buf);
        sendbuf.len = htonl(n);
        writen(sockfd , &sendbuf , n + 4);
        ret = readn(sockfd, &recvbuf.len ,  4);
        if(ret == -1)
            ERR_EXIT("readn");
        if(ret < 4){
            printf("close service(%s , %d)\n", inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port));
            close(sockfd);
            exit(EXIT_SUCCESS);
        }
        n = ntohl(recvbuf.len);
        ret = readn(sockfd , recvbuf.buf, n );
        if(ret == -1)
            ERR_EXIT("readn");
        if(ret < n){
            printf("close service(%s , %d)\n", inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port));
            close(sockfd);
            exit(EXIT_SUCCESS);
        }
        
        printf("service(%s , %d)->client(%s , %d): ",inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port),
                                                     inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port));
        fputs(recvbuf.buf, stdout);
        memset(&sendbuf, 0 , sizeof(sendbuf));
        memset(&recvbuf , 0 , sizeof(recvbuf));
    }

    return 0;
}
































