#ifndef _udp_CONNECTION_H
#define _udp_CONNECTION_H

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <malloc.h>

#define UDP_BUFFER_SIZE 640

#define UDP_RUN_OF_OF_MEM 0
#define UDP_ERROR_CANNOT_RESOLVE_HOST 1
#define UDP_CANNOT_BIND_TO_SOCKET 2
#define UDP_CANNOT_CONNECT 3
#define UDP_WRITE_ERROR 4
#define UDP_NO_ERROR 4

/*
 * Object represent a udp connection
 */
struct udp_connection {
  int sock;
  struct sockaddr_in serveraddr;
};

struct udp_connection* create_udp_connection(const char* host_name, int port, int* nerror);
void send_udp_request(struct udp_connection* udp_con, char* data,
    u_int16_t data_size, int* nerror);
void close_udp_connection(struct udp_connection* udp_con);

#endif
