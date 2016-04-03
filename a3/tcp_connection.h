#ifndef _TCP_CONNECTION_H
#define _TCP_CONNECTION_H

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <malloc.h>

#define TCP_BUFFER_SIZE 640

#define TCP_RUN_OF_OF_MEM 0
#define TCP_ERROR_CANNOT_RESOLVE_HOST 1
#define TCP_CANNOT_BIND_TO_SOCKET 2
#define TCP_CANNOT_CONNECT 3
#define TCP_WRITE_ERROR 4
#define TCP_READ_ERROR 4

/*
 * Object represent a TCP connection
 */
struct tcp_connection {
  int sock;
};

struct tcp_connection* create_tcp_connection(const char* host_name,
    int port, int* nerror);
char* send_tcp_request(struct tcp_connection* tcp_con, char* data,
    u_int16_t data_size, u_int16_t* respond_size, int* nerror);
void close_tcp_connection(struct tcp_connection* tcp_con);

#endif
