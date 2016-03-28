#ifndef _CTRL_REQUEST_SENDER_H
#define _CTRL_REQUEST_SENDER_H

#include <malloc.h>
#include <pthread.h>
#include <stdio.h>

#include "defs.h"
#include "tcp_connection.h"
#include "location_server_manager.h"

/*
 * This struct sends and revices all control requests and 
 * attemp to restore connection if current connection is not alive
 */
struct ctrl_request_sender {
  pthread_mutex_t sender_lock;
  struct location_server_manager* loc_srv_mgr;
};


/*
 * Create control request sender
 *
 * Agrs:
 *    char* member_name:
 *      The user name
 *    int udp_port:
 *      The reciever udp port
 *    struct receiver_manager* receiver_mgr:
 *      The receiver manager
 *
 * Return:
 *    struct ctrl_request_sender*:
 *      The control request sender.
 */
struct ctrl_request_sender* create_ctrl_request_sender(char* member_name, 
                                                        int udp_port);


void send_register_request(struct ctrl_request_sender* sender,
                            char* member_name,
                            u_int16_t udp_port);

#endif
