#ifndef SOCK_H
#define SOCK_H

#include <string>

using namespace std;

extern "C"{
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h> 
#include <linux/types.h>
}

#define BACKLOG 1024
#define INFTIM -1

/**
 * 建立主动TCP套接口
 * 连接到指定的服务器host:port
 * 成功返回套接字，失败返回-1
 */
int tcp_connect(const char* host, const char* port);

/**
 * 建立被动TCP服务套接口
 * 绑定指定端口port
 * 成功返回套接字，失败返回-1
 */
int tcp_listen(const char* port);

/**
 * 建立UDP套接口
 * 绑定端口(port)
 * 成功返回套接字，失败返回-1
 */
int udp_server(const char* port);

/**
 * 建立UDP套接口
 * 不指定端口
 * 成功返回套接字，失败返回-1
 */
int udp_client();

/**
 * UDP连接指定的主机(host)与端口(port)
 * 成功返回套接字，失败返回-1
 */
int udp_connect(const char* host, const char* port);

/**
 * 往套接字fd中写入以vptr为首地址的n个字节
 * 成功返回写入的字节数，失败返回-1
 */
ssize_t writen(int fd, const void *vptr, size_t n);

/**
 * 从套接字fd中读取n个字节，存入以vptr为首地址的缓存中
 * 成功返回读取的字节数，失败返回-1
 */
ssize_t readn(int fd, void *vptr, size_t n);

#endif
