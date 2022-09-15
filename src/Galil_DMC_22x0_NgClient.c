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
    if ((p=fgets(outgoing, sizeof(outgoing), stdin)) == (char *)NULL || strlen(p)<=2) { continue; }

    /* send to socket */
    (void) printf("Client handler to server: '%s'", incoming);
    if ((istat=send(handler_fd, outgoing, sizeof(outgoing), 0)) < 0) {
      (void) printf("Client handler send() failed\n");
      running = false;
      break;
    }

    /* get response */
    if ((istat=recv(handler_fd, incoming, sizeof(incoming), 0)) < 0) {;
      (void) printf("Client handler recv() failed\n");
      running = false;
      break;
    }
    (void) printf("Client handler from server: '%s'\n", incoming);

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

  /* declare some variable(s) and initialize them */
  int socket_fd = 0;
  int connection_fd = 0;
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
  server_addr.sin_addr.s_addr = inet_addr(BOK_NG_HOST);

  /* connect the client socket to server socket */
  if ((connection_fd=connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0) {
    printf("Client to server connection failed\n");
    close(socket_fd);
    exit(0);
  }

  /* handler for chat */
  client_handler(socket_fd);

  /* close the socket */
  close(connection_fd);
  close(socket_fd);
}
