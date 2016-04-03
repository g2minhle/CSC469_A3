#ifndef _HTTP_CONNECTION_H
#define _HTTP_CONNECTION_H

#include "tcp_connection.h"

#define HTTP_RUN_OF_OUT_MEM 0

#define HTTP_MAX_URL_LEN 1024

/*
 * Object represent a TCP connection
 */
struct http_connection {
  struct tcp_connection* tcp_con;
};

struct http_connection* create_http_connection(const char* host_name,
    int port, int* nerror);
char* send_http_request(struct http_connection* http_con, char* url,
    u_int16_t* http_status, int* nerror);
void close_http_connection(struct http_connection* http_con);

#endif
