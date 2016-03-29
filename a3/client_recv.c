/*
 *   CSC469 Winter 2016 A3
 *   Instructor: Bogdan Simion
 *   Date:       19/03/2016
 *  
 *      File:      client_recv.c 
 *      Author:    Angela Demke Brown
 *      Version:   1.0.0
 *      Date:      17/11/2010
 *   
 * Please report bugs/comments to bogdan@cs.toronto.edu
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include "client.h"

static char *option_string = "f:";

/* vars for UDP socket connection */
int udp_fd = 0;
int udp_port = 0;

/* For communication with chat client control process */
int ctrl2rcvr_qid;
char ctrl2rcvr_fname[MAX_FILE_NAME_LEN];


void usage(char **argv) {
  printf("usage:\n");
  printf("%s -f <msg queue file name>\n",argv[0]);
  exit(1);
}


void open_client_channel(int *qid) {

  /* Get messsage channel */
  key_t key = ftok(ctrl2rcvr_fname, 42);

  if ((*qid = msgget(key, 0400)) < 0) {
    perror("open_channel - msgget failed");
    fprintf(stderr,"for message channel ./msg_channel\n");

    /* No way to tell parent about our troubles, unless/until it 
     * wait's for us.  Quit now.
     */
    exit(1);
  }

  return;
}

void send_error(int qid, u_int16_t code)
{
  /* Send an error result over the message channel to client control process */
  msg_t msg;

  msg.mtype = CTRL_TYPE;
  msg.body.status = RECV_NOTREADY;
  msg.body.value = code;

  if (msgsnd(qid, &msg, sizeof(struct body_s), 0) < 0) {
    perror("send_error msgsnd");
  }
               
}

void send_ok(int qid, u_int16_t port)
{
  /* Send "success" result over the message channel to client control process */
  msg_t msg;

  msg.mtype = CTRL_TYPE;
  msg.body.status = RECV_READY;
  msg.body.value = port;

  if (msgsnd(qid, &msg, sizeof(struct body_s), 0) < 0) {
    perror("send_ok msgsnd");
  } 

}

/* initialize the udp socket, and send the udp port to the chat client if the
 * everything went okay.*/
void  init_udp_socket()
{
	struct sockaddr_in addr;
	socklen_t addr_len;
	int socket_fd;

	addr_len = sizeof(addr);

	if( (socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		send_error(ctrl2rcvr_qid, SOCKET_FAILED);
    return;
	}

	bzero(&addr, addr_len);
	addr.sin_family = AF_INET;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if( bind(socket_fd, (struct sockaddr *)&addr, addr_len) < 0 )
  {
    perror("bind");
		send_error(ctrl2rcvr_qid, BIND_FAILED);
    return;
  }

  if( getsockname(socket_fd, (struct sockaddr *)&addr, &addr_len) < 0 )
  {
    perror("getsockname");
		send_error(ctrl2rcvr_qid, NAME_FAILED);
    return;
  }

	/* server is created successfully */
  udp_port = ntohs(addr.sin_port);
  udp_fd = socket_fd;
  send_ok(ctrl2rcvr_qid, udp_port);
	printf("Chat receiver listening on %s port: %hu\n", "UDP\0", udp_port);
}

void init_receiver()
{

  /* 1. Make sure we can talk to parent (client control process) */
  printf("Trying to open client channel\n");

  open_client_channel(&ctrl2rcvr_qid);

  /* 2. Initialize UDP socket for receiving chat messages. */
  /* 3. Tell parent the port number if successful, or failure code if not. 
   *    Use the send_error and send_ok functions
   */
  init_udp_socket();
}




/* Function to deal with a single message from the chat server */
void handle_received_msg(char *buf)
{
	struct chat_msghdr* cmh = (struct chat_msghdr *)buf;

  printf("%s: %s\n", (char *)cmh->sender.member_name, *(cmh->msgdata));
}

/* Main function to receive and deal with messages from chat server
 * and client control process.  
 *
 * You may wish to refer to server_main.c for an example of the main 
 * server loop that receives messages, but remember that the client 
 * receiver will be receiving (1) connection-less UDP messages from the 
 * chat server and (2) IPC messages on the from the client control process
 * which cannot be handled with the same select()/FD_ISSET strategy used 
 * for file or socket fd's.
 */
void receive_msgs()
{
  char *buf = (char *)malloc(MAX_MSG_LEN);

  if (buf == 0) {
    printf("Could not malloc memory for message buffer\n");
    exit(1);
  }

  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(udp_fd, &fds);


  while(TRUE) {
    ssize_t msg_len = 0;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec= 100000;

    bzero(buf, MAX_MSG_LEN);
    // check the UDP connection for any incoming messages with select.
    msg_len = select(udp_fd, &fds, NULL, NULL, &tv);
    if (msg_len < 0)
    {
      perror("client_recv select()");
    }
    else if(FD_ISSET(udp_fd, &fds)) // probably don't need to check the FD, but oh well.
    {
      msg_len = recvfrom(udp_fd, buf, MAX_MSG_LEN, 0, NULL, 0);

      // Check if we failed to grab a message
      if(msg_len < 0) {
        /* don't do anything about this error. just log the error message */
        perror("client_recv recvfrom");
      } else if (msg_len > 0) {
        printf("%d \n", (int)msg_len);
        // we got a message
        handle_received_msg(buf);
      }
    }

    bzero(buf, MAX_MSG_LEN);
    // check the IPC message for any incoming message from the chat client
    msg_len = msgrcv(ctrl2rcvr_qid, buf, MAX_MSG_LEN, RECV_TYPE, IPC_NOWAIT);
    if (!(msg_len < 0 && errno == ENOMSG)) {
      printf("%d \n", (int)msg_len);
      msg_t* msg = (msg_t*)buf;

      // check if the message is telling the receiver to quit. in which case
      // exit immediately after closing all communication channels
      if (msg->body.status==CHAT_QUIT){
        printf("Exitting the chat. Have a good day.\n");
        close(udp_fd);
        return;
      }

      // else it's a simple message. get the chat_msg struct that falls
      // after the msg_t header information.
      handle_received_msg(buf+sizeof(msg_t));
    }
  }

  /* Cleanup */
  free(buf);
  return;
}


int main(int argc, char **argv) {
  char option;

  printf("RECEIVER alive: parsing options! (argc = %d\n",argc);

  while((option = getopt(argc, argv, option_string)) != -1) {
    switch(option) {
    case 'f':
      strncpy(ctrl2rcvr_fname, optarg, MAX_FILE_NAME_LEN);
      break;
    default:
      printf("invalid option %c\n",option);
      usage(argv);
      break;
    }
  }

  if(strlen(ctrl2rcvr_fname) == 0) {
    usage(argv);
  }

  printf("Receiver options ok... initializing\n");

  init_receiver();

  receive_msgs();

  return 0;
}
