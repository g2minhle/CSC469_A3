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

/*
 * Establish a udp connection given the host name and the udp port.
 *
 * Args:
 *    struct udp_connection* udp_con:
 *      The udp connection.
 *    const char* host_name:
 *      The server host name.
 *    int port:
 *      The server port.
 *    int* nerror:
 *      The error code if any.
 *
 * Return:
 *    struct udp_connection*:
 *      The new udp connection.
 *      If there is an error return NULL;
 */
struct udp_connection* create_udp_connection(const char* host_name, 
                                            int port,
                                            int* nerror);
                        
/*
 * Send a udp message throught the given udp_connection and get the respond.
 *
 * Arg:
 *    struct udp_connection* udp_con:
 *      The udp connection.
 *    char* data:
 *      The message content.
 *    u_int16_t data:
 *      The size of the message.
 *    u_int16_t* respond_size:
 *      The size of the respond
 *    int* nerror:
 *      The error code if any.
 *
 * Return:
 *    char*:
 *      The pointer to the repond. 
 *      If there is an error then we will return NULL.
 */
void send_udp_request(struct udp_connection* udp_con, 
                    char* data, 
                    u_int16_t data_size,
                    int* nerror);

/*
 * Close a udp connection.
 *
 * Agrs:
 *    struct udp_connection* udp_con:
 *      The udp connection.
 */
void close_udp_connection(struct udp_connection* udp_con);

#endif
