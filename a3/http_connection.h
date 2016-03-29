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

/*
 * Establish a HTTP connection given the host name and the HTTP port.
 *
 * Args:
 *    struct tcp_connection* tcp_con:
 *      The tcp connection.
 *    const char* host_name:
 *      The server host name.
 *    int port:
 *      The server port.
 *    int* nerror:
 *      The error code if any.
 *
 * Return:
 *    struct http_connection*:
 *      The new http connection.
 *      Return NULL if there is an error
 */
struct http_connection* create_http_connection(const char* host_name, 
                                              int port,
                                              int* nerror);
                        
/*
 * Send a tcp message throught the given tcp_connection and get the respond.
 *
 * Arg:
 *    struct tcp_connection* tcp_con:
 *      The tcp connection.
 *    char* data:
 *      The message content.
 *    size_t data:
 *      The size of the message.
 *
 * Return:
 *    char*:
 *      The pointer to the repond. 
 *      If there is an error then we will return NULL.
 */
char* send_http_request(struct http_connection* http_con, char* url, u_int16_t* http_status, int* nerror);

/*
 * Close a HTTP connection.
 *
 * Agrs:
 *    struct http_connection* http_con:
 *      The HTTP connection.
 */
void close_http_connection(struct http_connection* http_con);

#endif
