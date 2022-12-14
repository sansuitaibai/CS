#include<unistd.h>
#include<sys/select.h>
#include<sys/wait.h>
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
       printf("???????????????S(%s , %d) <-> C(%s , %d)\n",  inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port),
                                                        inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
       memset(recvbuf , 0 , sizeof(recvbuf));
       while(1){
            ret = readline(connfd , recvbuf , sizeof(recvbuf));
            if(ret < 0){
                ERR_EXIT("readline");
            }
            if(ret == 0){
               printf("???????????????S(%s , %d) <-> C(%s , %d)\n", inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port),
                                                                inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
               close(connfd);
               exit(EXIT_SUCCESS);
            }
            printf("C(%s , %d) -> S(%s , %d): ", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port),
                                                 inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port));
            fputs(recvbuf , stdout);
            sleep(4);
            ret = writen(connfd , recvbuf , strlen(recvbuf));
            if(ret < 0)
                ERR_EXIT("writen");
            memset(recvbuf , 0 , sizeof(recvbuf));
       }
}

void handle_child(int sig){
    while(waitpid(-1, NULL , WNOHANG)){ }
}
void handle_sigpipe(int sig){
    printf("SIGPIPE: sig = %d\n", sig);
    
}

int main(){
    signal(SIGCHLD, handle_child); 
    signal(SIGPIPE, handle_sigpipe);
    int listenfd ;
    if((listenfd = socket(AF_INET , SOCK_STREAM, IPPROTO_TCP)) < 0)
        ERR_EXIT("socket");
    int on = 1;
    if((setsockopt(listenfd , SOL_SOCKET , SO_REUSEADDR , &on , sizeof(on))) < 0)
        ERR_EXIT("setsockopt");
    struct sockaddr_in srvaddr;
    struct sockaddr_in peeraddr;
    memset(&srvaddr, 0 , sizeof(srvaddr));
    memset(&peeraddr , 0 , sizeof(peeraddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(8181);
    srvaddr.sin_addr.s_addr = htons(INADDR_ANY);
   
    if((bind(listenfd , (struct sockaddr*)&srvaddr, sizeof(srvaddr))) < 0){
        ERR_EXIT("bind");
    }
    if((listen(listenfd, SOMAXCONN)) < 0)
        ERR_EXIT("listen");
   int connfd;
   socklen_t addrlen ;
   fd_set allset;
   fd_set rset;
   FD_ZERO(&allset);
   FD_ZERO(&rset);
   int maxfd = listenfd; 
   FD_SET(listenfd, &allset);
   int ret = -1;
   int nread = 0;
   int client[FD_SETSIZE];
   for(int i = 0 ; i < FD_SETSIZE ; ++i){
        client[i] = -1;
   }
   char recvbuf[1024];
   memset(recvbuf , 0 , sizeof(recvbuf));
   while(1){
        rset = allset;
        nread = select(maxfd + 1 , &rset , NULL , NULL , NULL); 
        if(nread < 0)
            ERR_EXIT("select");
        if(nread == 0)
            continue;
       if(FD_ISSET(listenfd, &rset)){
            connfd = accept(listenfd , (struct sockaddr*)&peeraddr ,&addrlen);
            if(connfd < 0)
                ERR_EXIT("accept");
            printf("????????????: S(%s , %d) <-> C(%s , %d)\n", inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port),
                                                            inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
            int i = 0;
            for(  ; i < FD_SETSIZE ; ++i){
                if(client[i] == -1){
                    client[i] = connfd;
                    break;
                }
            }
            if(i == FD_SETSIZE ){
                ERR_EXIT("FD_SETSIZE");
            }
            if(connfd > maxfd)
                maxfd = connfd;
            FD_SET(connfd , &allset);
            if(--nread <= 0)
                continue;
       }
       for(int i = 0 ; i < FD_SETSIZE; ++i){
            if(client[i] == -1 )
                continue;
            if(FD_ISSET(client[i] , &rset)){
                if((getpeername(client[i], (struct sockaddr*)&peeraddr , &addrlen)) < 0)
                    printf("getpeername is failure\n");
                ret = readline(client[i],recvbuf , sizeof(recvbuf));
                if(ret == -1)
                    ERR_EXIT("readline");
                if(ret == 0){
                    printf("???????????????closed <- C(%s , %d)\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
                    FD_CLR(client[i] , &allset);
                    close(client[i]);
                    maxfd --;
                    client[i] = -1;
                }
               printf("C(%s , %d): ", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
               fputs(recvbuf, stdout);
               sleep(4);
               writen(client[i], recvbuf , strlen(recvbuf));
               if(--nread <= 0)
                   break;
            }
       }

   }
   
    return 0;
}




































