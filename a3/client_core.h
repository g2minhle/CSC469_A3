#ifndef _CLIENT_CORE_H
#define _CLIENT_CORE_H

#include <stdbool.h>
#include <stddef.h>

#include "client_to_server_sender.h"
#include "chatserver_manager.h"
#include "receiver_mgr.h"

/*
 * This struct contains and manages all the logic of client app client_main is 
 * just an interface that interact with this core struct
 */
struct client_core {
  char* member_name;
  u_int16_t member_id;
  struct client_to_server_sender* sender;
  struct location_server* location_server;
  struct receiver_communicator* receiver_communicator;
}; 
  
struct client_core* create_client_core(char* member_name,       
                                      char* server_host_name,
                                      u_int16_t server_tcp_port,
                                      u_int16_t server_udp_port);

void cli_core_room_list_request(struct client_core* cli_core);
void cli_core_member_list_request(struct client_core* cli_core, char* room_name);
void cli_core_switch_room_request(struct client_core* cli_core, char* room_name);
void cli_core_create_room_request(struct client_core* cli_core, char* room_name);
void cli_core_heart_beat(struct client_core* cli_core);
void cli_core_quit(struct client_core* cli_core);
void cli_core_send_chatmsg(struct client_core* cli_core, char* chat_message);


#endif
