#include "udp_connection.h"

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
                                            int* nerror){
  struct udp_connection* udp_con = (struct udp_connection*)malloc(
    sizeof(struct udp_connection)
  );
  
  if (udp_con == NULL) {
    *nerror = UDP_RUN_OF_OF_MEM;
    return NULL; 
  }
  
  struct hostent* host_entry;

  /*get ip address of the host*/
  host_entry = gethostbyname(host_name);

  if (!host_entry) {
    free(udp_con);
    *nerror = UDP_ERROR_CANNOT_RESOLVE_HOST;
    return NULL;
  } 

  bzero((char *) &(udp_con->serveraddr), sizeof(udp_con->serveraddr));
  udp_con->serveraddr.sin_addr=*(struct in_addr *) host_entry->h_addr_list[0];
  udp_con->serveraddr.sin_family=AF_INET;
  udp_con->serveraddr.sin_port=htons(port);

  /* open socket */
  udp_con->sock = socket(AF_INET, SOCK_DGRAM, 0);
  if ( udp_con->sock < 0 ) {
    free(udp_con);
    *nerror = UDP_CANNOT_BIND_TO_SOCKET;
    return NULL;
  }

  return udp_con;
}

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
                    int* nerror) {
  int serverlen = sizeof(udp_con->serveraddr);
  int io_result = sendto(udp_con->sock, data, data_size, 0, (const struct sockaddr *)&(udp_con->serveraddr), serverlen);
  if( io_result < 0 ) {
    *nerror = UDP_WRITE_ERROR;   
  } else {
    *nerror = UDP_NO_ERROR;   
  }
}

/*
 * Close a udp connection.
 *
 * Agrs:
 *    struct udp_connection* udp_con:
 *      The udp connection.
 */
void close_udp_connection(struct udp_connection* udp_con) {
  close(udp_con->sock);
  free(udp_con);
}
