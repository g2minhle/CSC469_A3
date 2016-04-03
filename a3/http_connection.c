#include "http_connection.h"

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
struct http_connection* create_http_connection(const char* host_name, int port, int* nerror)
{
  struct http_connection* http_con = (struct http_connection*)malloc(sizeof(struct http_connection));

  if (http_con == NULL)
  {
    *nerror = HTTP_RUN_OF_OUT_MEM;
    return NULL;
  }

  http_con->tcp_con = create_tcp_connection(host_name, port, nerror);
  if (http_con->tcp_con == NULL)
  {
    return NULL;
  }

  return http_con;
}



/*
 * Send a tcp message through the given tcp_connection and get the response.
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
char* send_http_request(struct http_connection* http_con, char* url, u_int16_t* http_status, int* nerror)
{
  char* http_request_template = "GET %s HTTP/1.0\r\n\r\n";

  u_int16_t response_size;
  u_int16_t request_len = strlen(http_request_template) + strnlen(url, HTTP_MAX_URL_LEN) + 1;

  char* request = (char*) malloc(request_len);
  sprintf (request, http_request_template, url);

  char* raw_response = send_tcp_request(http_con->tcp_con, request, request_len, &response_size, nerror);
  if (raw_response == NULL)
    return NULL;

  char* previous = NULL;
  char* token = strtok(raw_response, "\r\n");
  sscanf (token, "%*s %hu %*s", http_status);
  while( token != NULL )
  {
    previous = token;
    token = strtok(NULL, "\r\n");
  }

  char* response = (char*) malloc(strlen(previous));
  strcpy(response, previous);

  free(request);
  free(raw_response);
  return response;
}

/*
 * Close a HTTP connection.
 *
 * Agrs:
 *    struct http_connection* http_con:
 *      The HTTP connection.
 */
void close_http_connection(struct http_connection* http_con)
{
  close_tcp_connection(http_con->tcp_con);
  free(http_con);
}

