#include "client_to_server_sender.h"

/*
 * Convert given msg type, member id and mgs len to network format 
 * and put inside the msg header
 *
 * Agrs:
 *    struct control_msghdr* msghdr:
 *      The message header.
 *    u_int16_t msg_type:
 *      The message type.
 *    u_int16_t member_id:
 *      The member id.
 *    u_int16_t msg_len:
 *      The message length. 
 */
void encode_control_msghdr(struct control_msghdr* msghdr,
                            u_int16_t msg_type,
                            u_int16_t member_id,
                            u_int16_t msg_len) {
  msghdr->msg_type = htons(msg_type);
  msghdr->member_id = htons(member_id);
  msghdr->msg_len = htons(msg_len);
}

/*
 * Convert given msg type, member id and mgs len to host format 
 * and put inside the msg header
 *
 * Agrs:
 *    struct control_msghdr* msghdr:
 *      The message header.
 *    u_int16_t msg_type:
 *      The message type.
 *    u_int16_t member_id:
 *      The member id.
 *    u_int16_t msg_len:
 *      The message length. 
 */
void decode_control_msghdr(struct control_msghdr* msghdr,
                            u_int16_t msg_type,
                            u_int16_t member_id,
                            u_int16_t msg_len) {
  msghdr->msg_type = ntohs(msg_type);
  msghdr->member_id = ntohs(member_id);
  msghdr->msg_len = ntohs(msg_len);
}

/*
 * Send a control message given the sender, the message and the size of the
 * the message. If the chatserver cannot be reach, the function will evoke the 
 * location server to ask for a different chatserver untill we make a connection.
 *
 * Args:
 *    struct client_to_server_sender* sender:
 *      The control message sender.
 *    char* message:
 *      The message.
 *    u_int16_t message_size:
 *      The size of the message.
 *
 * Return:
 *    The respond of the request.
 */
char* send_control_msg(struct client_to_server_sender* sender, 
                        char* request, 
                        u_int16_t request_size, 
                        u_int16_t* respond_size) {
  pthread_mutex_lock(&sender->sender_lock);
  
  int chatserver_manager_result;
  int nerror;
  struct chatserver_manager* chatserver_manager = sender->chatserver_manager;
  int tcp_port = chatserver_manager->tcp_port;
  char* host_name = chatserver_manager->host_name; 
  char* respond;

  // Keep trying until we make a connection
  while (1) {
    struct tcp_connection* tcp_con = create_tcp_connection(host_name, tcp_port, &nerror);      
    
    if (tcp_con == NULL) {
      chatserver_manager_result = refresh_chatserver(chatserver_manager);
      if (chatserver_manager_result != 0) {
        // TODO #improvement: Handle the case location server failed
      }
      continue;
    }
    respond = send_tcp_request(tcp_con, request, request_size, respond_size, &nerror);
    
    if (respond == NULL) {
      close_tcp_connection(tcp_con);
      chatserver_manager_result = refresh_chatserver(chatserver_manager);
      if (chatserver_manager_result != 0) {
        // TODO #improvement: Handle the case location server failed
      }
      continue;
    }
    
    close_tcp_connection(tcp_con);
    break;    
  }
  
  pthread_mutex_unlock(&sender->sender_lock);
  return respond; 
}

char* prepare_request_with_no_data(u_int16_t msg_type, 
                                    u_int16_t member_id, 
                                    u_int16_t* request_len) {
  *request_len = sizeof(struct control_msghdr);
                  
  char* request = (char*)malloc(*request_len);
  
  // TODO #improvement: Handle the case when buf cannot be created
  
  encode_control_msghdr((struct control_msghdr*)request, msg_type, member_id, *request_len);  
  return request;
}

char* prepare_request_with_data(u_int16_t msg_type, 
                                  u_int16_t member_id, 
                                  u_int16_t* request_len, 
                                  char* msgdata, 
                                  u_int16_t msg_len) {
  *request_len = sizeof(struct control_msghdr) + msg_len;
                  
  struct control_msghdr* msghdr = (struct control_msghdr*)malloc(*request_len);
  
  // TODO #improvement: Handle the case when buf cannot be created
     
  encode_control_msghdr(msghdr, msg_type, member_id, *request_len);  
  memcpy (msghdr->msgdata, msgdata, msg_len);
  
  return (char*)msghdr;
}


char* prepare_register_request(u_int16_t udp_port, char* member_name, u_int16_t* request_len) {
  u_int16_t member_name_size = strnlen(member_name, MAX_MEMBER_NAME_LEN);
  u_int16_t msg_len = sizeof(struct register_msgdata) + member_name_size + 1;
                  
  struct register_msgdata* msgdata = (struct register_msgdata*)malloc(msg_len);
  
  // TODO #improvement: Handle the case when buf cannot be created
    
  msgdata->udp_port = htons(udp_port);
  strncpy((char*)msgdata->member_name, member_name, member_name_size);
  
  char* respond = prepare_request_with_data(REGISTER_REQUEST, 0, request_len, (char*)msgdata, msg_len);
  
  free(msgdata);
  return respond;
}

char* handel_register_respond(char* respond, u_int16_t respond_len, u_int16_t* member_id) {

  struct control_msghdr* msghdr = (struct control_msghdr*)(
    respond
  );
  /*struct register_msgdata* msgdata = (struct register_msgdata*)(
    respond + sizeof(struct control_msghdr)
  );*/
  
  decode_control_msghdr(msghdr, 
                        msghdr->msg_type,
                        msghdr->member_id,
                        msghdr->msg_len);

  if (msghdr->msg_type == REGISTER_FAIL) {
    u_int16_t error_msg_len = respond_len - sizeof(struct control_msghdr) + 1;
    char* error_msg = (char*) malloc(respond_len);
    strncpy(error_msg, (char*)msghdr->msgdata, error_msg_len);
    return error_msg;
  } else if (msghdr->msg_type == REGISTER_SUCC) {    
    *member_id = msghdr->member_id;
    return NULL;
  } else {
    return NULL;
  }
}

char* send_register_request(struct client_to_server_sender* sender,
                                char* member_name,
                                u_int16_t udp_port,
                                u_int16_t* member_id) {
  u_int16_t request_len;
  char* request = prepare_register_request(udp_port, member_name, &request_len);
  
  // TODO #improvement: Handle the case when request cannot be created
  
  // We should always get back a respond since if the chatserver fail
  // "send_control_msg" will try to poke location server for a new chatserver.
  u_int16_t respond_len;
  char* respond = send_control_msg(sender, request, request_len, &respond_len);
  
  // TODO #improvement: Handle the case when cannot send a ctrl request
  
  // Handle the the respond
  char* error_msg = handel_register_respond(respond, respond_len, member_id);
  
  free(request);
  free(respond);
  return error_msg;
}

char* send_room_list_request(struct client_to_server_sender* sender, u_int16_t member_id) {
  u_int16_t request_len;
  char* request = prepare_request_with_no_data(ROOM_LIST_REQUEST, member_id, &request_len);
  
  u_int16_t respond_len;
  char* respond = send_control_msg(sender, request, request_len, &respond_len);

  u_int16_t msg_len = respond_len - sizeof(struct control_msghdr) + 1;
  char* msg = (char*) malloc(msg_len);
  strncpy(msg, (char*)(((struct control_msghdr*)respond)->msgdata), msg_len);
  
  free(request);
  free(respond);
  return msg;
}

char* send_member_list_request(struct client_to_server_sender* sender, u_int16_t member_id, char* room_name) {
  u_int16_t request_len;
  u_int16_t room_name_len = strnlen(room_name, MAX_ROOM_NAME_LEN);
  
  char* request = prepare_request_with_data(MEMBER_LIST_REQUEST, member_id, &request_len, room_name, room_name_len);
  
  u_int16_t respond_len;
  char* respond = send_control_msg(sender, request, request_len, &respond_len);
  
  u_int16_t msg_len = respond_len - sizeof(struct control_msghdr) + 1;
  char* msg = (char*) malloc(msg_len);
  strncpy(msg, (char*)(((struct control_msghdr*)respond)->msgdata), msg_len);
  
  free(request);
  free(respond);
  return msg;
}

char* send_switch_room_request(struct client_to_server_sender* sender, u_int16_t member_id, char* room_name) {
  u_int16_t request_len;
  u_int16_t room_name_len = strnlen(room_name, MAX_ROOM_NAME_LEN);
  
  char* request = prepare_request_with_data(SWITCH_ROOM_REQUEST, member_id, &request_len, room_name, room_name_len);
  
  u_int16_t respond_len;
  char* respond = send_control_msg(sender, request, request_len, &respond_len);
  
  u_int16_t msg_len = respond_len - sizeof(struct control_msghdr) + 1;
  char* msg = (char*) malloc(msg_len);
  strncpy(msg, (char*)(((struct control_msghdr*)respond)->msgdata), msg_len);
  
  free(request);
  free(respond);
  return msg;
}

char* send_create_room_request(struct client_to_server_sender* sender, u_int16_t member_id, char* room_name) {
  u_int16_t request_len;
  u_int16_t room_name_len = strnlen(room_name, MAX_ROOM_NAME_LEN);
  
  char* request = prepare_request_with_data(CREATE_ROOM_REQUEST, member_id, &request_len, room_name, room_name_len);
  
  u_int16_t respond_len;
  char* respond = send_control_msg(sender, request, request_len, &respond_len);
  
  u_int16_t msg_len = respond_len - sizeof(struct control_msghdr) + 1;
  char* msg = (char*) malloc(msg_len);
  strncpy(msg, (char*)(((struct control_msghdr*)respond)->msgdata), msg_len);
  
  free(request);
  free(respond);
  return msg;
}

void send_quit_request(struct client_to_server_sender* sender, u_int16_t member_id) {
  u_int16_t request_len;
  char* request = prepare_request_with_no_data(QUIT_REQUEST, member_id, &request_len);
  
  u_int16_t respond_len;
  char* respond = send_control_msg(sender, request, request_len, &respond_len);
  
  free(respond);
  free(request);
}

void send_heart_beat(struct client_to_server_sender* sender, u_int16_t member_id) {
  u_int16_t request_len;
  char* request = prepare_request_with_no_data(MEMBER_KEEP_ALIVE, member_id, &request_len);
  
  u_int16_t respond_len;
  char* respond = send_control_msg(sender, request, request_len, &respond_len);

  free(respond);
  free(request);
}


/*
 * Create control request sender
 *
 * Return:
 *    struct client_to_server_sender*:
 *      The control request sender.
 */
struct client_to_server_sender* create_client_to_server_sender(char* server_host_name,
                                                                u_int16_t server_tcp_port,
                                                                u_int16_t server_udp_port) {
  
  struct client_to_server_sender* ctrl_sender = (struct client_to_server_sender*)malloc(
    sizeof(struct client_to_server_sender)
  );
  
  if(ctrl_sender == NULL) return NULL;
    
  ctrl_sender->chatserver_manager = create_chatserver_manager(server_host_name,
                                                                server_tcp_port,
                                                                server_udp_port);
  
  if(ctrl_sender->chatserver_manager == NULL) {
    free(ctrl_sender);
    return NULL;
  }

  pthread_mutex_init(&ctrl_sender->sender_lock, NULL);     
  
  return ctrl_sender;
}

void destroy_client_to_server_sender(struct client_to_server_sender* sender){
  destroy_chatserver_manager(sender->chatserver_manager);
  free(sender);
}
