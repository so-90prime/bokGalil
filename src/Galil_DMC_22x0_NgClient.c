/*******************************************************************************
 *
 * Galil_DMC_22x0_NgClient.c
 *
 ******************************************************************************/


/*******************************************************************************
 * include(s)
 ******************************************************************************/
#include "__hosts__.h"
#include "bokGalil.h"
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>


/*******************************************************************************
 * define(s)
 ******************************************************************************/
#undef _HELP_
#define _HELP_  "NG client for the Galil_DMC_22x0 via gclib"
#undef _NAME_
#define _NAME_  "Galil_DMC_22x0_NgClient"


/*******************************************************************************
 * function: client_handler()
 ******************************************************************************/
void client_handler(int handler_fd) {

  /* declare some variable(s) and initialize them */
  bool running = true;
  char *p = (char *)NULL;
  int istat = 0;

  ssize_t bytes_read = 0;
  bool message_read = false;
  char incoming[BOK_NG_STRING];
  char outgoing[BOK_NG_STRING];
  (void) memset(incoming, '\0', sizeof(incoming));
  (void) memset(outgoing, '\0', sizeof(outgoing));

  /* loop */
  while (running) {

    /* clear strings and get input */
    (void) memset(incoming, '\0', sizeof(incoming));
    (void) memset(outgoing, '\0', sizeof(outgoing));
    (void) printf("NG command> ");
    if ((p=fgets(outgoing, sizeof(outgoing), stdin)) == (char *)NULL || strlen(p)<=2) { 
      continue; 
    }

    running = false; // at this point, stdin has been received. No need to loop. Send and receive once.

    /* send to socket */
    (void) printf("Client handler to server: '%s'", incoming);
    if ((istat=send(handler_fd, outgoing, sizeof(outgoing), 0)) < 0) {
      (void) printf("Client handler send() failed\n");
      running = false;
      break;
    }

    /* get response */
    while ((istat=recv(handler_fd, incoming + bytes_read, sizeof(incoming) - bytes_read, 0)) > 0)
    {
      bytes_read += istat;

      if (strchr(incoming, 0) != NULL)
      {
        message_read = true;
        break; // terminating character (0) found. Incoming message is complete.
      }
    }

    if (istat < 0) {;
      (void) printf("Client handler recv() failed\n");
      running = false;
      break;
    }

    if (message_read)
    {
      (void) printf("Client handler from server: '%s'\n", incoming);
    }

    /* exit */
    if ((p=strstr(outgoing, " EXIT"))!=(char *)NULL && (p=strstr(incoming, " OK"))!=(char *)NULL) {
      (void) printf("Client handler exiting\n");
      running = false;
      break;
    }
  }
}


/*******************************************************************************
 * function: main()
 ******************************************************************************/
int main(int argc, char *argv[]) {

  while (true)
  {
    /* declare some variable(s) and initialize them */
    int socket_fd = 0;
    int connect_status = 0;
    struct sockaddr_in server_addr;

    /* socket create and verification */
    if ((socket_fd=socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      (void) printf("Client to server socket creation failed\n");
      exit(socket_fd);
    }

    /* assign IP, PORT */
    (void) memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(BOK_NG_PORT);
    server_addr.sin_addr.s_addr = inet_addr(BOK_NG_ADDR);

    /* connect the client socket to server socket */
    if ((connect_status=connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0) {
      printf("Client to server connection failed\n");
      /* close the socket */
      close(connect_status);
      close(socket_fd);
      return connect_status;
    }

    /* handler for chat */
    client_handler(socket_fd);

    /* close the socket */
    // close(connect_status); // ERROR: for some reason, this causes fgets() to hang when trying to read stdin
    close(socket_fd);
  }

  return 0;
}
