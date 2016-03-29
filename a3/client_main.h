#ifndef CLIENT_MAIN_H
#define CLIENT_MAIN_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <netdb.h>

#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "client.h"
#include "client_core.h"
#include "client_to_server_sender.h"

#include "defs.h"

#define MAX_MSGDATA (MAX_MSG_LEN - sizeof(struct chat_msghdr))  

static char *option_string = "h:t:u:n:";

struct chatclient_context {
  char server_host_name[MAX_HOST_NAME_LEN];
  /* For control messages */
  u_int16_t server_tcp_port;
  /* For chat messages */
  u_int16_t server_udp_port; 
  char member_name[MAX_MEMBER_NAME_LEN];
  u_int16_t client_udp_port; 
};

#endif
