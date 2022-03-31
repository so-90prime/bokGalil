/*******************************************************************************
 *
 * Galil_DMC_22x0_NgServer.c
 *
 * Request(s) supported:
 *   BOK 90PRIME <cmd-id> REQUEST IFILTERS - done
 *   BOK 90PRIME <cmd-id> REQUEST IFILTER  - done
 *   BOK 90PRIME <cmd-id> REQUEST GFILTERS - done
 *   BOK 90PRIME <cmd-id> REQUEST GFILTER  - done
 *   BOK 90PRIME <cmd-id> REQUEST ENCODERS - done
 *   BOK 90PRIME <cmd-id> REQUEST IFOCUS   - done
 *   BOK 90PRIME <cmd-id> REQUEST GFOCUS   - done
 *   BOK 90PRIME <cmd-id> COMMAND TEST     - done
 *   BOK 90PRIME <cmd-id> COMMAND EXIT     - done
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

  /* declare some variable(s) and initialize them */
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
  int udp_shm_fd = -1;
  tcp_val_p tcp_shm_ptr = (tcp_val_p)NULL;
  udp_val_p udp_shm_ptr = (udp_val_p)NULL;

  char buffer[BOK_NG_STRING];
  char incoming[BOK_NG_STRING];
  char outgoing[BOK_NG_STRING];
  (void) memset(buffer, '\0', sizeof(buffer));
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

    /* initialize response */
    (void) sprintf(outgoing, "%s %s %s", bok_ng_commands[0], bok_ng_commands[1], bok_ng_commands[2]);

    /* handle command(s) */
    if ((istat=strncasecmp(bok_ng_commands[0], BOK_NG_TELESCOPE, strlen(BOK_NG_TELESCOPE))) != 0) {
      (void) strcat(outgoing, " ERROR (invalid telescope)\n");

    } else if ((istat=strncasecmp(bok_ng_commands[1], BOK_NG_INSTRUMENT, strlen(BOK_NG_INSTRUMENT))) != 0) {
      (void) strcat(outgoing, " ERROR (invalid instrument)\n");

    } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))!=0) &&
               (istat=strncasecmp(bok_ng_commands[3], BOK_NG_REQUEST, strlen(BOK_NG_REQUEST))!=0) ) {
      (void) strcat(outgoing, " ERROR (invalid command or request)\n");

    } else {

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> REQUEST IFILTERS
       ******************************************************************************/
      if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_REQUEST, strlen(BOK_NG_REQUEST))==0) &&
          (istat=strncasecmp(bok_ng_commands[4], "IFILTERS", strlen("IFILTERS"))==0) ) {

       /* read memory */
       tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
       tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);

       if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
         (void) strcat(outgoing, " ERROR (invalid tcp memory)\n");
       } else {

         /* read filter(s) file */
         (void) memset(buffer, '\0', sizeof(buffer));
         if ((p=getenv("BOK_GALIL_DOCS")) == (char *)NULL) {
           (void) sprintf(buffer, "%s", BOK_IFILTER_FILE);
         } else {
           (void) sprintf(buffer, "%s/%s", p, BOK_IFILTER_FILE);
         }
         for (int i=0; i<BOK_IFILTER_SLOTS; i++) {(void) memset((void *)&bok_ifilters[i], 0, sizeof(filter_file_t));}
         read_filters_from_file(buffer, (filter_file_t *)bok_ifilters, BOK_IFILTER_SLOTS, BOK_IFILTER_COLUMNS);

         /* report filters */
         (void) strcat(outgoing, " OK");
         for (int j=0; j<BOK_IFILTERS; j++) {
           (void) memset(buffer, '\0', sizeof(buffer));
           istat = (int)round(tcp_shm_ptr->filtvals[j]);
           (void) sprintf(buffer, " %d=%d:%s", j, istat, bok_ifilters[istat].name);
           (void) strcat(outgoing, buffer);
         }
         (void) strcat(outgoing, "\n");
       }

       /* close memory */
       if (tcp_shm_ptr != (tcp_val_p)NULL) { (void) munmap(tcp_shm_ptr, TCP_VAL_SIZE); }
       if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> REQUEST IFILTER
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_REQUEST, strlen(BOK_NG_REQUEST))==0) &&
          (istat=strncasecmp(bok_ng_commands[4], "IFILTER", strlen("IFILTER"))==0) ) {

       /* read memory */
       tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
       tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);

       udp_shm_fd = shm_open(BOK_SHM_UDP_NAME, O_RDONLY, 0666);
       udp_shm_ptr = (udp_val_p)mmap(0, UDP_VAL_SIZE, PROT_READ, MAP_SHARED, udp_shm_fd, 0);

       if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
         (void) strcat(outgoing, " ERROR (invalid tcp memory)\n");
       } else if (udp_shm_fd<0 || udp_shm_ptr==(udp_val_p)NULL) {
         (void) strcat(outgoing, " ERROR (invalid udp memory)\n");
       } else {

         /* read filter(s) file */
         (void) memset(buffer, '\0', sizeof(buffer));
         if ((p=getenv("BOK_GALIL_DOCS")) == (char *)NULL) {
           (void) sprintf(buffer, "%s", BOK_IFILTER_FILE);
         } else {
           (void) sprintf(buffer, "%s/%s", p, BOK_IFILTER_FILE);
         }
         for (int i=0; i<BOK_IFILTER_SLOTS; i++) {(void) memset((void *)&bok_ifilters[i], 0, sizeof(filter_file_t));}
         read_filters_from_file(buffer, (filter_file_t *)bok_ifilters, BOK_IFILTER_SLOTS, BOK_IFILTER_COLUMNS);

         /* report filters */
         (void) memset(buffer, '\0', sizeof(buffer));
         istat = (int)round(tcp_shm_ptr->lv.filtval);
         (void) sprintf(buffer, " OK FILTVAL=%d:%s INBEAM=%s ROTATING=%s TRANSLATING=%s\n", 
           istat, bok_ifilters[istat].name, 
           ((int)round(tcp_shm_ptr->lv.filtisin)==1 ? "True" : "False"),
           ((int)round(udp_shm_ptr->faxis_moving)==1 ? "True" : "False"),
           ((int)round(udp_shm_ptr->gaxis_moving)==1 ? "True" : "False"));
         (void) strcat(outgoing, buffer);
       }
 
       /* close memory */
       if (tcp_shm_ptr != (tcp_val_p)NULL) { (void) munmap(tcp_shm_ptr, TCP_VAL_SIZE); }
       if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }
       if (udp_shm_ptr != (udp_val_p)NULL) { (void) munmap(udp_shm_ptr, UDP_VAL_SIZE); }
       if (udp_shm_fd >= 0) { (void) close(udp_shm_fd); }
 
      /*******************************************************************************
       * BOK 90PRIME <cmd-id> REQUEST GFILTERS
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_REQUEST, strlen(BOK_NG_REQUEST))==0) &&
                 (istat=strncasecmp(bok_ng_commands[4], "GFILTERS", strlen("GFILTERS"))==0) ) {

       /* read memory */
       tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
       tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);

       if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
         (void) strcat(outgoing, " ERROR (invalid tcp memory)\n");
       } else {

         /* read filter(s) file */
         (void) memset(buffer, '\0', sizeof(buffer));
         if ((p=getenv("BOK_GALIL_DOCS")) == (char *)NULL) {
           (void) sprintf(buffer, "%s", BOK_GFILTER_FILE);
         } else {
           (void) sprintf(buffer, "%s/%s", p, BOK_GFILTER_FILE);
         }
         for (int i=0; i<BOK_GFILTER_SLOTS; i++) {(void) memset((void *)&bok_gfilters[i], 0, sizeof(filter_file_t));}
         read_filters_from_file(buffer, (filter_file_t *)bok_gfilters, BOK_GFILTER_SLOTS, BOK_GFILTER_COLUMNS);

         /* report filters */
         (void) strcat(outgoing, " OK");
         for (int j=0; j<BOK_GFILTERS; j++) {
           (void) memset(buffer, '\0', sizeof(buffer));
           (void) sprintf(buffer, " %d=%d:%s", j+1, j+1, bok_gfilters[j+1].name);
           (void) strcat(outgoing, buffer);
         }
         (void) strcat(outgoing, "\n");
       }

       /* close memory */
       if (tcp_shm_ptr != (tcp_val_p)NULL) { (void) munmap(tcp_shm_ptr, TCP_VAL_SIZE); }
       if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> REQUEST GFILTER
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_REQUEST, strlen(BOK_NG_REQUEST))==0) &&
          (istat=strncasecmp(bok_ng_commands[4], "GFILTER", strlen("GFILTER"))==0) ) {

       /* read memory */
       tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
       tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);

       udp_shm_fd = shm_open(BOK_SHM_UDP_NAME, O_RDONLY, 0666);
       udp_shm_ptr = (udp_val_p)mmap(0, UDP_VAL_SIZE, PROT_READ, MAP_SHARED, udp_shm_fd, 0);

       if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
         (void) strcat(outgoing, " ERROR (invalid tcp memory)\n");
       } else if (udp_shm_fd<0 || udp_shm_ptr==(udp_val_p)NULL) {
         (void) strcat(outgoing, " ERROR (invalid udp memory)\n");
       } else {

         /* read filter(s) file */
         (void) memset(buffer, '\0', sizeof(buffer));
         if ((p=getenv("BOK_GALIL_DOCS")) == (char *)NULL) {
           (void) sprintf(buffer, "%s", BOK_GFILTER_FILE);
         } else {
           (void) sprintf(buffer, "%s/%s", p, BOK_GFILTER_FILE);
         }
         for (int i=0; i<BOK_GFILTER_SLOTS; i++) {(void) memset((void *)&bok_gfilters[i], 0, sizeof(filter_file_t));}
         read_filters_from_file(buffer, (filter_file_t *)bok_gfilters, BOK_GFILTER_SLOTS, BOK_GFILTER_COLUMNS);

         /* report filters */
         (void) memset(buffer, '\0', sizeof(buffer));
         istat = (int)round(tcp_shm_ptr->lv.gfiltn);
         (void) sprintf(buffer, " OK GFILTN=%d:%s ROTATING=%s\n", 
           istat, bok_gfilters[istat].name, 
           ((int)round(udp_shm_ptr->haxis_moving)==1 ? "True" : "False"));
         (void) strcat(outgoing, buffer);
       }
 
       /* close memory */
       if (tcp_shm_ptr != (tcp_val_p)NULL) { (void) munmap(tcp_shm_ptr, TCP_VAL_SIZE); }
       if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }
       if (udp_shm_ptr != (udp_val_p)NULL) { (void) munmap(udp_shm_ptr, UDP_VAL_SIZE); }
       if (udp_shm_fd >= 0) { (void) close(udp_shm_fd); }
 
      /*******************************************************************************
       * BOK 90PRIME <cmd-id> REQUEST ENCODERS
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_REQUEST, strlen(BOK_NG_REQUEST))==0) &&
                 (istat=strncasecmp(bok_ng_commands[4], "ENCODERS", strlen("ENCODERS"))==0) ) {

       /* read memory */
       udp_shm_fd = shm_open(BOK_SHM_UDP_NAME, O_RDONLY, 0666);
       udp_shm_ptr = (udp_val_p)mmap(0, UDP_VAL_SIZE, PROT_READ, MAP_SHARED, udp_shm_fd, 0);

       if (udp_shm_fd<0 || udp_shm_ptr==(udp_val_p)NULL) {
         (void) strcat(outgoing, " ERROR (invalid udp memory)\n");
       } else {

         /* report encoder(s) */
         (void) memset(buffer, '\0', sizeof(buffer));
         (void) sprintf(buffer, " OK A=%.4f B=%.4f C=%.4f\n", (float)udp_shm_ptr->aaxis_motor_position,
           (float)udp_shm_ptr->baxis_motor_position, (float)udp_shm_ptr->caxis_motor_position);
         (void) strcat(outgoing, buffer);
       }

       /* close memory */
       if (udp_shm_ptr != (udp_val_p)NULL) { (void) munmap(udp_shm_ptr, UDP_VAL_SIZE); }
       if (udp_shm_fd >= 0) { (void) close(udp_shm_fd); }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> REQUEST IFOCUS
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_REQUEST, strlen(BOK_NG_REQUEST))==0) &&
                 (istat=strncasecmp(bok_ng_commands[4], "IFOCUS", strlen("IFOCUS"))==0) ) {

       /* read memory */
       udp_shm_fd = shm_open(BOK_SHM_UDP_NAME, O_RDONLY, 0666);
       udp_shm_ptr = (udp_val_p)mmap(0, UDP_VAL_SIZE, PROT_READ, MAP_SHARED, udp_shm_fd, 0);

       if (udp_shm_fd<0 || udp_shm_ptr==(udp_val_p)NULL) {
         (void) strcat(outgoing, " ERROR (invalid udp memory)\n");
       } else {

         /* report ifocus(s) */
         (void) memset(buffer, '\0', sizeof(buffer));
         (void) sprintf(buffer, " OK A=%.4f B=%.4f C=%.4f\n",
           (float)udp_shm_ptr->baxis_analog_in*BOK_LVDT_STEPS, (float)udp_shm_ptr->daxis_analog_in*BOK_LVDT_STEPS,
           (float)udp_shm_ptr->faxis_analog_in*BOK_LVDT_STEPS);
         (void) strcat(outgoing, buffer);
       }

       /* close memory */
       if (udp_shm_ptr != (udp_val_p)NULL) { (void) munmap(udp_shm_ptr, UDP_VAL_SIZE); }
       if (udp_shm_fd >= 0) { (void) close(udp_shm_fd); }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> REQUEST GFOCUS
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_REQUEST, strlen(BOK_NG_REQUEST))==0) &&
                 (istat=strncasecmp(bok_ng_commands[4], "GFOCUS", strlen("GFOCUS"))==0) ) {

       /* read memory */
       udp_shm_fd = shm_open(BOK_SHM_UDP_NAME, O_RDONLY, 0666);
       udp_shm_ptr = (udp_val_p)mmap(0, UDP_VAL_SIZE, PROT_READ, MAP_SHARED, udp_shm_fd, 0);

       if (udp_shm_fd<0 || udp_shm_ptr==(udp_val_p)NULL) {
         (void) strcat(outgoing, " ERROR (invalid udp memory)\n");
       } else {

         /* report gfocus(s) */
         (void) memset(buffer, '\0', sizeof(buffer));
         (void) sprintf(buffer, " OK %.4f\n", (float)udp_shm_ptr->eaxis_reference_position);
         (void) strcat(outgoing, buffer);
       }

       /* close memory */
       if (udp_shm_ptr != (udp_val_p)NULL) { (void) munmap(udp_shm_ptr, UDP_VAL_SIZE); }
       if (udp_shm_fd >= 0) { (void) close(udp_shm_fd); }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND TEST
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
                 (istat=strncasecmp(bok_ng_commands[4], "TEST", strlen("TEST"))==0) ) {
        (void) strcat(outgoing, " TEST OK\n");

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND EXIT
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
                 (istat=strncasecmp(bok_ng_commands[4], "EXIT", strlen("EXIT"))==0) ) {
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

  /* declare some variable(s) and initialize them */
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
