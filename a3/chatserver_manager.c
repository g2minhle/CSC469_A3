#include "chatserver_manager.h"

// TODO #feature: Enable this
//struct location_server_manager* create_location_server_manager(struct receiver_manager* receiver_mgr) {
struct chatserver_manager* create_chatserver_manager(char* host_name, u_int16_t tcp_port, u_int16_t udp_port) {
  struct chatserver_manager* chatserver_mgr = (struct chatserver_manager*) malloc(
    sizeof(struct chatserver_manager)
  );
  strncpy(chatserver_mgr->host_name, host_name, MAX_HOST_NAME_LEN);  
  chatserver_mgr->tcp_port = tcp_port;
  chatserver_mgr->udp_port = udp_port;
  return chatserver_mgr;
}

void destroy_chatserver_manager(struct chatserver_manager* chatserver_manager) {
  free(chatserver_manager);
}

int refresh_chatserver(struct chatserver_manager* chatserver_mgr) {  
  int nerror;
  u_int16_t http_status;
  struct http_connection* http_con = create_http_connection(LOC_SERV_HOST_NAME, LOC_SERV_PORT, &nerror);  
  char* respond = send_http_request(http_con, LOC_SERV_URL, &http_status, &nerror);  
  sscanf (respond, "%s %hu %hu", 
            chatserver_mgr->host_name,
            &chatserver_mgr->tcp_port, 
            &chatserver_mgr->udp_port);
  
  close_http_connection(http_con);
  free(respond);
  return 0;
}

