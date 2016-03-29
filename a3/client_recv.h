#ifndef _CLIENT_RECV_H
#define _CLIENT_RECV_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include "client.h"

static char *option_string = "f:";

struct client_receiver_context {
  /* vars for UDP socket connection */
  int udp_fd;
  int udp_port;

  /* For communication with chat client control process */
  int ctrl2rcvr_qid;
  char ctrl2rcvr_fname[MAX_FILE_NAME_LEN];
};

#endif
