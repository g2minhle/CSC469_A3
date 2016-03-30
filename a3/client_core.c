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
    destroy_receiver_manager(receiver_mgr);
    return NULL;
  }
  
  struct client_core* cli_core = (struct client_core*)malloc(sizeof(struct client_core));
  cli_core->member_name = member_name;
  char* error_msg = send_register_request(client_to_server_sender, 
                                          cli_core->member_name,
                                          receiver_mgr->client_udp_port,
                                          &cli_core->member_id);
                                          
  cli_core->sender = client_to_server_sender;
  cli_core->receiver_manager = receiver_mgr;
                                            
  if(error_msg) {
    fprintf(stderr, "Registration Failed: %s \n", error_msg);
    free(error_msg);
    cli_core_shutdown(cli_core);
    return NULL;
  }       
      
  return cli_core;    
}

void cli_core_shutdown(struct client_core* cli_core) {
  destroy_client_to_server_sender(cli_core->sender);
  destroy_receiver_manager(cli_core->receiver_manager);
  free(cli_core);  
}

void cli_core_room_list_request(struct client_core* cli_core) {
  receiver_printf(cli_core->receiver_manager, "Sending room list request");
  char* respond = send_room_list_request(cli_core->sender, cli_core->member_id);
  receiver_printf(cli_core->receiver_manager, respond);
  free(respond);
}

void cli_core_member_list_request(struct client_core* cli_core, char* room_name) {
  receiver_printf(cli_core->receiver_manager, "Sending member list request");
  char* respond = send_member_list_request(cli_core->sender, cli_core->member_id, room_name);
  receiver_printf(cli_core->receiver_manager, respond);
  free(respond);
}

void cli_core_switch_room_request(struct client_core* cli_core, char* room_name) {
  receiver_printf(cli_core->receiver_manager, "Sending switch room request");
  char* respond = send_switch_room_request(cli_core->sender, cli_core->member_id, room_name);
  receiver_printf(cli_core->receiver_manager, respond);
  free(respond);
}

void cli_core_create_room_request(struct client_core* cli_core, char* room_name) {
  receiver_printf(cli_core->receiver_manager, "Sending create request");
  char* respond = send_create_room_request(cli_core->sender, cli_core->member_id, room_name);
  receiver_printf(cli_core->receiver_manager, respond);
  free(respond);
}

void cli_core_heart_beat(struct client_core* cli_core) {
  send_heart_beat(cli_core->sender,cli_core->member_id);
}

void cli_core_quit(struct client_core* cli_core){
  send_quit_request(cli_core->sender, cli_core->member_id);
}

void cli_core_send_chatmsg(struct client_core* cli_core, char* chat_message){
  send_chat_msg (cli_core->sender, cli_core->member_name, chat_message);
}
