#ifndef _CLIENT_TO_SERVER_SENDER_H
#define _CLIENT_TO_SERVER_SENDER_H

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
struct client_to_server_sender {
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
 *    struct client_to_server_sender*:
 *      The control request sender.
 */
struct client_to_server_sender* create_ctrl_request_sender();


char* send_register_request(struct client_to_server_sender* sender, char* member_name, u_int16_t udp_port, u_int16_t* member_id);

void send_room_list_request(struct client_to_server_sender* sender, u_int16_t member_id);
void send_member_list_request(struct client_to_server_sender* sender, u_int16_t member_id, char* room_name);
void send_switch_room_request(struct client_to_server_sender* sender, u_int16_t member_id, char* room_name);
void send_create_room_request(struct client_to_server_sender* sender, u_int16_t member_id, char* room_name);
void send_quit_request(struct client_to_server_sender* sender, u_int16_t member_id);
void send_heart_beat(struct client_to_server_sender* sender, u_int16_t member_id);

#endif
