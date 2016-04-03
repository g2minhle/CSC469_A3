#ifndef __LOCATION_SERVER_MANAGER_H
#define __LOCATION_SERVER_MANAGER_H

#define LOC_SERV_HOST_NAME "www.cdf.toronto.edu"
#define LOC_SERV_PORT 80
#define LOC_SERV_URL "/~csc469h/winter/chatserver.txt"

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "defs.h"
#include "http_connection.h"

/*
 * This struct tries to provide information about chatserver through communicating
 * the location server. This class sends out HTTP request to get info about the 
 * chat server.
 */
struct chatserver_manager {  
  char host_name[MAX_HOST_NAME_LEN];
  u_int16_t tcp_port;
  u_int16_t udp_port; 
};

struct chatserver_manager* create_chatserver_manager(char* host_name, u_int16_t tcp_port, u_int16_t udp_port);
void destroy_chatserver_manager(struct chatserver_manager* chatserver_manager);
int refresh_chatserver(struct chatserver_manager* chatserver_mgr);

#endif
