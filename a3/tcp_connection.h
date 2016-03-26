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

#define NO_ERROR 0
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

/*
 * Establish a TCP connection given the host name and the TCP port.
 *
 * Args:
 *    struct tcp_connection* tcp_con:
 *      The tcp connection.
 *    const char* host_name:
 *      The server host name.
 *    int port:
 *      The server port.
 */
int init_tcp_connection(struct tcp_connection* tcp_con, 
                        const char* host_name, 
                        int port);
                        
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
char* send_request(struct tcp_connection* tcp_con, 
                    char* data, 
                    size_t data_size,
                    int* nerror);

/*
 * Close a TCP connection.
 *
 * Agrs:
 *    struct tcp_connection* tcp_con:
 *      The TCP connection.
 */
void close_tcp_connection(struct tcp_connection* tcp_con);

#endif
