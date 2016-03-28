#ifndef _RECEIVER_MGR_H
#define _RECEIVER_MGR_H

#define IPC_GET_UDP_PORT_TRIALS 20

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <errno.h>
#include <errno.h>

#include "client.h"
#include "defs.h"

struct receiver_manager {
  pid_t receiver_pid;
  u_int16_t client_udp_port;

  int ctrl2rcvr_qid;
  char ctrl2rcvr_fname[MAX_FILE_NAME_LEN];
};

struct receiver_manager* create_receiver_manager();

#endif
