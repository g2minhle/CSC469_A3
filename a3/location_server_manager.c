#include "location_server_manager.h"

// TODO #feature: Enable this
//struct location_server_manager* create_location_server_manager(struct receiver_manager* receiver_mgr) {
struct location_server_manager* create_location_server_manager() {
  struct location_server_manager* loc_srv_mgr = (struct location_server_manager*) malloc(
    sizeof(struct location_server_manager)
  );
  loc_srv_mgr->host_name = (char*)malloc(MAX_HOST_NAME_LEN);
  // TODO enable list
  //loc_srv_mgr->receiver_mgr = receiver_mgr;
  refresh_chatserver(loc_srv_mgr);
  return loc_srv_mgr;
}

int refresh_chatserver(struct location_server_manager* loc_srv_mgr) {
  //char raw_data[] = "wolf.cdf.toronto.edu 4000 4000";  
  //char* token = strtok (raw_data, " ");
  //strncpy(loc_srv_mgr->host_name, token, MAX_HOST_NAME_LEN);
  //token = strtok (NULL, " ");  
  //loc_srv_mgr->tcp_port = atoi(token);
  //token = strtok (NULL, " ");
  //loc_srv_mgr->udp_port = atoi(token);
  strncpy(loc_srv_mgr->host_name, "localhost", MAX_HOST_NAME_LEN);
  loc_srv_mgr->tcp_port = 2412;
  loc_srv_mgr->udp_port = 2413;
  return 0;
}

