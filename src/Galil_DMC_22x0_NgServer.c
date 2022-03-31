/*******************************************************************************
 *
 * Galil_DMC_22x0_NgServer.c
 *
 ******************************************************************************/


/*******************************************************************************
 * include(s)
 ******************************************************************************/
#include "bokGalil.h"
#include <math.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>


/*******************************************************************************
 * define(s)
 ******************************************************************************/
#undef _HELP_
#define _HELP_      "NG server for the Galil_DMC_22x0 via gclib"
#undef _NAME_
#define _NAME_      "Galil_DMC_22x0_NgServer"


/*******************************************************************************
 * function: thread_handler()
 ******************************************************************************/
void *thread_handler(void *thread_fd) {

  /* declare some variable(s) and initialuze them */
  int handler_fd = *(int *)thread_fd;
  int istat = 0;
  int rstat = 0;
  int wstat = 0;
  unsigned int cstat = 0;
  char *p = (char *)NULL;
  char *bok_ng_commands[BOK_NG_BUCKETS];
  filter_file_t bok_ifilters[BOK_IFILTER_SLOTS];
  filter_file_t bok_gfilters[BOK_GFILTER_SLOTS];

  int tcp_shm_fd = -1;
  tcp_val_p tcp_shm_ptr = (tcp_val_p)NULL;

  char iname[BOK_NG_STRING];
  char gname[BOK_NG_STRING];
  char incoming[BOK_NG_STRING];
  char outgoing[BOK_NG_STRING];
  (void) memset(iname, '\0', sizeof(iname));
  (void) memset(gname, '\0', sizeof(gname));
  (void) memset(incoming, '\0', sizeof(incoming));
  (void) memset(outgoing, '\0', sizeof(outgoing));

  /* initialize */
  cliMalloc(BOK_NG_BUCKETS, BOK_NG_WORD, bok_ng_commands);

  /* loop */
  while ((rstat=recv(handler_fd, incoming, sizeof(incoming), 0)) > 0) {

    /* report incoming */
    if ((p=strchr(incoming, '\n')) != (char *)NULL) { *p = '\0'; }
    if ((p=strchr(incoming, '\r')) != (char *)NULL) { *p = '\0'; }
    (void) printf("Server thread handler received from client: '%s'\n", incoming);

    /* parse */
    cliInit(BOK_NG_BUCKETS, BOK_NG_WORD, bok_ng_commands);
    cliParse(" ", incoming, BOK_NG_STRING, &cstat, BOK_NG_BUCKETS, bok_ng_commands);

    /* generate response */
    if ((istat=strncasecmp(bok_ng_commands[0], BOK_NG_TELESCOPE, strlen(BOK_NG_TELESCOPE))) != 0) {
      (void) strcat(outgoing, "ERROR (invalid telescope)");
    } else if ((istat=strncasecmp(bok_ng_commands[1], BOK_NG_INSTRUMENT, strlen(BOK_NG_INSTRUMENT))) != 0) {
      (void) strcat(outgoing, "ERROR (invalid instrument)");
    } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))!=0) && 
               (istat=strncasecmp(bok_ng_commands[3], BOK_NG_REQUEST, strlen(BOK_NG_REQUEST))!=0) ) {
      (void) strcat(outgoing, "ERROR (invalid command)");
    } else {

      /* initialize response */
      (void) sprintf(outgoing, "%s %s %s", bok_ng_commands[0], bok_ng_commands[1], bok_ng_commands[2]);

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> REQUEST IFILTERS
       ******************************************************************************/
      if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_REQUEST, strlen(BOK_NG_REQUEST))==0) &&
          (istat=strncasecmp(bok_ng_commands[4], "IFILTERS", strlen("IFILTERS"))==0) ) {

           /* read memory */
           if ((tcp_shm_fd=shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666))<0 ||
               (tcp_shm_ptr=(tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0))==(tcp_val_p)NULL) {
               (void) strcat(outgoing, " ERROR (invalid tcp memory)");
           } else {

             /* read filter(s) file */
             (void) memset(iname, '\0', sizeof(iname));
             if ((p=getenv("BOK_GALIL_DOCS")) == (char *)NULL) {
               (void) sprintf(iname, "%s", BOK_IFILTER_FILE);
             } else {
               (void) sprintf(iname, "%s/%s", p, BOK_IFILTER_FILE);
             }
             for (int i=0; i<BOK_IFILTER_SLOTS; i++) {(void) memset((void *)&bok_ifilters[i], 0, sizeof(filter_file_t));}
             read_filters_from_file(iname, (filter_file_t *)bok_ifilters, BOK_IFILTER_SLOTS, BOK_IFILTER_COLUMNS);

             /* report filters */
             (void) strcat(outgoing, " OK");
             for (int j=0; j<BOK_IFILTERS; j++) {
               (void) memset(iname, '\0', sizeof(iname));
               istat = (int)round(tcp_shm_ptr->filtvals[j]);
               (void) sprintf(iname, " %d:%d:'%s'", j, istat, bok_ifilters[istat].name);
               (void) strcat(outgoing, iname);
             }
             (void) strcat(outgoing, "\n");
           }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> REQUEST GFILTERS
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_REQUEST, strlen(BOK_NG_REQUEST))==0) &&
                 (istat=strncasecmp(bok_ng_commands[4], "GFILTERS", strlen("GFILTERS"))==0) ) {

           /* read memory */
           if ((tcp_shm_fd=shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666))<0 ||
               (tcp_shm_ptr=(tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0))==(tcp_val_p)NULL) {
               (void) strcat(outgoing, " ERROR (invalid tcp memory)");
           } else {

             /* read filter(s) file */
             (void) memset(gname, '\0', sizeof(gname));
             if ((p=getenv("BOK_GALIL_DOCS")) == (char *)NULL) {
               (void) sprintf(gname, "%s", BOK_GFILTER_FILE);
             } else {
               (void) sprintf(gname, "%s/%s", p, BOK_GFILTER_FILE);
             }
             for (int i=0; i<BOK_GFILTER_SLOTS; i++) {(void) memset((void *)&bok_gfilters[i], 0, sizeof(filter_file_t));}
             read_filters_from_file(gname, (filter_file_t *)bok_gfilters, BOK_GFILTER_SLOTS, BOK_GFILTER_COLUMNS);

             /* report filters */
             (void) strcat(outgoing, " OK");
             for (int j=0; j<BOK_GFILTERS; j++) {
               (void) memset(gname, '\0', sizeof(gname));
               (void) sprintf(gname, " %d:%d:'%s'", j+1, j+1, bok_gfilters[j+1].name);
               (void) strcat(outgoing, gname);
             }
             (void) strcat(outgoing, "\n");
           }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND DUMP
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[4], "DUMP", strlen("DUMP"))) == 0) {
        (void) strcat(outgoing, " DUMP OK\n");

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND EXIT
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[4], "EXIT", strlen("EXIT"))) == 0) {
        (void) strcat(outgoing, " EXIT OK\n");

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> ?????
       ******************************************************************************/
      } else {
        (void) sprintf(outgoing, "%s %s %s", bok_ng_commands[0], bok_ng_commands[1], bok_ng_commands[2]);
        (void) strcat(outgoing, " ERROR (unsupported command)\n");
      }
    }

    /* write response */
    if ((wstat=write(handler_fd, outgoing, strlen(outgoing))) < 0) {
      (void) printf("Server thread handler write() failed\n");
      break;
    }
    (void) printf("Server thread handler sent to client: '%s'\n", outgoing);

    /* reset string(s) */
    (void) memset(incoming, '\0', sizeof(incoming));
    (void) memset(outgoing, '\0', sizeof(outgoing));
  }
  if (rstat == 0) {
    (void) printf("Server thread handler client disconnected\n");
  }

  /* free memory and return */
  cliFree(BOK_NG_BUCKETS, bok_ng_commands);
  (void) free(thread_fd);
  return 0;
}


/*******************************************************************************
 * function: main()
 ******************************************************************************/
int main(int argc, char *argv[]) {

  /* declare some variable(s) and initialuze them */
  int socket_fd = 0;
  int client_fd = 0;
  int client_len = 0;
  int istat = 0;
  int *new_sock = (int *)NULL;

  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;
  (void) memset(&server_addr, '\0', sizeof(server_addr));
  (void) memset(&client_addr, '\0', sizeof(client_addr));

  /* socket create and verification */
  if ((socket_fd=socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    (void) printf("Server socket() failed\n");
    return socket_fd;
  }

  /* assign IP, PORT */
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(BOK_NG_PORT);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  /* bind socket */
  if ((istat=bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0) {
    (void) printf("Server socket bind() failed\n");
    return istat;
  }

  /* listen socket */
  printf("%s is listening for clients\n", _NAME_);
  if ((istat=listen(socket_fd, 5)) < 0) {
    (void) printf("Server listen() failed\n");
    return istat;
  }

  /* accept connection via thread */
  client_len = sizeof(client_addr);
  while ((client_fd=accept(socket_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_len))) {
    pthread_t this_thread;
    (void) memset(&this_thread, '\0', sizeof(this_thread));
    new_sock = malloc(1);
    *new_sock = client_fd;
    if ((istat=pthread_create(&this_thread, NULL, thread_handler, (void *)new_sock)) < 0) {
      (void) printf("Server pthread_create() failed\n");
      (void) free(new_sock);
      return istat;
    }
  }
  if (client_fd < 0) {
    (void) printf("Server accept() failed\n");
    return istat;
  }

  /* return */  
  return 0;
}
