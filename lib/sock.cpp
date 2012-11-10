#include "sock.h"
#include "error.h"

#include <iostream>

using namespace std;

int 
tcp_connect(const char* host, const char* port){
    int sockfd;     
    const int on = 1;
    struct sockaddr_in servaddr;

    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
        err_ret("socket error");
        return -1;
    }
    
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0){
        err_ret("setsockopt error");
        return -1;
    }

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(port));
    if(inet_pton(AF_INET,host,&servaddr.sin_addr) <= 0){
        err_ret("inet_pton error for %s",host);
        return -1;
    }
    if(connect(sockfd,(const sockaddr*)&servaddr,sizeof(servaddr)) < 0){
    //    cout<<"can not connect to "<<host<<endl;
        sleep(15);
        if(connect(sockfd,(const sockaddr*)&servaddr,sizeof(servaddr)) < 0){
      //      cout<<"can not connect to "<<host<<endl;
            sleep(250);
            if(connect(sockfd,(const sockaddr*)&servaddr,sizeof(servaddr)) < 0){
                err_ret("tcp connect error for %s, %s",host,port);
                return -1;
            }
        }
    }
	//cout<<"connected ..."<<endl;
    return sockfd;
}

int 
tcp_listen(const char* port){
    int listenfd,backlog;
    const int on = 1;
    struct sockaddr_in servaddr;
    char* ptr;

    if((listenfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
        err_ret("socket error");
        return -1;
    }

    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0){
        err_ret("setsockopt error");
        return -1;
    }

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(port));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listenfd,(const sockaddr*)&servaddr,sizeof(servaddr)) < 0){
        err_ret("bind error for port %s",port);
        return -1;
    }


    /*can override 2nd argument with environment variable */
    if((ptr = getenv("LISTENQ")) != NULL){
        backlog = atoi(ptr);
    }else{
        /* BACKLOG defined in sock.h */
        backlog = BACKLOG;
    }

    if(listen(listenfd,backlog) < 0){
        err_ret("listen error");
        return -1;
    }

    return listenfd;
}

int 
udp_server(const char* port){
    int sockfd;
    const int on = 1;
    struct sockaddr_in servaddr;

    if((sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0){
        err_ret("socket error");
        return -1;
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0){
        err_ret("setsockopt error");
        return -1;
    }

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(port));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sockfd,(const sockaddr*)&servaddr,sizeof(servaddr)) < 0){
        err_ret("bind error for port %s",port);
        return -1;
    }

    return sockfd;
}

int 
udp_client(){
    int sockfd;

    if((sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0){
        err_ret("socket error");
        return -1;
    }

    return sockfd;
}

int 
udp_connect(const char* host, const char* port){
    int sockfd;
    struct sockaddr_in servaddr;

    if((sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0){
        err_ret("socket error");
        return -1;
    }

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(port));
    if(inet_pton(AF_INET,host,&servaddr.sin_addr) <= 0){
        err_ret("inet_pton error for host %s",host);
        return -1;
    }

    if(connect(sockfd,(const sockaddr*)&servaddr,sizeof(servaddr)) < 0){
        err_ret("udp connect error for %s, %s",host,port);
        return -1;
    }

    return sockfd;
}

ssize_t						/* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
    size_t		nleft;
    ssize_t		nwritten;
    const char	*ptr;

    ptr = (const char*)vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
            if (errno == EINTR)
                nwritten = 0;		/* and call write() again */
            else
                return(-1);			/* error */
        }

        nleft -= nwritten;
        ptr   += nwritten;
    }
    return(n);
}


ssize_t						/* Read "n" bytes from a descriptor. */
readn(int fd, void *vptr, size_t n)
{
    size_t	nleft;
    ssize_t	nread;
    char	*ptr;

    ptr = (char *)vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;		/* and call read() again */
            else
                return(-1);
        } else if (nread == 0)
            break;				/* EOF */

        nleft -= nread;
        ptr   += nread;
    }
    return(n - nleft);		/* return >= 0 */
}
