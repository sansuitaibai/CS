#include<unistd.h>
#include<sys/select.h>
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


void do_client(int sockfd , struct sockaddr_in localaddr , struct sockaddr_in srvaddr){
    
     int maxfd = sockfd;
     fd_set rset;
     FD_ZERO(&rset);
     int stdinfd = fileno(stdin);
     if(stdinfd > sockfd)
         maxfd = stdinfd;
     int ret = 0;
     int nready = -1;
     char recvbuf[1024];
     char sendbuf[1024];
     memset(recvbuf , 0 , sizeof(recvbuf));
     memset(sendbuf , 0 , sizeof(sendbuf));
     int stdineof = 0;
     while(1){
        if(stdineof == 0)
            FD_SET(stdinfd, &rset);
        FD_SET(sockfd , &rset);
        nready = select(maxfd + 1 , &rset , NULL , NULL , NULL);
        if(nready < 0){
            ERR_EXIT("select");
        }else if(nready == 0){
            continue;
        }
        if(FD_ISSET(stdinfd , &rset)){
            char* retch = fgets(sendbuf , sizeof(sendbuf), stdin);
            if(retch == NULL){
                printf("通信结束：C(%s , %d) <-> S(%s , %d)\n", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port),
                                                                inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port));
                stdineof  = 1;
               // close(sockfd);
               //ERR_EXIT("fgets");   
               shutdown(sockfd , SHUT_WR); 
            }
            writen(sockfd, sendbuf , strlen(sendbuf));
            memset(sendbuf , 0 , sizeof(sendbuf));
            if(--nready <= 0){
                continue;
            }
        }
        if(FD_ISSET(sockfd, &rset)){
            ret = readline(sockfd , recvbuf , sizeof(recvbuf));
            if(ret < 0){
                ERR_EXIT("readn");
            }else if(ret == 0){
                printf("通信结束：C(%s , %d) <-> S(%s , %d)\n",inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port),
                                                               inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port));
                close(sockfd);
                exit(EXIT_SUCCESS);
            }
            printf("S(%s , %d)->: ", inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port));
            fputs(recvbuf , stdout);
            memset(recvbuf ,  0 , sizeof(recv_peek));
            if(--nready <= 0)
                continue;
        }

     }
}


int main(){
    int sockfd ;
    if((sockfd = socket(AF_INET , SOCK_STREAM, IPPROTO_TCP)) < 0)
        ERR_EXIT("socket");
    struct sockaddr_in srvaddr;
    struct sockaddr_in localaddr;
    memset(&srvaddr, 0 , sizeof(srvaddr));
    memset(&localaddr , 0 , sizeof(localaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(8181);
    srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    
    if((connect(sockfd , (struct sockaddr*)&srvaddr , sizeof(srvaddr))) < 0)
        ERR_EXIT("connect");
    socklen_t addrlen = sizeof(localaddr);
   if((getsockname(sockfd , (struct sockaddr*)&localaddr, &addrlen)) < 0)
       ERR_EXIT("getsockname");
   printf("连接成功：C(%s , %d) <-> S(%s , %d)\n", inet_ntoa(localaddr.sin_addr), localaddr.sin_port,
                                                   inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port)); 
    
   do_client(sockfd, localaddr , srvaddr);

    
    return 0;
}




































