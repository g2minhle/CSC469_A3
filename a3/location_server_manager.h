#ifndef __LOCATION_SERVER_MANAGER_H
#define __LOCATION_SERVER_MANAGER_H

#define LOC_SERV_HOST_NAME

#include <string.h>
#include <stdlib.h>

#include "defs.h"
//#include "http_connection.h"

/*
 * This struct tries to provide information about chatserver through communicating
 * the location server. This class sends out HTTP request to get info about the 
 * chat server.
 */
struct location_server_manager {  
  char* host_name;
  int tcp_port;
  int udp_port;  
  // TODO #feature: enable this
  //struct receiver_manager* receiver_mgr;
};

struct location_server_manager* create_location_server_manager();
int refresh_chatserver(struct location_server_manager* loc_srv_mgr);

#endif
