#include "client_core.h"

pthread_t hb_thread;

struct client_core* create_client_core(char* member_name,   
                                        char* server_host_name,
                                        u_int16_t server_tcp_port,
                                        u_int16_t server_udp_port) { 

  struct receiver_manager* receiver_mgr = create_receiver_manager();
  
  if (receiver_mgr == NULL) return NULL;
  
  struct client_core* cli_core = (struct client_core*)malloc(sizeof(struct client_core));
  
  if (cli_core == NULL) {
    destroy_receiver_manager(receiver_mgr);
    return NULL;
  }
  
  cli_core->member_name = member_name;
  cli_core->receiver_manager = receiver_mgr;
  
  struct client_to_server_sender* client_to_server_sender = 
    create_client_to_server_sender(server_host_name, server_tcp_port, server_udp_port);
  
  if (client_to_server_sender == NULL) {
    free(cli_core);
    destroy_receiver_manager(receiver_mgr);
    return NULL;
  }

  client_to_server_sender->cli_core = cli_core;
  cli_core->sender = client_to_server_sender;

  char* error_msg = send_register_request(client_to_server_sender, 
                                          cli_core->member_name,
                                          receiver_mgr->client_udp_port,
                                          &cli_core->member_id);
                                                                                      
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

void start_hb_thread(struct client_core* cli_core)
{
  pthread_attr_t attr;
  if(pthread_attr_init(&attr)){
    printf("error!");
  }
  if (pthread_create(&hb_thread, &attr, cli_core_heart_beat, (void*)cli_core))
  {
    perror("heartbeat pthread_create");
  }
  
}

void* cli_core_heart_beat(void* param) {
  struct client_core* cli_core = (struct client_core*) param;
  while (1){
    send_heart_beat(cli_core->sender,cli_core->member_id);
    sleep(5);
  }
  return NULL;
}

void cli_core_quit(struct client_core* cli_core){
  pthread_cancel(hb_thread);
  pthread_join(hb_thread, NULL);
  send_quit_request(cli_core->sender, cli_core->member_id);
}

void cli_core_send_chatmsg(struct client_core* cli_core, char* chat_message){
  send_chat_msg (cli_core->sender, chat_message, cli_core->member_id);
}
