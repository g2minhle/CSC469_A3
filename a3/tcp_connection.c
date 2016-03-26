#include "tcp_connection.h"

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
 *    int* nerror:
 *      The error code if any.
 *
 * Return:
 *    struct tcp_connection*:
 *      The new tcp connection.
 *      If there is an error return NULL;
 */
struct tcp_connection* create_tcp_connection(const char* host_name, 
                                            int port,
                                            int* nerror){
  struct tcp_connection* tcp_con = (struct tcp_connection*)malloc(
    sizeof(struct tcp_connection)
  );
  
  struct sockaddr_in addr;
  struct hostent* host_entry;

  /*get ip address of the host*/
  host_entry = gethostbyname(host_name);

  if (!host_entry) {
    *nerror = TCP_ERROR_CANNOT_RESOLVE_HOST;
    return NULL;
  } 

  memset(&addr,0,sizeof(addr));
  addr.sin_addr=*(struct in_addr *) host_entry->h_addr_list[0];
  addr.sin_family=AF_INET;
  addr.sin_port=htons(port);

  /* open socket */
  tcp_con->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if ( tcp_con->sock < 0 ) {
    *nerror = TCP_CANNOT_BIND_TO_SOCKET;
    return NULL;
  }

  if( connect(tcp_con->sock, (struct sockaddr*)&addr, sizeof(addr)) < 0 ) {
    *nerror = TCP_CANNOT_CONNECT;
    return NULL;
  }

  return tcp_con;
}

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
 *    int* nerror:
 *      The error code if any.
 *
 * Return:
 *    char*:
 *      The pointer to the repond. 
 *      If there is an error then we will return NULL.
 */
char* send_tcp_request(struct tcp_connection* tcp_con, 
                    char* data, 
                    size_t data_size,
                    int* nerror) {
  int io_result = write(tcp_con->sock, data, data_size);
  if( io_result < 0 ) {
    *nerror = TCP_WRITE_ERROR;
    return NULL;
  }
  
  char* buf = (char*) malloc(TCP_BUFFER_SIZE);
  size_t buf_used = 0;
  
  while(buf_used < TCP_BUFFER_SIZE) {
    io_result  = read(tcp_con->sock, &buf[buf_used], TCP_BUFFER_SIZE - buf_used);
    if(io_result  < 0) {
      *nerror = TCP_READ_ERROR;
      free(buf);
      return NULL;
    }
    else if (io_result  == 0) {
      // No more data to read
      break;
    }
    else {
      buf_used +=  io_result ;
    }
  }
  return buf;  
}

/*
 * Close a TCP connection.
 *
 * Agrs:
 *    struct tcp_connection* tcp_con:
 *      The TCP connection.
 */
void close_tcp_connection(struct tcp_connection* tcp_con) {
  close(tcp_con->sock);
  free(tcp_con);
}
