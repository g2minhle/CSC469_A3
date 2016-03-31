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

/* Given the response from a control request, depending on the response type,
 * determine what should be outputted and sent to the client receiver. */
char* process_response (char* resp, u_int16_t resp_len, char* extra)
{
  struct control_msghdr* resp_hdr = (struct control_msghdr*) resp;
  u_int16_t msg_len = resp_len - sizeof(struct control_msghdr) + 1;

  if (msg_len <= 1) // there will be a null char
    msg_len = 100+MAX_ROOM_NAME_LEN; // want to returm a msg, so allocate some space

  char* msg = (char*) malloc(msg_len);
  //TODO: error check mallocs
  bzero(msg, msg_len);

  switch (ntohs(resp_hdr->msg_type)){
    case REGISTER_SUCC:
    case REGISTER_FAIL:
    case ROOM_LIST_SUCC:
    case ROOM_LIST_FAIL:
    case MEMBER_LIST_SUCC:
    case MEMBER_LIST_FAIL:
    case SWITCH_ROOM_FAIL:
    case CREATE_ROOM_FAIL:
      strncpy(msg, (char*)(resp_hdr->msgdata), msg_len);
      break;
    case SWITCH_ROOM_SUCC:
      snprintf(msg, msg_len, "Successfully switched to room %s", extra);
      break;
    case CREATE_ROOM_SUCC:
      snprintf(msg, msg_len, "Successfully created room %s", extra);
      break;
  }

  return msg;
}

void re_register_func(struct client_to_server_sender* sender){
  while (1) {
    char* error_msg = send_register_request(sender, 
                                            sender->cli_core->member_name,
                                            sender->cli_core->receiver_manager->client_udp_port,
                                            &(sender->cli_core->member_id));
    if(error_msg) {
      free(error_msg);
      sprintf(sender->cli_core->member_name, "%s_", sender->cli_core->member_name);          
    } else {
      return;       
    }    
  }
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
                        u_int16_t* respond_size,
                        bool re_register) {
  pthread_mutex_lock(&sender->sender_lock);
  int nerror;
  int tcp_port;
  char* respond;
  int chatserver_manager_result;
  struct chatserver_manager* chatserver_manager = sender->chatserver_manager;
  char* host_name = chatserver_manager->host_name; 

  // Keep trying until we make a connection
  while (1) {
    tcp_port = chatserver_manager->tcp_port;

    struct tcp_connection* tcp_con = create_tcp_connection(host_name, tcp_port, &nerror);      
    
    if (tcp_con == NULL) {
      chatserver_manager_result = refresh_chatserver(chatserver_manager);
      if (chatserver_manager_result != 0) {
        // TODO #improvement: Handle the case location server failed
      } else {
        if(re_register) {
          re_register_func(sender);
        }
      }
      continue;
    }
    respond = send_tcp_request(tcp_con, request, request_size, respond_size, &nerror);
    
    if (respond == NULL) {
      close_tcp_connection(tcp_con);
      chatserver_manager_result = refresh_chatserver(chatserver_manager);
      if (chatserver_manager_result != 0) {
        // TODO #improvement: Handle the case location server failed
      } else {
        if(re_register) {
          re_register_func(sender);
        }
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
  printf("%s\n", member_name);
  u_int16_t member_name_size = strnlen(member_name, MAX_MEMBER_NAME_LEN);
  u_int16_t msg_len = sizeof(struct register_msgdata) + member_name_size;
                  
  struct register_msgdata* msgdata = (struct register_msgdata*)malloc(msg_len);
  
  // TODO #improvement: Handle the case when buf cannot be created
    
  msgdata->udp_port = htons(udp_port);
  strncpy((char*)msgdata->member_name, member_name, member_name_size);
  printf("%s\n", (char*)msgdata->member_name);
  
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
#ifdef DBUG
    printf("%d\n", msghdr->member_id);
#endif
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
  char* respond = send_control_msg(sender, request, request_len, &respond_len, FALSE);
  
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
  char* respond = send_control_msg(sender, request, request_len, &respond_len, TRUE);

  char * msg = process_response (respond, respond_len, "\0");
  free(request);
  free(respond);
  return msg;
}

char* send_member_list_request(struct client_to_server_sender* sender, u_int16_t member_id, char* room_name) {
  u_int16_t request_len;
  u_int16_t room_name_len = strnlen(room_name, MAX_ROOM_NAME_LEN);
  
  char* request = prepare_request_with_data(MEMBER_LIST_REQUEST, member_id, &request_len, room_name, room_name_len);
  
  u_int16_t respond_len;
  char* respond = send_control_msg(sender, request, request_len, &respond_len, TRUE);
  
  char * msg = process_response (respond, respond_len, room_name);
  free(request);
  free(respond);
  return msg;
}

char* send_switch_room_request(struct client_to_server_sender* sender, u_int16_t member_id, char* room_name) {
  u_int16_t request_len;
  u_int16_t room_name_len = strnlen(room_name, MAX_ROOM_NAME_LEN);
  
  char* request = prepare_request_with_data(SWITCH_ROOM_REQUEST, member_id, &request_len, room_name, room_name_len);
  
  u_int16_t respond_len;
  char* respond = send_control_msg(sender, request, request_len, &respond_len, TRUE);
  
  char * msg = process_response (respond, respond_len, room_name);
  free(request);
  free(respond);
  return msg;
}

char* send_create_room_request(struct client_to_server_sender* sender, u_int16_t member_id, char* room_name) {
  u_int16_t request_len;
  u_int16_t room_name_len = strnlen(room_name, MAX_ROOM_NAME_LEN);
  
  char* request = prepare_request_with_data(CREATE_ROOM_REQUEST, member_id, &request_len, room_name, room_name_len);
  
  u_int16_t respond_len;
  char* respond = send_control_msg(sender, request, request_len, &respond_len, TRUE);
  
  char * msg = process_response (respond, respond_len, "\0");
  free(request);
  free(respond);
  return msg;
}

void send_quit_request(struct client_to_server_sender* sender, u_int16_t member_id) {
  u_int16_t request_len;
  char* request = prepare_request_with_no_data(QUIT_REQUEST, member_id, &request_len);
  
  u_int16_t respond_len;
  char* respond = send_control_msg(sender, request, request_len, &respond_len, TRUE);
  
  free(respond);
  free(request);
}

void send_heart_beat(struct client_to_server_sender* sender, u_int16_t member_id) {
  u_int16_t request_len;
  char* request = prepare_request_with_no_data(MEMBER_KEEP_ALIVE, member_id, &request_len);
  
  u_int16_t respond_len;
  char* respond = send_control_msg(sender, request, request_len, &respond_len, TRUE);

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

  pthread_mutexattr_t Attr;

  pthread_mutexattr_init(&Attr);
  pthread_mutexattr_settype(&Attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&ctrl_sender->sender_lock, &Attr);
  return ctrl_sender;
}

void destroy_client_to_server_sender(struct client_to_server_sender* sender) {
  destroy_chatserver_manager(sender->chatserver_manager);
  free(sender);
}


/* Given the ctos sender and the chat message to be sent, send the chat message */
void send_chat_msg (struct client_to_server_sender* sender, char* cmsg, u_int16_t member_id) {
  uint8_t* msg = (uint8_t*) malloc(MAX_MSG_LEN);
  if (msg == NULL) {
    perror("client_to_server_sender malloc");
    return;
  }

  /* since this is being sent to the server and not through IPC, we don't need
   * the msg_t header. therefore, just set up the chat_msghdr */
  bzero(msg, MAX_MSG_LEN);
  struct chat_msghdr* cmh = (struct chat_msghdr*) (msg);
  u_int16_t cmsg_len = strnlen(cmsg, MAX_MSG_LEN - sizeof(struct chat_msghdr));
  u_int16_t msg_len = sizeof(struct chat_msghdr) + cmsg_len;  

  memcpy(cmh->msgdata, cmsg, cmsg_len);
  cmh->sender.member_id = htons(member_id);
  cmh->msg_len = htons(cmsg_len);

  int nerror;
  struct chatserver_manager* chatserver_manager = sender->chatserver_manager;
  int udp_port = chatserver_manager->udp_port;
  char* host_name = chatserver_manager->host_name; 

  struct udp_connection* udp_con = create_udp_connection(host_name, udp_port, &nerror); 
  send_udp_request(udp_con,(char*) msg, msg_len, &nerror);
  close_udp_connection(udp_con);

  free(msg);
}
