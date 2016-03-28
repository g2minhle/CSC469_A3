#include "ctrl_request_sender.h"

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
 *    struct ctrl_request_sender* sender:
 *      The control message sender.
 *    char* message:
 *      The message.
 *    size_t message_size:
 *      The size of the message.
 *
 * Return:
 *    The respond of the request.
 */
char* send_control_msg(struct ctrl_request_sender* sender, 
                        char* request, 
                        size_t request_size, 
                        size_t* respond_size) {
  pthread_mutex_lock(&sender->sender_lock);
  
  int loc_srv_mgr_result;
  int nerror;
  struct location_server_manager* loc_srv_mgr = sender->loc_srv_mgr;
  int tcp_port = loc_srv_mgr->tcp_port;
  char* host_name = loc_srv_mgr->host_name; 
  char* respond;

  // Keep trying until we make a connection
  while (1) {
    struct tcp_connection* tcp_con = create_tcp_connection(host_name, tcp_port, &nerror);      
    
    if (tcp_con == NULL) {
      loc_srv_mgr_result = refresh_chatserver(loc_srv_mgr);
      if (loc_srv_mgr_result != 0) {
        // TODO #improvement: Handle the case location server failed
      }
      continue;
    }
    respond = send_tcp_request(tcp_con, request, request_size, respond_size, &nerror);
    
    if (respond == NULL) {
      close_tcp_connection(tcp_con);
      loc_srv_mgr_result = refresh_chatserver(loc_srv_mgr);
      if (loc_srv_mgr_result != 0) {
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

char* prepare_register_request(u_int16_t udp_port, char* member_name, size_t* request_len) {

  size_t member_name_size = strnlen(member_name, MAX_MEMBER_NAME_LEN);
  *request_len = sizeof(struct control_msghdr)
                  + sizeof(struct register_msgdata)
                  + member_name_size
                  + 1;
                  
  char* buf = (char*)malloc(*request_len);
  
  // TODO #improvement: Handle the case when buf cannot be created
  
  struct control_msghdr* msghdr = (struct control_msghdr*)(
    buf
  );
  struct register_msgdata* msgdata = (struct register_msgdata*)msghdr->msgdata;
  
  msgdata->udp_port = htons(udp_port);
  strncpy((char*)msgdata->member_name, member_name, member_name_size);

  encode_control_msghdr(msghdr, REGISTER_REQUEST, 0, *request_len);
  
  return buf;
}

void handel_register_respond(char* respond) {

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
    printf("register failed");
  } else if (msghdr->msg_type == REGISTER_SUCC) {
    printf("register SUCC");
  }    
}

void send_register_request(struct ctrl_request_sender* sender,
                            char* member_name,
                            u_int16_t udp_port) {
  size_t request_len;
  char* request = prepare_register_request(udp_port, member_name, &request_len);
  
  // TODO #improvement: Handle the case when request cannot be created
  
  // We should always get back a respond since if the chatserver fail
  // "send_control_msg" will try to poke location server for a new chatserver.
  size_t respond_len;
  char* respond = send_control_msg(sender, request, request_len, &respond_len);
  
  // TODO #improvement: Handle the case when cannot send a ctrl request
  
  // Handle the the respond
  handel_register_respond(respond);
  
  free(request);
  free(respond);
}

/*
 * Create control request sender
 *
 * Agrs:
 *    char* member_name:
 *      The user name
 *    int udp_port:
 *      The reciever udp port
 *    struct receiver_manager* receiver_mgr:
 *      The receiver manager
 *
 * Return:
 *    struct ctrl_request_sender*:
 *      The control request sender.
 */
struct ctrl_request_sender* create_ctrl_request_sender(char* member_name, 
                                                        int udp_port) {
                                                        // TODO #feature: enable this struct receiver_manager* receiver_mgr) {
  
  struct ctrl_request_sender* ctrl_sender = (struct ctrl_request_sender*)malloc(
    sizeof(struct ctrl_request_sender)
  );
  
  // TODO #improvement: Handle the case when ctrl_sender cannot be created
  
  // TODO #feature: ctrl_sender->loc_srv_mgr = create_location_server_manager(receiver_mgr);
  ctrl_sender->loc_srv_mgr = create_location_server_manager();
  
  // TODO #improvement: Handle the case when loc_srv_mgr cannot be created

  //pthread_mutex_init(&ctrl_sender->sender_lock, NULL);
    
  send_register_request(ctrl_sender, member_name, udp_port);
  
  return ctrl_sender;
}
