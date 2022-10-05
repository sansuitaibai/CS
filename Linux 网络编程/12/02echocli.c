#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<signal.h>

#define ERR_EXIT(m) \
    do{ \
        perror(m); \
        exit(EXIT_FAILURE); \
    }while(0)

ssize_t readn(int fd, void *buf, size_t count){
        int nleft = count;
        int ret = 0;
        char* bufp = (char*)buf;
        
        while(nleft > 0){
            ret = read(fd, bufp , nleft);
            if(ret == -1){
                if(errno = EINTR)
                    continue;
                return -1;
            }
            if(ret == 0)
                return count - nleft;
            bufp += ret;
            nleft -= ret;            
        }
        return count - nleft;
}

ssize_t writen(int fd, const void *buf, size_t count){
        int ret = 0;
        int nleft = count;
        char* bufp = (char*)buf;
        while(nleft > 0){
            ret = write(fd, bufp , nleft);
            if(ret ==-1){
                if(errno == EINTR)
                    continue;
                return -1;
            }
            if(ret == 0)
                continue;
           bufp += ret;
           nleft -= ret;
        }
        return count - nleft;
}


ssize_t recv_peek(int sockfd, void *buf, size_t len){
    int ret = 0;
    while(1){
        ret = recv(sockfd , buf , len, MSG_PEEK);
        if(ret == -1 && errno == EINTR)
            continue;
        return ret;
    }

}


ssize_t readline(int sockfd , void* buf , size_t maxlen){
    int ret = 0;
    char* bufp = (char*)buf;
    size_t nleft = maxlen;
    int nread = 0;
    int i ;
    while(1){
        ret = recv_peek(sockfd, bufp , nleft);
        if(ret < 0){
            return -1;
        }else if(ret == 0){
            return ret;
        }
        nread = ret;
        if(nread >nleft)
            exit(EXIT_FAILURE);
        for(i = 0 ; i < nread ; ++i){
            if(bufp[i] == '\n'){
                ret = readn(sockfd , bufp , i + 1);
                if(ret != i + 1)
                    exit(EXIT_FAILURE);
                return maxlen - nleft + ret;
            }
            

        }
       nleft -= nread;
       
       ret = readn(sockfd , bufp , nread);
       if(ret != nread)
           exit(EXIT_FAILURE);
       bufp += nread;

    }
    return -1; 

}


void echo_srv(int connfd , struct sockaddr_in srvaddr , struct sockaddr_in peeraddr ){
       if(connfd < 0)
           ERR_EXIT("echo_srv");
       char recvbuf[1024];
       int ret = 0;
       printf("连接成功：S(%s , %d) <-> C(%s , %d)\n",  inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port),
                                                        inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
       memset(recvbuf , 0 , sizeof(recvbuf));
       while(1){
            ret = readline(connfd , recvbuf , sizeof(recvbuf));
            if(ret < 0){
                ERR_EXIT("readline");
            }
            if(ret == 0){
               printf("通信结束：S(%s , %d) <-> C(%s , %d)\n", inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port),
                                                                inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
               close(connfd);
               exit(EXIT_SUCCESS);
            }
            printf("C(%s , %d) -> S(%s , %d): ", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port),
                                                 inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port));
            fputs(recvbuf , stdout);
            ret = writen(connfd , recvbuf , strlen(recvbuf));
            if(ret < 0)
                ERR_EXIT("writen");
            memset(recvbuf , 0 , sizeof(recvbuf));
       }
}


void handle_sigpipe(int sig){
    printf("产生sigpipe的信号： %d\n",sig );
}

int main(){
    int sockfd ;
    signal(SIGPIPE , handle_sigpipe);
    if((sockfd = socket(AF_INET , SOCK_STREAM, IPPROTO_TCP)) < 0)
        ERR_EXIT("socket");
    struct sockaddr_in srvaddr;
    struct sockaddr_in localaddr;
    memset(&srvaddr, 0 , sizeof(srvaddr));
    memset(&localaddr , 0 , sizeof(localaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(8181);
    srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    
    int ret ; 
    if((connect(sockfd , (struct sockaddr*)&srvaddr , sizeof(srvaddr))) < 0)
        ERR_EXIT("connect");
    socklen_t addrlen = sizeof(localaddr);
   if((getsockname(sockfd , (struct sockaddr*)&localaddr, &addrlen)) < 0)
       ERR_EXIT("getsockname");
   printf("连接成功：C(%s , %d) <-> S(%s , %d)\n", inet_ntoa(localaddr.sin_addr), localaddr.sin_port,
                                                   inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port)); 
    char recvbuf[1024];
    char sendbuf[1024];
    while((fgets( sendbuf , sizeof(sendbuf), stdin )) != NULL){
        
        writen(sockfd ,sendbuf , 1);
        writen(sockfd ,sendbuf + 1 , strlen(sendbuf) -1);
        if(ret < 0)
            ERR_EXIT("writen");
        ret = readline(sockfd, recvbuf , sizeof(recvbuf));
        if(ret == 0){
            printf("通信结束：C(%s , %d) <-> S(%s , %d)\n", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port),
                                                            inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port));
            close(sockfd);
            exit(EXIT_SUCCESS);
        }
        printf("S(%s , %d): ", inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port));
        fputs(recvbuf , stdout);
        memset(recvbuf , 0 , sizeof(recvbuf));
        memset(sendbuf, 0 , sizeof(sendbuf));

    }

    return 0;
}




































