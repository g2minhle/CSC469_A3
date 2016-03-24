/*
 * This struct contains and manages all the logic of client app client_main is 
 * just an interface that interact with this core struct
 */
struct client_core {
  // Interface:
    // Init structure
      // 
      // send_register_req()     
      
    // get_room_list() 
    // get_member_list_req(char *room_name)
    // switch_room_req(char *room_name)
    // create_room_req(char *room_name)
    // quit()
}


/*
 * This struct sends and revices all control requests and report of the current connection is still alive
 */
struct control_request_sender {

}

/*
 * This struct tries to provide information about chatserver through communicating
 * the location server. This class sends out HTTP request to get info about the 
 * chat server.
 */
struct location_server_manager {
  
}

/*
 * This struct manages the receiver. Including creating the reciver,
 * shutting the receiver down, establish udp connecting to and sending out messages.
 */
struct receiver_communicator {
  
}

/*
 * This struct connects to a server, send a request and get back the result. 
 */
struct tcp_connection {
  // Init with ip - port
  // Send request
  // Get respond
}
