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
#define _HELP_  "NG server for the Galil_DMC_22x0 via gclib"
#undef _NAME_
#define _NAME_  "Galil_DMC_22x0_NgServer"


/*******************************************************************************
 * times for action(s)
 ******************************************************************************/
#define BOK_NG_GUIDER_FILTER_TIME      45
#define BOK_NG_GUIDER_FOCUS_TIME       30
#define BOK_NG_GUIDER_INIT_TIME        15
#define BOK_NG_INSTRUMENT_FILTER_TIME  45
#define BOK_NG_INSTRUMENT_FOCUS_TIME   30
#define BOK_NG_INSTRUMENT_INIT_TIME    15
#define BOK_NG_INSTRUMENT_LOAD_TIME    10
#define BOK_NG_INSTRUMENT_UNLOAD_TIME  10


/*******************************************************************************
 * function: thread_handler()
 ******************************************************************************/
void *thread_handler(void *thread_fd) {

  /* declare some variable(s) and initialize them */
  bool is_done = false;
  float fval = 0.0;
  float focus_a = 0.0;
  float focus_b = 0.0;
  float focus_c = 0.0;
  float lvdt_a = 0.0;
  float lvdt_b = 0.0;
  float lvdt_c = 0.0;
  GReturn gstat = (GReturn)0;
  int handler_fd = *(int *)thread_fd;
  int istat = 0;
  int rstat = 0;
  int wstat = 0;
  int ival = 0;
  unsigned int countdown = 0;
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
    (void) fprintf(stdout, "Server thread handler received from client: '%s'\n", incoming);
    (void) fflush(stdout);

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
       * BOK 90PRIME <cmd-id> COMMAND EXIT
       ******************************************************************************/
      if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
                 (istat=strncasecmp(bok_ng_commands[4], "EXIT", strlen("EXIT"))==0) ) {
        (void) strcat(outgoing, " OK\n");

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND GFILTER INIT
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
          (istat=strncasecmp(bok_ng_commands[4], "GFILTER", strlen("GFILTER"))==0) &&
          (istat=strncasecmp(bok_ng_commands[5], "INIT", strlen("INIT"))==0) ) {

        /* output message */
        (void) fprintf(stdout, "Server thread is initializing guider filter wheel\n");
        (void) fflush(stdout);

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          (void) sleep(BOK_NG_GUIDER_INIT_TIME);
          (void) strcat(outgoing, " OK\n");

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);

          /* check memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)\n");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)\n");

          /* execute */
          } else {
            (void) xq_hx();
            is_done = false;
            if ((gstat=xq_gfwinit())==G_NO_ERROR) {
              countdown = BOK_NG_GUIDER_INIT_TIME;
              while (--countdown > 0) {
                /* there is nothing to check on the initialization! */
                (void) sleep(1);
                (void) fprintf(stdout, "Server thread is waiting for gfwinit() to finish %d\n", countdown);
                (void) fflush(stdout);
              }
              is_done = true;
            }
            if (is_done) {
              (void) strcat(outgoing, " OK\n");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)\n");
            }
            (void) xq_hx();
          }

          /* close memory */
          if (tcp_shm_ptr != (tcp_val_p)NULL) { (void) munmap(tcp_shm_ptr, TCP_VAL_SIZE); }
          if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }
        }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND GFILTER NAME <str>
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
          (istat=strncasecmp(bok_ng_commands[4], "GFILTER", strlen("GFILTER"))==0) &&
          (istat=strncasecmp(bok_ng_commands[5], "NAME", strlen("NAME"))==0) &&
          (istat=(int)strlen(bok_ng_commands[6])>0) ) {

        /* convert name to number */
        ival = INT_MAX;
        for (int j=0; j<BOK_GFILTER_SLOTS; j++) {
          if ((istat=strncasecmp(bok_gfilters[j].name, bok_ng_commands[6], strlen(bok_ng_commands[6]))) == 0) {
            ival = j;
            break;
          }
        }

        /* output message */
        (void) fprintf(stdout, "Server thread is moving guider filter to '%s' (%d)\n", bok_ng_commands[6], ival);
        (void) fflush(stdout);

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          if (ival>0 && ival<BOK_GFILTER_SLOTS) {
            (void) sleep(BOK_NG_GUIDER_FILTER_TIME);
            (void) strcat(outgoing, " OK\n");
          } else {
            (void) strcat(outgoing, " ERROR (number out of range)\n");
          }

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);

          /* check memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)\n");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)\n");

          /* check gfilter wheel has been initialized */
          } else if ((int)abs(round(tcp_shm_ptr->lv.gfiltn)) > BOK_GFILTER_SLOTS) {
            (void) strcat(outgoing, " ERROR (guider filter wheel not initialized)\n");

          /* check decoded value is good */
          } else if (ival == INT_MIN) {
            (void) strcat(outgoing, " ERROR (number bad value)\n");

          /* check decoded value is valid */
          } else if (ival<0 || ival>BOK_GFILTER_SLOTS) {
            (void) strcat(outgoing, " ERROR (number out of range)\n");

          /* execute */
          } else {
            (void) xq_hx();
            is_done = false;
            if ((gstat=xq_gfiltn((float)ival))==G_NO_ERROR && (gstat=xq_gfwmov())==G_NO_ERROR) {
              countdown = BOK_NG_GUIDER_FILTER_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                (void) fprintf(stdout, "Server thread is checking gfiltn %d and snum %d\n",
                  (int)round(tcp_shm_ptr->lv.gfiltn), (int)round(tcp_shm_ptr->lv.snum));
                (void) fflush(stdout);
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==1 && (int)round(tcp_shm_ptr->lv.snum)==1) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==2 && (int)round(tcp_shm_ptr->lv.snum)==3) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==3 && (int)round(tcp_shm_ptr->lv.snum)==2) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==4 && (int)round(tcp_shm_ptr->lv.snum)==6) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==5 && (int)round(tcp_shm_ptr->lv.snum)==4) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==6 && (int)round(tcp_shm_ptr->lv.snum)==5) { is_done = true; break; }
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK\n");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)\n");
            }
            (void) xq_hx();
          }

          /* close memory */
          if (tcp_shm_ptr != (tcp_val_p)NULL) { (void) munmap(tcp_shm_ptr, TCP_VAL_SIZE); }
          if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }
        }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND GFILTER NUMBER <int>
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
          (istat=strncasecmp(bok_ng_commands[4], "GFILTER", strlen("GFILTER"))==0) &&
          (istat=strncasecmp(bok_ng_commands[5], "NUMBER", strlen("NUMBER"))==0) &&
          (istat=(int)strlen(bok_ng_commands[6])>0) ) {

        /* output message */
        decode_integer(bok_ng_commands[6], &ival);
        if (ival>0 && ival<BOK_GFILTER_SLOTS) {
          (void) fprintf(stdout, "Server thread is moving guider filter to %d ('%s')\n", ival, bok_gfilters[ival].name);
        } else {
          (void) fprintf(stdout, "Server thread is moving guider filter to %d\n", ival);
        }
        (void) fflush(stdout);

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          if (ival>0 && ival<BOK_GFILTER_SLOTS) {
            (void) sleep(BOK_NG_GUIDER_FILTER_TIME);
            (void) strcat(outgoing, " OK\n");
          } else {
            (void) strcat(outgoing, " ERROR (number out of range)\n");
          }

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);

          /* check memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)\n");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)\n");

          /* check gfilter wheel has been initialized */
          } else if ((int)abs(round(tcp_shm_ptr->lv.gfiltn)) > BOK_GFILTER_SLOTS) {
            (void) strcat(outgoing, " ERROR (guider filter wheel not initialized)\n");

          /* check decoded value is good */
          } else if (ival == INT_MIN) {
            (void) strcat(outgoing, " ERROR (number bad value)\n");

          /* check decoded value is valid */
          } else if (ival<0 || ival>BOK_GFILTER_SLOTS) {
            (void) strcat(outgoing, " ERROR (number out of range)\n");

          /* execute */
          } else {
            (void) xq_hx();
            is_done = false;
            if ((gstat=xq_gfiltn((float)ival))==G_NO_ERROR && (gstat=xq_gfwmov())==G_NO_ERROR) {
              countdown = BOK_NG_GUIDER_FILTER_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                (void) fprintf(stdout, "Server thread is checking gfiltn %d and snum %d\n",
                  (int)round(tcp_shm_ptr->lv.gfiltn), (int)round(tcp_shm_ptr->lv.snum));
                (void) fflush(stdout);
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==1 && (int)round(tcp_shm_ptr->lv.snum)==1) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==2 && (int)round(tcp_shm_ptr->lv.snum)==3) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==3 && (int)round(tcp_shm_ptr->lv.snum)==2) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==4 && (int)round(tcp_shm_ptr->lv.snum)==6) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==5 && (int)round(tcp_shm_ptr->lv.snum)==4) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==6 && (int)round(tcp_shm_ptr->lv.snum)==5) { is_done = true; break; }
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK\n");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)\n");
            }
            (void) xq_hx();
          }

          /* close memory */
          if (tcp_shm_ptr != (tcp_val_p)NULL) { (void) munmap(tcp_shm_ptr, TCP_VAL_SIZE); }
          if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }
        }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND GFOCUS DELTA <float>
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
          (istat=strncasecmp(bok_ng_commands[4], "GFOCUS", strlen("GFOCUS"))==0) &&
          (istat=strncasecmp(bok_ng_commands[5], "DELTA", strlen("DELTA"))==0) &&
          (istat=(int)strlen(bok_ng_commands[6])>0) ) {

        /* output message */
        decode_float(bok_ng_commands[6], &fval);
        (void) fprintf(stdout, "Server thread is moving guider focus by %.4f\n", fval);
        (void) fflush(stdout);

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          if (fval != NAN) {
            (void) sleep(BOK_NG_GUIDER_FOCUS_TIME);
            (void) strcat(outgoing, " OK\n");
          } else {
            (void) strcat(outgoing, " ERROR (number out of range)\n");
          }

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);
          udp_shm_fd = shm_open(BOK_SHM_UDP_NAME, O_RDONLY, 0666);
          udp_shm_ptr = (udp_val_p)mmap(0, UDP_VAL_SIZE, PROT_READ, MAP_SHARED, udp_shm_fd, 0);

          /* check tcp memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)\n");

          /* check udp memory */
          } else if (udp_shm_fd<0 || udp_shm_ptr==(udp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid udp memory)\n");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)\n");

          /* check input is valid */
          } else if (fval == NAN) {
            (void) strcat(outgoing, " ERROR (invalid number)\n");

          /* execute */
          } else {
            (void) xq_hx;
            is_done = false;
            float _gfocend = (float)udp_shm_ptr->eaxis_reference_position + fval;
            if ((gstat=xq_gfocus(fval)) == G_NO_ERROR) {
              countdown = BOK_NG_GUIDER_FOCUS_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                (void) fprintf(stdout, "Server thread is checking _gfocend %.4f with eaxis_reference_position %.4f\n",
                  _gfocend, (float)udp_shm_ptr->eaxis_reference_position);
                (void) fflush(stdout);
                if (abs(_gfocend - (float)udp_shm_ptr->eaxis_reference_position) < 0.1) { is_done = true; break; }
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK\n");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)\n");
            }
            (void) xq_hx;
          }

          /* close memory */
          if (tcp_shm_ptr != (tcp_val_p)NULL) { (void) munmap(tcp_shm_ptr, TCP_VAL_SIZE); }
          if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }
          if (udp_shm_ptr != (udp_val_p)NULL) { (void) munmap(udp_shm_ptr, UDP_VAL_SIZE); }
          if (udp_shm_fd >= 0) { (void) close(udp_shm_fd); }
        }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND IFILTER INIT
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
          (istat=strncasecmp(bok_ng_commands[4], "IFILTER", strlen("IFILTER"))==0) &&
          (istat=strncasecmp(bok_ng_commands[5], "INIT", strlen("INIT"))==0) ) {

        /* output message */
        (void) fprintf(stdout, "Server thread is initializing instrument filter wheel\n");
        (void) fflush(stdout);

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          (void) sleep(BOK_NG_INSTRUMENT_INIT_TIME);
          (void) strcat(outgoing, " OK\n");

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);

          /* check memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)\n");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)\n");

          /* cannot initialize when a filter is loaded */
          } else if (((int)round(tcp_shm_ptr->lv.filtisin)) == 1) {
              (void) strcat(outgoing, " ERROR (filter in beam)\n");

          /* execute */
          } else {
            is_done = false;
            if ((gstat=xq_hx())==G_NO_ERROR && (gstat=xq_filtldm())==G_NO_ERROR) {
              countdown = BOK_NG_INSTRUMENT_INIT_TIME;
              while (--countdown > 0) {
                /* there is nothing to check on the initialization! */
                (void) sleep(1);
                (void) fprintf(stdout, "Server thread is waiting for filtldm() to finish %d\n", countdown);
                (void) fflush(stdout);
              }
            }
            if ((gstat=xq_hx())==G_NO_ERROR && (gstat=xq_filtrd())==G_NO_ERROR) {
              countdown = BOK_NG_INSTRUMENT_FILTER_TIME;
              while (--countdown > 0) {
                /* there is nothing to check on the initialization! */
                (void) sleep(1);
                (void) fprintf(stdout, "Server thread is waiting for filtrd() to finish %d\n", countdown);
                (void) fflush(stdout);
              }
              is_done = true;
              //countdown = BOK_NG_INSTRUMENT_INIT_TIME;
              //while (--countdown > 0) {
              //  (void) sleep(1);
              //  (void) fprintf(stdout, "Server thread is checking initfilt %d\n", (int)round(tcp_shm_ptr->lv.initfilt));
              //  (void) fflush(stdout);
              //  if (((int)round(tcp_shm_ptr->lv.initfilt)) == 1) { is_done = true; break; }
              //}
            }
            if (is_done) {
              (void) strcat(outgoing, " OK\n");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)\n");
            }
          }

          /* close memory */
          if (tcp_shm_ptr != (tcp_val_p)NULL) { (void) munmap(tcp_shm_ptr, TCP_VAL_SIZE); }
          if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }
        }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND IFILTER LOAD
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
          (istat=strncasecmp(bok_ng_commands[4], "IFILTER", strlen("IFILTER"))==0) &&
          (istat=strncasecmp(bok_ng_commands[5], "LOAD", strlen("LOAD"))==0) ) {

        /* output message */
        (void) fprintf(stdout, "Server thread is loading instrument filter\n");
        (void) fflush(stdout);

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          (void) sleep(BOK_NG_INSTRUMENT_LOAD_TIME);
          (void) strcat(outgoing, " OK\n");

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);

          /* check memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)\n");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)\n");

          /* do nothing if filter already loaded */
          } else if (((int)round(tcp_shm_ptr->lv.filtisin)) == 1) {
              (void) strcat(outgoing, " OK\n");

          /* talk to hardware */
          } else {
            is_done = false;
            if ((gstat=xq_hx())==G_NO_ERROR && (gstat=xq_filtin())==G_NO_ERROR) {
              countdown = BOK_NG_INSTRUMENT_LOAD_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                (void) fprintf(stdout, "Server thread is checking filtisin %d\n", (int)round(tcp_shm_ptr->lv.filtisin));
                (void) fflush(stdout);
                if (((int)round(tcp_shm_ptr->lv.filtisin)) == 1) { is_done = true; break; }
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK\n");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)\n");
            }
          }

          /* close memory */
          if (tcp_shm_ptr != (tcp_val_p)NULL) { (void) munmap(tcp_shm_ptr, TCP_VAL_SIZE); }
          if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }
        }

       /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND IFILTER NAME <str>
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
          (istat=strncasecmp(bok_ng_commands[4], "IFILTER", strlen("IFILTER"))==0) &&
          (istat=strncasecmp(bok_ng_commands[5], "NAME", strlen("NAME"))==0) &&
          (istat=(int)strlen(bok_ng_commands[6])>0) ) {

        /* convert name to number */
        ival = INT_MAX;
        for (int j=0; j<BOK_IFILTER_SLOTS; j++) {
          if ((istat=strncasecmp(bok_ifilters[j].name, bok_ng_commands[6], strlen(bok_ng_commands[6]))) == 0) {
            ival = j;
            break;
          }
        }

        /* output message */
        (void) fprintf(stdout, "Server thread is moving instrument filter to '%s' (%d)\n", bok_ng_commands[6], ival);
        (void) fflush(stdout);

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          if (ival>0 && ival<BOK_IFILTER_SLOTS) {
            (void) sleep(BOK_NG_INSTRUMENT_FILTER_TIME);
            (void) strcat(outgoing, " OK\n");
          } else {
            (void) strcat(outgoing, " ERROR (number out of range)\n");
          }

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);

          /* check memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)\n");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)\n");

          /* check filter wheel has been initialized */
          } else if ((int)abs(round(tcp_shm_ptr->lv.initfilt)) != 1) {
            (void) strcat(outgoing, " ERROR (instrument filter wheel not initialized)\n");

          /* check filter wheel has no filter loaded */
          } else if ((int)abs(round(tcp_shm_ptr->lv.filtisin)) == 1) {
            (void) strcat(outgoing, " ERROR (instrument filter wheel loaded)\n");

          /* check decoded value is good */
          } else if (ival == INT_MIN) {
            (void) strcat(outgoing, " ERROR (number bad value)\n");

          /* check decoded value is valid */
          } else if (ival<0 || ival>BOK_IFILTER_SLOTS) {
            (void) strcat(outgoing, " ERROR (number out of range)\n");

          /* execute */
          } else {
            is_done = false;
            if ((gstat=xq_reqfilt((float)ival))==G_NO_ERROR && (gstat=xq_filtmov())==G_NO_ERROR) {
              countdown = BOK_NG_INSTRUMENT_FILTER_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                (void) fprintf(stdout, "Server thread is checking reqfilt %d and filtval %d\n",
                  (int)round(tcp_shm_ptr->lv.reqfilt), (int)round(tcp_shm_ptr->lv.filtval));
                (void) fflush(stdout);
                if ((int)round(tcp_shm_ptr->lv.reqfilt) == (int)round(tcp_shm_ptr->lv.filtval)) { is_done = true; break; }
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK\n");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)\n");
            }
          }

          /* close memory */
          if (tcp_shm_ptr != (tcp_val_p)NULL) { (void) munmap(tcp_shm_ptr, TCP_VAL_SIZE); }
          if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }
        }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND IFILTER NUMBER <int>
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
          (istat=strncasecmp(bok_ng_commands[4], "IFILTER", strlen("IFILTER"))==0) &&
          (istat=strncasecmp(bok_ng_commands[5], "NUMBER", strlen("NUMBER"))==0) &&
          (istat=(int)strlen(bok_ng_commands[6])>0) ) {

        /* output message */
        decode_integer(bok_ng_commands[6], &ival);
        if (ival>0 && ival<BOK_IFILTER_SLOTS) {
          (void) fprintf(stdout, "Server thread is moving instrument filter to %d ('%s')\n", ival, bok_ifilters[ival].name);
        } else {
          (void) fprintf(stdout, "Server thread is moving instrument filter to %d\n", ival);
        }
        (void) fflush(stdout);

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          if (ival>0 && ival<BOK_IFILTER_SLOTS) {
            (void) sleep(BOK_NG_INSTRUMENT_FILTER_TIME);
            (void) strcat(outgoing, " OK\n");
          } else {
            (void) strcat(outgoing, " ERROR (number out of range)\n");
          }

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);

          /* check memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)\n");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)\n");

          /* check filter wheel has been initialized */
          } else if ((int)abs(round(tcp_shm_ptr->lv.initfilt)) != 1) {
            (void) strcat(outgoing, " ERROR (instrument filter wheel not initialized)\n");

          /* check filter wheel has no filter loaded */
          } else if ((int)abs(round(tcp_shm_ptr->lv.filtisin)) == 1) {
            (void) strcat(outgoing, " ERROR (instrument filter wheel loaded)\n");

          /* check decoded value is good */
          } else if (ival == INT_MIN) {
            (void) strcat(outgoing, " ERROR (number bad value)\n");

          /* check decoded value is valid */
          } else if (ival<0 || ival>BOK_IFILTER_SLOTS) {
            (void) strcat(outgoing, " ERROR (number out of range)\n");

          /* execute */
          } else {
            is_done = false;
            if ((gstat=xq_reqfilt((float)ival))==G_NO_ERROR && (gstat=xq_filtmov())==G_NO_ERROR) {
              countdown = BOK_NG_INSTRUMENT_FILTER_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                (void) fprintf(stdout, "Server thread is checking reqfilt %d and filtval %d\n",
                  (int)round(tcp_shm_ptr->lv.reqfilt), (int)round(tcp_shm_ptr->lv.filtval));
                (void) fflush(stdout);
                if ((int)round(tcp_shm_ptr->lv.reqfilt) == (int)round(tcp_shm_ptr->lv.filtval)) { is_done = true; break; }
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK\n");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)\n");
            }
          }

          /* close memory */
          if (tcp_shm_ptr != (tcp_val_p)NULL) { (void) munmap(tcp_shm_ptr, TCP_VAL_SIZE); }
          if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }
        }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND IFILTER UNLOAD
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
          (istat=strncasecmp(bok_ng_commands[4], "IFILTER", strlen("IFILTER"))==0) &&
          (istat=strncasecmp(bok_ng_commands[5], "UNLOAD", strlen("UNLOAD"))==0) ) {

        /* output message */
        (void) fprintf(stdout, "Server thread is unloading ifilter\n");
        (void) fflush(stdout);

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          (void) sleep(BOK_NG_INSTRUMENT_UNLOAD_TIME);
          (void) strcat(outgoing, " OK\n");

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);

          /* check memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)\n");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)\n");

          /* do nothing if filter already unloaded */
          } else if (((int)round(tcp_shm_ptr->lv.filtisin)) == 0) {
              (void) strcat(outgoing, " OK\n");

          /* talk to hardware */
          } else {
            is_done = false;
            if ((gstat=xq_hx())==G_NO_ERROR && (gstat=xq_filtout())==G_NO_ERROR) {
              countdown = BOK_NG_INSTRUMENT_UNLOAD_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                (void) fprintf(stdout, "Server thread is checking filtisin %d\n", (int)round(tcp_shm_ptr->lv.filtisin));
                (void) fflush(stdout);
                if (((int)round(tcp_shm_ptr->lv.filtisin)) == 0) { is_done = true; break; }
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK\n");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)\n");
            }
          }

          /* close memory */
          if (tcp_shm_ptr != (tcp_val_p)NULL) { (void) munmap(tcp_shm_ptr, TCP_VAL_SIZE); }
          if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }
        }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND IFOCUS A <float> B <float> C <float>
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
          (istat=strncasecmp(bok_ng_commands[4], "IFOCUS", strlen("IFOCUS"))==0) &&
          (istat=strncasecmp(bok_ng_commands[5], "A", strlen("A"))==0) &&
          (istat=(int)strlen(bok_ng_commands[6])>0) &&
          (istat=strncasecmp(bok_ng_commands[7], "B", strlen("B"))==0) &&
          (istat=(int)strlen(bok_ng_commands[8])>0) &&
          (istat=strncasecmp(bok_ng_commands[9], "C", strlen("C"))==0) &&
          (istat=(int)strlen(bok_ng_commands[10])>0) ) {

        /* output message */
        decode_float(bok_ng_commands[6], &focus_a);
        decode_float(bok_ng_commands[8], &focus_b);
        decode_float(bok_ng_commands[10], &focus_c);
        (void) fprintf(stdout, "Server thread is setting instrument focus to %.4f %.4f %.4f\n", focus_a, focus_b, focus_c);
        (void) fflush(stdout);

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          (void) sleep(BOK_NG_INSTRUMENT_FOCUS_TIME);
          (void) strcat(outgoing, " OK\n");

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);
          udp_shm_fd = shm_open(BOK_SHM_UDP_NAME, O_RDONLY, 0666);
          udp_shm_ptr = (udp_val_p)mmap(0, UDP_VAL_SIZE, PROT_READ, MAP_SHARED, udp_shm_fd, 0);

          /* check tcp memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)\n");

          /* check tcp memory */
          } else if (udp_shm_fd<0 || udp_shm_ptr==(udp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid udp memory)\n");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)\n");

          /* check inputs are valid */
          } else if (focus_a==NAN || focus_b==NAN || focus_c==NAN) {
            (void) strcat(outgoing, " ERROR (invalid number)\n");

          /* execute */
          } else {
            is_done = false;
            if ((gstat=xq_focusind(focus_a, focus_b, focus_c)) == G_NO_ERROR) {
              countdown = BOK_NG_INSTRUMENT_FOCUS_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                float vala = (float)udp_shm_ptr->baxis_analog_in * BOK_LVDT_STEPS;
                float valb = (float)udp_shm_ptr->daxis_analog_in * BOK_LVDT_STEPS;
                float valc = (float)udp_shm_ptr->faxis_analog_in * BOK_LVDT_STEPS;
                (void) fprintf(stdout, "Server thread is checking instrument focus a %.4f\n", vala);
                (void) fprintf(stdout, "Server thread is checking instrument focus b %.4f\n", valb);
                (void) fprintf(stdout, "Server thread is checking instrument focus c %.4f\n", valc);
                (void) fflush(stdout);
                /* ??? */
                is_done = true;
                break;
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK\n");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)\n");
            }
          }

          /* close memory */
          if (tcp_shm_ptr != (tcp_val_p)NULL) { (void) munmap(tcp_shm_ptr, TCP_VAL_SIZE); }
          if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }
          if (udp_shm_ptr != (udp_val_p)NULL) { (void) munmap(udp_shm_ptr, UDP_VAL_SIZE); }
          if (udp_shm_fd >= 0) { (void) close(udp_shm_fd); }
        }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND IFOCUSALL <float>
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
          (istat=strncasecmp(bok_ng_commands[4], "IFOCUSALL", strlen("IFOCUSALL"))==0) &&
          (istat=(int)strlen(bok_ng_commands[5])>0) ) {

        /* output message */
        decode_float(bok_ng_commands[5], &fval);
        (void) fprintf(stdout, "Server thread is setting instrument focusall to %.4f\n", fval);
        (void) fflush(stdout);

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          (void) sleep(BOK_NG_INSTRUMENT_FOCUS_TIME);
          (void) strcat(outgoing, " OK\n");

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);
          udp_shm_fd = shm_open(BOK_SHM_UDP_NAME, O_RDONLY, 0666);
          udp_shm_ptr = (udp_val_p)mmap(0, UDP_VAL_SIZE, PROT_READ, MAP_SHARED, udp_shm_fd, 0);

          /* check tcp memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)\n");

          /* check tcp memory */
          } else if (udp_shm_fd<0 || udp_shm_ptr==(udp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid udp memory)\n");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)\n");

          /* check inputs are valid */
          } else if (fval == NAN) {
            (void) strcat(outgoing, " ERROR (invalid number)\n");

          /* execute */
          } else {
            is_done = false;
            if ((gstat=xq_focusall(fval)) == G_NO_ERROR) {
              countdown = BOK_NG_INSTRUMENT_FOCUS_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                float vala = (float)udp_shm_ptr->baxis_analog_in * BOK_LVDT_STEPS;
                float valb = (float)udp_shm_ptr->daxis_analog_in * BOK_LVDT_STEPS;
                float valc = (float)udp_shm_ptr->faxis_analog_in * BOK_LVDT_STEPS;
                (void) fprintf(stdout, "Server thread is checking instrument focusall a %.4f\n", vala);
                (void) fprintf(stdout, "Server thread is checking instrument focusall b %.4f\n", valb);
                (void) fprintf(stdout, "Server thread is checking instrument focusall c %.4f\n", valc);
                (void) fflush(stdout);
                /* ??? */
                is_done = true;
                break;
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK\n");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)\n");
            }
          }

          /* close memory */
          if (tcp_shm_ptr != (tcp_val_p)NULL) { (void) munmap(tcp_shm_ptr, TCP_VAL_SIZE); }
          if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }
          if (udp_shm_ptr != (udp_val_p)NULL) { (void) munmap(udp_shm_ptr, UDP_VAL_SIZE); }
          if (udp_shm_fd >= 0) { (void) close(udp_shm_fd); }
        }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND LVDT A <float> B <float> C <float>
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
          (istat=strncasecmp(bok_ng_commands[4], "LVDT", strlen("LVDT"))==0) &&
          (istat=strncasecmp(bok_ng_commands[5], "A", strlen("A"))==0) &&
          (istat=(int)strlen(bok_ng_commands[6])>0) &&
          (istat=strncasecmp(bok_ng_commands[7], "B", strlen("B"))==0) &&
          (istat=(int)strlen(bok_ng_commands[8])>0) &&
          (istat=strncasecmp(bok_ng_commands[9], "C", strlen("C"))==0) &&
          (istat=(int)strlen(bok_ng_commands[10])>0) ) {

        /* output message */
        decode_float(bok_ng_commands[6], &lvdt_a);
        decode_float(bok_ng_commands[8], &lvdt_b);
        decode_float(bok_ng_commands[10], &lvdt_c);
        (void) fprintf(stdout, "Server thread is setting instrument lvdt to %.4f %.4f %.4f\n", lvdt_a, lvdt_b, lvdt_c);
        (void) fflush(stdout);

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          (void) sleep(BOK_NG_INSTRUMENT_FOCUS_TIME);
          (void) strcat(outgoing, " OK\n");

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);
          udp_shm_fd = shm_open(BOK_SHM_UDP_NAME, O_RDONLY, 0666);
          udp_shm_ptr = (udp_val_p)mmap(0, UDP_VAL_SIZE, PROT_READ, MAP_SHARED, udp_shm_fd, 0);

          /* check tcp memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)\n");

          /* check tcp memory */
          } else if (udp_shm_fd<0 || udp_shm_ptr==(udp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid udp memory)\n");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)\n");

          /* check inputs are valid */
          } else if (lvdt_a==NAN || lvdt_b==NAN || lvdt_c==NAN) {
            (void) strcat(outgoing, " ERROR (invalid number)\n");

          /* execute */
          } else {
            is_done = false;
            float vala = (float)udp_shm_ptr->baxis_analog_in * BOK_LVDT_STEPS;
            float valb = (float)udp_shm_ptr->daxis_analog_in * BOK_LVDT_STEPS;
            float valc = (float)udp_shm_ptr->faxis_analog_in * BOK_LVDT_STEPS;
            float dista = round((lvdt_a / 1000.0 - vala) * BOK_LVDT_ATOD);
            float distb = round((lvdt_b / 1000.0 - valb) * BOK_LVDT_ATOD);
            float distc = round((lvdt_c / 1000.0 - valc) * BOK_LVDT_ATOD);
            if ((gstat=xq_focusind(dista, distb, distc)) == G_NO_ERROR) {
              countdown = BOK_NG_INSTRUMENT_FOCUS_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                float vala = (float)udp_shm_ptr->baxis_analog_in * BOK_LVDT_STEPS;
                float valb = (float)udp_shm_ptr->daxis_analog_in * BOK_LVDT_STEPS;
                float valc = (float)udp_shm_ptr->faxis_analog_in * BOK_LVDT_STEPS;
                (void) fprintf(stdout, "Server thread is checking instrument focus a %.4f\n", vala);
                (void) fprintf(stdout, "Server thread is checking instrument focus b %.4f\n", valb);
                (void) fprintf(stdout, "Server thread is checking instrument focus c %.4f\n", valc);
                (void) fflush(stdout);
                /* ??? */
                is_done = true;
                break;
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK\n");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)\n");
            }
          }

          /* close memory */
          if (tcp_shm_ptr != (tcp_val_p)NULL) { (void) munmap(tcp_shm_ptr, TCP_VAL_SIZE); }
          if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }
          if (udp_shm_ptr != (udp_val_p)NULL) { (void) munmap(udp_shm_ptr, UDP_VAL_SIZE); }
          if (udp_shm_fd >= 0) { (void) close(udp_shm_fd); }
        }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND LVDTALL <float>
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
          (istat=strncasecmp(bok_ng_commands[4], "LVDTALL", strlen("LVDTALL"))==0) &&
          (istat=(int)strlen(bok_ng_commands[5])>0) ) {

        /* output message */
        decode_float(bok_ng_commands[5], &fval);
        (void) fprintf(stdout, "Server thread is setting instrument lvdtall to %.4f\n", fval);
        (void) fflush(stdout);

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          (void) sleep(BOK_NG_INSTRUMENT_FOCUS_TIME);
          (void) strcat(outgoing, " OK\n");

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);
          udp_shm_fd = shm_open(BOK_SHM_UDP_NAME, O_RDONLY, 0666);
          udp_shm_ptr = (udp_val_p)mmap(0, UDP_VAL_SIZE, PROT_READ, MAP_SHARED, udp_shm_fd, 0);

          /* check tcp memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)\n");

          /* check tcp memory */
          } else if (udp_shm_fd<0 || udp_shm_ptr==(udp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid udp memory)\n");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)\n");

          /* check inputs are valid */
          } else if (fval == NAN) {
            (void) strcat(outgoing, " ERROR (invalid number)\n");

          /* execute */
          } else {
            is_done = false;
            float distall = round((fval / 1000.0) * BOK_LVDT_ATOD);
            if ((gstat=xq_focusind(distall, distall, distall)) == G_NO_ERROR) {
              countdown = BOK_NG_INSTRUMENT_FOCUS_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                float vala = (float)udp_shm_ptr->baxis_analog_in * BOK_LVDT_STEPS;
                float valb = (float)udp_shm_ptr->daxis_analog_in * BOK_LVDT_STEPS;
                float valc = (float)udp_shm_ptr->faxis_analog_in * BOK_LVDT_STEPS;
                (void) fprintf(stdout, "Server thread is checking instrument focus a %.4f\n", vala);
                (void) fprintf(stdout, "Server thread is checking instrument focus b %.4f\n", valb);
                (void) fprintf(stdout, "Server thread is checking instrument focus c %.4f\n", valc);
                (void) fflush(stdout);
                /* ??? */
                is_done = true;
                break;
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK\n");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)\n");
            }
          }

          /* close memory */
          if (tcp_shm_ptr != (tcp_val_p)NULL) { (void) munmap(tcp_shm_ptr, TCP_VAL_SIZE); }
          if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }
          if (udp_shm_ptr != (udp_val_p)NULL) { (void) munmap(udp_shm_ptr, UDP_VAL_SIZE); }
          if (udp_shm_fd >= 0) { (void) close(udp_shm_fd); }
        }

       /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND TEST
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
                 (istat=strncasecmp(bok_ng_commands[4], "TEST", strlen("TEST"))==0) ) {
        (void) strcat(outgoing, " TEST OK\n");

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
       * BOK 90PRIME <cmd-id> REQUEST GFILTERS
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_REQUEST, strlen(BOK_NG_REQUEST))==0) &&
                 (istat=strncasecmp(bok_ng_commands[4], "GFILTERS", strlen("GFILTERS"))==0) ) {

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
          (void) sprintf(buffer, " OK GFOCUS=%.4f\n", (float)udp_shm_ptr->eaxis_reference_position);
          (void) strcat(outgoing, buffer);
        }

        /* close memory */
        if (udp_shm_ptr != (udp_val_p)NULL) { (void) munmap(udp_shm_ptr, UDP_VAL_SIZE); }
        if (udp_shm_fd >= 0) { (void) close(udp_shm_fd); }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> REQUEST IFILTERS
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_REQUEST, strlen(BOK_NG_REQUEST))==0) &&
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
        if (udp_shm_ptr != (udp_val_p)NULL) { (void) munmap(udp_shm_ptr, UDP_VAL_SIZE); }
        if (udp_shm_fd >= 0) { (void) close(udp_shm_fd); }

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
          (void) sprintf(buffer, " OK A=%.4f B=%.4f C=%.4f MEAN=%.4f\n",
            (float)udp_shm_ptr->baxis_analog_in*BOK_LVDT_STEPS, (float)udp_shm_ptr->daxis_analog_in*BOK_LVDT_STEPS,
            (float)udp_shm_ptr->faxis_analog_in*BOK_LVDT_STEPS,
            ((float)udp_shm_ptr->baxis_analog_in*BOK_LVDT_STEPS + (float)udp_shm_ptr->daxis_analog_in*BOK_LVDT_STEPS +
            (float)udp_shm_ptr->faxis_analog_in*BOK_LVDT_STEPS)/3.0);
          (void) strcat(outgoing, buffer);
        }

        /* close memory */
        if (udp_shm_ptr != (udp_val_p)NULL) { (void) munmap(udp_shm_ptr, UDP_VAL_SIZE); }
        if (udp_shm_fd >= 0) { (void) close(udp_shm_fd); }

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
    (void) printf("Server thread handler sent to client: '%s'", outgoing);

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

  char clientname[BOK_STR_256];
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;
  (void) memset(&clientname, '\0', sizeof(clientname));
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
    } else {
      (void) printf("Server pthread_create() success\n");
      (void) printf("Server handling client '%s'\n", inet_ntop(AF_INET, &client_addr.sin_addr, clientname, sizeof(clientname)));
    }
  }
  if (client_fd < 0) {
    (void) printf("Server accept() failed\n");
    return istat;
  }

  /* return */  
  return 0;
}
