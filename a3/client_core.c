#include "client_core.h"

struct client_core* create_client_core(char* member_name,   
                                        char* server_host_name,
                                        u_int16_t server_tcp_port,
                                        u_int16_t server_udp_port) { 

  struct receiver_manager* receiver_mgr = create_receiver_manager();
  
  if (receiver_mgr == NULL) return NULL;
  
  struct client_to_server_sender* client_to_server_sender = 
    create_client_to_server_sender(server_host_name, server_tcp_port, server_udp_port);
  
  if (client_to_server_sender == NULL) {
    shutdown_receiver(receiver_mgr);
    destroy_receiver_manager(receiver_mgr);
    return NULL;
  }
  
  struct client_core* cli_core = (struct client_core*)malloc(sizeof(struct client_core));
  cli_core->member_name = member_name;
  char* error_msg = send_register_request(client_to_server_sender, 
                                          cli_core->member_name,
                                          receiver_mgr->client_udp_port,
                                          &cli_core->member_id);
  if(error_msg) {
    fprintf(stderr, "%s \n", error_msg);
    shutdown_receiver(receiver_mgr);
    free(receiver_mgr);
    free(error_msg);
    destroy_client_to_server_sender(client_to_server_sender);
    return NULL;
  }       

  return cli_core;    
}

void cli_core_room_list_request(struct client_core* cli_core) {
  send_room_list_request(cli_core->sender, cli_core->member_id);
}

void cli_core_member_list_request(struct client_core* cli_core, char* room_name) {
  send_member_list_request(cli_core->sender, cli_core->member_id, room_name);
}

void cli_core_switch_room_request(struct client_core* cli_core, char* room_name) {
  send_switch_room_request(cli_core->sender, cli_core->member_id, room_name);
}

void cli_core_create_room_request(struct client_core* cli_core, char* room_name) {
  send_create_room_request(cli_core->sender, cli_core->member_id, room_name);
}

void cli_core_heart_beat(struct client_core* cli_core) {
  send_heart_beat(cli_core->sender,cli_core->member_id);
}

void cli_core_quit(struct client_core* cli_core){
  send_quit_request(cli_core->sender, cli_core->member_id);
}

void cli_core_send_chatmsg(struct client_core* cli_core, char* chat_message){
}
