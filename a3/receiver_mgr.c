#include "receiver_mgr.h"

int init_ipc_with_receiver(char* ctrl2rcvr_fname, int *qid)
{
  /* Create IPC message queue for communication with receiver process */

  int msg_fd;
  int msg_key;

  /* 1. Create file for message channels */

  snprintf(ctrl2rcvr_fname, MAX_FILE_NAME_LEN,"/tmp/ctrl2rcvr_channel.XXXXXX");
  msg_fd = mkstemp(ctrl2rcvr_fname);

  if (msg_fd  < 0) {
    perror("Could not create file for communication channel");
    return -1;
  }

  close(msg_fd);

  /* 2. Create message channel... if it already exists, delete it and try again */

  msg_key = ftok(ctrl2rcvr_fname, 42);

  if ( (*qid = msgget(msg_key, IPC_CREAT|IPC_EXCL|S_IREAD|S_IWRITE)) < 0) {
    if (errno == EEXIST) {
      if ( (*qid = msgget(msg_key, S_IREAD|S_IWRITE)) < 0) {
        perror("First try said queue existed. Second try can't get it");
        unlink(ctrl2rcvr_fname);
        return -1;
      }
      if (msgctl(*qid, IPC_RMID, NULL)) {
        perror("msgctl removal failed. Giving up");
        unlink(ctrl2rcvr_fname);
        return -1;
      } else {
        /* Removed... try opening again */
        if ( (*qid = msgget(msg_key, IPC_CREAT|IPC_EXCL|S_IREAD|S_IWRITE)) < 0) {
          perror("Removed queue, but create still fails. Giving up");
          unlink(ctrl2rcvr_fname);
          return -1;
        }
      }

    } else {
      perror("Could not create message queue for client control <--> receiver");
      unlink(ctrl2rcvr_fname);
      return -1;
    }
    
  }

  return 0;
}

pid_t start_receiver(char* ctrl2rcvr_fname) {

  pid_t receiver_pid = fork();

  if (receiver_pid < 0) {
    fprintf(stderr,"Could not fork child for receiver\n");
    return -1;
  }

  if (receiver_pid == 0) {
    /* this is the child. Exec receiver */
    char *argv[] = {"xterm",
        "-e",
        "./receiver",
        "-f",
        ctrl2rcvr_fname,
        0
    };

    execvp("xterm", argv);
    fprintf(stderr,"Child: exec returned. that can't be good.\n");
    exit(1);
  } 
  return receiver_pid;
}


int get_client_udp_port(int ctrl2rcvr_qid, u_int16_t* client_udp_port, pid_t receiver_pid) {
  msg_t msg;
  int result;
  int numtries = 0;
  
  while ( numtries < IPC_GET_UDP_PORT_TRIALS  ) {
    result = msgrcv(ctrl2rcvr_qid, 
                    &msg, 
                    sizeof(struct body_s), 
                    CTRL_TYPE, 
                    IPC_NOWAIT);
                    
    if (result == -1 && errno == ENOMSG) {
      sleep(1);
      numtries++;
    } else if (result > 0) {
      if (msg.body.status == RECV_READY) {
        printf("Start of receiver successful, port %u\n",msg.body.value);
        *client_udp_port = msg.body.value;
      } else {
        fprintf(stderr,"Start of receiver failed with code %u\n",msg.body.value);
        return -1;
      }
      break;
    } else {
      perror("msgrcv");
    }
    
  }
  
  if (numtries == IPC_GET_UDP_PORT_TRIALS ) {
    /* give up.  wait for receiver to exit so we get an exit code at least */
    int exitcode;
    fprintf(stderr,"Gave up waiting for msg.  Waiting for receiver to exit now\n");
    waitpid(receiver_pid, &exitcode, 0);
    fprintf(stderr,"Start of receiver failed, exited with code %d\n",exitcode);
    return -1;
  }
  return 0;

}

struct receiver_manager* create_receiver_manager()
{
  /* 
   * Create the receiver process using fork/exec and get the port number
   * that it is receiving chat messages on.
   */

  int ctrl2rcvr_qid;  
  char ctrl2rcvr_fname[MAX_FILE_NAME_LEN];
  
  u_int16_t client_udp_port;

  /* 1. Set up message channel for use by control and receiver processes */
  if (init_ipc_with_receiver(ctrl2rcvr_fname, &ctrl2rcvr_qid) < 0) return NULL;
   
  /* 2. fork/exec xterm */
  pid_t receiver_pid = start_receiver(ctrl2rcvr_fname);  

  /* 3. Read message queue and find out what port client receiver is using */
  get_client_udp_port(ctrl2rcvr_qid, &client_udp_port, receiver_pid);

  struct receiver_manager* receiver_mgr = (struct receiver_manager*)malloc(
    sizeof(struct receiver_manager)
  );

  receiver_mgr->receiver_pid = receiver_pid;
  receiver_mgr->client_udp_port = client_udp_port;
  receiver_mgr->ctrl2rcvr_qid = ctrl2rcvr_qid;
  memcpy(receiver_mgr-> ctrl2rcvr_fname, ctrl2rcvr_fname, MAX_FILE_NAME_LEN);

  return receiver_mgr;
}

void receiver_printf(struct receiver_manager* receiver_manager, char* message) {
  uint8_t* data = (uint8_t*) malloc(MAX_MSG_LEN);
  bzero(data, MAX_MSG_LEN);

  if (data == NULL) {
    perror("receiver_mgr malloc");
    return;
  }

  /* fill out the header/metadata for the IPC msg */
  msg_t* hdr = (msg_t*)data;
  hdr->mtype = RECV_TYPE;
  hdr->body.status = RECV_READY;

  /* generate and fill in the struct to hold the message */
  struct chat_msghdr* msg = (struct chat_msghdr*) (data + sizeof(msg_t));
  strncpy(msg->sender.member_name, "--Ctrl req stat--", MAX_MEMBER_NAME_LEN);
  msg->msg_len = strnlen(message, MAX_MSG_LEN - sizeof(msg_t) - sizeof(struct chat_msghdr));
  strncpy((char*)(msg->msgdata), message, msg->msg_len);
  msgsnd(receiver_manager->ctrl2rcvr_qid, data, sizeof(struct body_s) + sizeof(struct chat_msghdr) + msg->msg_len , 0);
}

void shutdown_receiver(struct receiver_manager* receiver_manager) {
  msg_t msg;
  msg.mtype=RECV_TYPE;
  msg.body.status=CHAT_QUIT;

  msgsnd(receiver_manager->ctrl2rcvr_qid, &msg, sizeof(struct body_s), 0);
}

void destroy_receiver_manager(struct receiver_manager* receiver_manager) {
  shutdown_receiver(receiver_manager);
  free(receiver_manager);
}
