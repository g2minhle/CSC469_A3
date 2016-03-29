#include "client_main.h"

/*
 * Print out the usage of the chatclient and exit the program.
 *
 * Args;
 *    char **argv: The agrument list.
 *
 * Return:
 *    This function exit the program so it never returns.
 */
static void usage(char **argv) {

  printf("usage:\n");

  #ifdef USE_LOCN_SERVER
    printf("%s -n <client member name>\n",argv[0]);
  #else 
    printf("%s -h <server host name> -t <server tcp port> -u <server udp port> -n <client member name>\n",argv[0]);
  #endif /* USE_LOCN_SERVER */

  exit(1);
}

/*
 * Get command line agruments into chatclient context.
 * If command line agruments are invalid then print out the usage and exit the program.
 * 
 * Agrs:
 *    struct chatclient_context* chatcli_ctx:
 *      The chatclient context.
 *    int argc:
 *      The number of argument.
 *    char **argv:
 *      The vector of agruments.
 */
void get_agrs(struct chatclient_context* chatcli_ctx, int argc, char **argv){
  char option;
  
  while((option = getopt(argc, argv, option_string)) != -1) {
    switch(option) {
    case 'h':
      strncpy(chatcli_ctx->server_host_name, optarg, MAX_HOST_NAME_LEN);
      break;
    case 't':
      chatcli_ctx->server_tcp_port = atoi(optarg);
      break;
    case 'u':
      chatcli_ctx->server_udp_port = atoi(optarg);
      break;
    case 'n':
      strncpy(chatcli_ctx->member_name, optarg, MAX_MEMBER_NAME_LEN);
      break;
    default:
      printf("invalid option %c\n",option);
      usage(argv);
      break;
    }
  } 

#ifdef USE_LOCN_SERVER

  printf("Using location server to retrieve chatserver information\n");

  if (strlen(chatcli_ctx->member_name) == 0) {
    usage(argv);
  }

#else

  if(chatcli_ctx->server_tcp_port == 0 
      || chatcli_ctx->server_udp_port == 0 
      || strlen(chatcli_ctx->server_host_name) == 0 
      || strlen(chatcli_ctx->member_name) == 0) {
    usage(argv);
  }

#endif /* USE_LOCN_SERVER */
  
}

/*
 * Validate if the user input is correct and return the data of the ctrl command
 *
 * Args:
 *    char cmd:
 *      The control command character code
 *    char* line:
 *      The user input behind the control command character
 *
 * Return:
 *    The data of the control command character.
 *    NULL if the ctrl command is invalid.
 */
char* validate_and_extract_data_from_ctrl_command(char cmd, char* line) {
  switch(cmd) {

  case 'r':
  case 'q':
    if (strlen(line) != 0) {
      printf("Error in command format: !%c should not be followed by anything.\n",cmd);
      return NULL;
    }
    break;

  case 'c':
  case 'm':
  case 's': {
      int allowed_len = MAX_ROOM_NAME_LEN;

      if (line[0] != ' ') {
        printf("Error in command format: !%c should be followed by a space and a room name.\n",cmd);
        return NULL;
      }
      
      line++; /* skip space before room name */

      int len = strlen(line);
      int goodlen = strcspn(line, " \t\n"); /* Any more whitespace in line? */
      
      if (len != goodlen) {
        printf("Error in command format: line contains extra whitespace (space, tab or carriage return)\n");
        return NULL;
      }
      if (len > allowed_len) {
        printf("Error in command format: name must not exceed %d characters.\n",allowed_len);
        return NULL;
      }
      return line;
  }

  default:
    printf("Error: unrecognized command !%c\n",cmd);
    return NULL;
  }
  return line;
}

bool send_ctrl_request(struct client_core* cli_core, char cmd, char* msgdata) {
  switch(cmd) {

  case 'r':
    cli_core_room_list_request(cli_core);
    return TRUE;
  case 'c':
    cli_core_create_room_request(cli_core, msgdata);
    return TRUE;

  case 'm':
    cli_core_member_list_request(cli_core, msgdata);
    return TRUE;

  case 's':
    cli_core_switch_room_request(cli_core, msgdata);
    return TRUE;

  case 'q':
    return FALSE;

  default:
    printf("Error !%c is not a recognized command.\n",cmd);
    return TRUE;
  }

}

/* 
 * This should be called with the leading "!" from input line. 
 */
bool handle_command_input(struct client_core* cli_core, char *line)
{
  /* Check if control message or chat message */
  /* buf probably ends with newline.  If so, get rid of it. */
  int len = strlen(line);
  if ( line[len - 1] == '\n') {
    line[ len - 1] = '\0';
  }
  char cmd = line[1]; /* single character identifying which command */
  /* skip ! and cmd char*/
  line += 2;  
  
  /* 1. Simple format check */
  char* msgdata = validate_and_extract_data_from_ctrl_command(cmd, line);

  if (msgdata) {
    /* 2. Passed format checks.  Handle the command */
    return send_ctrl_request(cli_core,cmd, msgdata);
  }

  // Do not quit the program yet
  return TRUE;
}

/*
 * Print out the shell header and get user input.
 * 
 * Agrs:
 *    char* member_name:
 *      The name of the current member.
 *    char *buf:
 *      The buffer space.
 *
 * Returns:
 *    char*:
 *      The user input.
 */
char* get_user_input(char* member_name, char* buf) {
    bzero(buf, MAX_MSGDATA);
    printf("\n[%s]>  ",  member_name);
    return fgets(buf, MAX_MSGDATA, stdin);  
}

void chat_interface(struct client_core* cli_core) {
  char* user_input;
  bool _continue = TRUE;
  char *buf = (char *)malloc(MAX_MSGDATA);
  while(_continue) {
    user_input = get_user_input(cli_core->member_name, buf);
    
    if (user_input == NULL) {
      printf("Error or EOF while reading user input.  Guess we're done.\n");
      _continue = FALSE;
    } else if (buf[0] == '!') {
      _continue = handle_command_input(cli_core, buf);
    } else {
      cli_core_send_chatmsg(cli_core, buf);
    }
  }  
  
  cli_core_quit(cli_core);  
  free(buf);  
}

#include "http_connection.h"

#define LOC_SERV_HOST_NAME "www.cdf.toronto.edu"
#define LOC_SERV_PORT 80
#define LOC_SERV_URL "/~csc469h/winter/chatserver.txt"

int main(int argc, char **argv)
{
  struct chatclient_context chatcli_ctx;
  get_agrs(&chatcli_ctx, argc, argv);

  struct client_core* cli_core = create_client_core(chatcli_ctx.member_name, 
                                                    chatcli_ctx.server_host_name,
                                                    chatcli_ctx.server_tcp_port,
                                                    chatcli_ctx.server_udp_port);  
  if (cli_core) {  
    chat_interface(cli_core);    
    free(cli_core);
    return 0;
  }
  return 1;
}
