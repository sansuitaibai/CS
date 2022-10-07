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

ssize_t readn(int fd, void *buf, size_t count){
        int nleft = count;
        int ret = 0;
        char* bufp = (char*)buf;
        
        while(nleft > 0){
            ret = read(fd, bufp , nleft);
            if(ret == -1){
                if(errno == EINTR)
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
void activate_nonblock(int fd)
{
    int ret;
    int flags = fcntl(fd, F_GETFL);
    if(flags == -1)
        ERR_EXIT("fcntl");
    flags |= O_NONBLOCK;
    ret = fcntl(fd, F_SETFL, flags);
    if(ret == -1)
        ERR_EXIT("fcntl");
}

void handle_child(int sig){
    while(waitpid(-1, NULL , WNOHANG)){ }
}

int main(){
    signal(SIGCHLD, handle_child); 
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
   int ret = -1;
   int nread = 0;
   struct epoll_event event;
   memset(&event, 0 , sizeof(event));
   int eventfd  = epoll_create1(EPOLL_CLOEXEC);
   event.events = EPOLLIN | EPOLLET;
   event.data.fd = listenfd;
   epoll_ctl(eventfd, EPOLL_CTL_ADD, listenfd , &event);
   std::vector<struct epoll_event> events(16);
   char recvbuf[1024];
   memset(recvbuf , 0 , sizeof(recvbuf));
   int count = 0;
   while(1){
        nread = epoll_wait(eventfd, &*events.begin(),static_cast<int>(events.size()), -1 ); 
        if(nread < 0){
            if(errno ==  EINTR)
                continue;
            ERR_EXIT("epoll");
        }
        if(nread == 0)
            continue;
        if((size_t)nread == events.size())
            events.resize(events.size() *2);
        for(int i = 0 ; i < nread; ++i){
            
           if( events[0].data.fd  == listenfd ){
                connfd = accept(listenfd , (struct sockaddr*)&peeraddr ,&addrlen);
                if(connfd < 0){
                    ERR_EXIT("accept");
                }
                printf("连接成功: S(%s , %d) <-> C(%s , %d)\n", inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port),
                                                                inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
                activate_nonblock(connfd);
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = connfd;
                epoll_ctl(eventfd , EPOLL_CTL_ADD, connfd , &event);
                printf("连接个数 count: %d\n", ++count);
           }else{
                if(events[i].events & EPOLLIN){
                    connfd = events[i].data.fd;
                    if(connfd < 0)
                        continue;
                    addrlen = sizeof(peeraddr);
                    if((getpeername(connfd, (struct sockaddr*)&peeraddr , &addrlen)) < 0)
                        ERR_EXIT("getpeername");
                    ret = readline(connfd,recvbuf , sizeof(recvbuf));
                    if(ret == -1)
                        ERR_EXIT("readline");
                    if(ret == 0){
                        printf("通信结束：closed <- C(%s , %d)\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
                        event = events[i];
                        epoll_ctl(eventfd , EPOLL_CTL_DEL , connfd , &event);
                        close(connfd);
                    }
                   printf("C(%s , %d): ", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
                   fputs(recvbuf, stdout);
                   writen(connfd, recvbuf , strlen(recvbuf));
                }
            }
        }

   }
   
   close(listenfd);
    return 0;
}
