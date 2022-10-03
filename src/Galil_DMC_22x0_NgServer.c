/*******************************************************************************
 *
 * Galil_DMC_22x0_NgServer.c
 *
 ******************************************************************************/


/*******************************************************************************
 * include(s)
 ******************************************************************************/
#include "__hosts__.h"
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
#define BOK_NG_GUIDER_FILTER_TIME      60
#define BOK_NG_GUIDER_FOCUS_TIME       30
#define BOK_NG_GUIDER_INIT_TIME        30
#define BOK_NG_INSTRUMENT_FILTER_TIME  75
#define BOK_NG_INSTRUMENT_FOCUS_TIME   30
#define BOK_NG_INSTRUMENT_INIT_TIME    30
#define BOK_NG_INSTRUMENT_LOAD_TIME    10
#define BOK_NG_INSTRUMENT_LVDT_TIME    30
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
  float tolerance = 0.0;
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
  filter_file_t bok_sfilters[BOK_SFILTER_SLOTS];

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
    (void) logtime(" received from client: '%s'\n", incoming);

    /* parse */
    cliInit(BOK_NG_BUCKETS, BOK_NG_WORD, bok_ng_commands);
    cliParse(" ", incoming, BOK_NG_STRING, &cstat, BOK_NG_BUCKETS, bok_ng_commands);

    /* initialize response */
    (void) sprintf(outgoing, "%s %s %s", bok_ng_commands[0], bok_ng_commands[1], bok_ng_commands[2]);

    /* handle command(s) */
    if ((istat=strncasecmp(bok_ng_commands[0], BOK_NG_TELESCOPE, strlen(BOK_NG_TELESCOPE))) != 0) {
      (void) strcat(outgoing, " ERROR (invalid telescope)");

    } else if ((istat=strncasecmp(bok_ng_commands[1], BOK_NG_INSTRUMENT, strlen(BOK_NG_INSTRUMENT))) != 0) {
      (void) strcat(outgoing, " ERROR (invalid instrument)");

    } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))!=0) &&
               (istat=strncasecmp(bok_ng_commands[3], BOK_NG_REQUEST, strlen(BOK_NG_REQUEST))!=0) ) {
      (void) strcat(outgoing, " ERROR (invalid command or request)");

    } else {

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND EXIT
       ******************************************************************************/
      if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
          (istat=strncasecmp(bok_ng_commands[4], "EXIT", strlen("EXIT"))==0) ) {
        (void) xq_hx();
        (void) strcat(outgoing, " OK");

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND GFILTER INIT
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
                 (istat=strncasecmp(bok_ng_commands[4], "GFILTER", strlen("GFILTER"))==0) &&
                 (istat=strncasecmp(bok_ng_commands[5], "INIT", strlen("INIT"))==0) ) {

        /* output message */
        (void) logtime(" initializing guider filter wheel\n");

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          (void) sleep(BOK_NG_GUIDER_INIT_TIME);
          (void) strcat(outgoing, " OK");

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);

          /* check memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)");

          /* execute */
          } else {
            is_done = false;
            (void) xq_hx();
            if ((gstat=xq_gfwinit())==G_NO_ERROR) {
              countdown = BOK_NG_GUIDER_INIT_TIME;
              while (--countdown > 0) {
                /* there is nothing to check on the initialization! */
                (void) sleep(1);
                (void) logtime(" waiting for gfwinit() to finish %d\n", countdown);
              }
            }
            (void) xq_hx();
            if ((gstat=xq_gfiltn(1.0))==G_NO_ERROR && (gstat=xq_gfwmov())==G_NO_ERROR) {
              countdown = BOK_NG_GUIDER_FILTER_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                (void) logtime(" checking gfiltn %d and snum %d\n", (int)round(tcp_shm_ptr->lv.gfiltn), (int)round(tcp_shm_ptr->lv.snum));
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==1 && (int)round(tcp_shm_ptr->lv.snum)==1) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==2 && (int)round(tcp_shm_ptr->lv.snum)==3) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==3 && (int)round(tcp_shm_ptr->lv.snum)==2) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==4 && (int)round(tcp_shm_ptr->lv.snum)==6) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==5 && (int)round(tcp_shm_ptr->lv.snum)==4) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==6 && (int)round(tcp_shm_ptr->lv.snum)==5) { is_done = true; break; }
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)");
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
        (void) logtime(" moving guider filter to '%s' (%d)\n", bok_ng_commands[6], ival);

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          if (ival>0 && ival<BOK_GFILTER_SLOTS) {
            (void) sleep(BOK_NG_GUIDER_FILTER_TIME);
            (void) strcat(outgoing, " OK");
          } else {
            (void) strcat(outgoing, " ERROR (number out of range)");
          }

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);

          /* check memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)");

          /* check gfilter wheel has been initialized */
          } else if ((int)abs(round(tcp_shm_ptr->lv.gfiltn)) > BOK_GFILTER_SLOTS) {
            (void) strcat(outgoing, " ERROR (guider filter wheel not initialized)");

          /* check decoded value is good */
          } else if (ival == INT_MIN) {
            (void) strcat(outgoing, " ERROR (number bad value)");

          /* check decoded value is valid */
          } else if (ival<0 || ival>BOK_GFILTER_SLOTS) {
            (void) strcat(outgoing, " ERROR (number out of range)");

          /* execute */
          } else {
            is_done = false;
            (void) xq_hx();
            if ((gstat=xq_gfiltn((float)ival))==G_NO_ERROR && (gstat=xq_gfwmov())==G_NO_ERROR) {
              countdown = BOK_NG_GUIDER_FILTER_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                (void) logtime(" checking gfiltn %d and snum %d\n", (int)round(tcp_shm_ptr->lv.gfiltn), (int)round(tcp_shm_ptr->lv.snum));
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==1 && (int)round(tcp_shm_ptr->lv.snum)==1) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==2 && (int)round(tcp_shm_ptr->lv.snum)==3) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==3 && (int)round(tcp_shm_ptr->lv.snum)==2) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==4 && (int)round(tcp_shm_ptr->lv.snum)==6) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==5 && (int)round(tcp_shm_ptr->lv.snum)==4) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==6 && (int)round(tcp_shm_ptr->lv.snum)==5) { is_done = true; break; }
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)");
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
          (void) logtime(" moving guider filter to %d ('%s')\n", ival, bok_gfilters[ival].name);
        } else {
          (void) logtime(" moving guider filter to %d\n", ival);
        }

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          if (ival>0 && ival<BOK_GFILTER_SLOTS) {
            (void) sleep(BOK_NG_GUIDER_FILTER_TIME);
            (void) strcat(outgoing, " OK");
          } else {
            (void) strcat(outgoing, " ERROR (number out of range)");
          }

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);

          /* check memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)");

          /* check gfilter wheel has been initialized */
          } else if ((int)abs(round(tcp_shm_ptr->lv.gfiltn)) > BOK_GFILTER_SLOTS) {
            (void) strcat(outgoing, " ERROR (guider filter wheel not initialized)");

          /* check decoded value is good */
          } else if (ival == INT_MIN) {
            (void) strcat(outgoing, " ERROR (number bad value)");

          /* check decoded value is valid */
          } else if (ival<0 || ival>BOK_GFILTER_SLOTS) {
            (void) strcat(outgoing, " ERROR (number out of range)");

          /* execute */
          } else {
            is_done = false;
            (void) xq_hx();
            if ((gstat=xq_gfiltn((float)ival))==G_NO_ERROR && (gstat=xq_gfwmov())==G_NO_ERROR) {
              countdown = BOK_NG_GUIDER_FILTER_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                (void) logtime(" checking gfiltn %d and snum %d\n", (int)round(tcp_shm_ptr->lv.gfiltn), (int)round(tcp_shm_ptr->lv.snum));
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==1 && (int)round(tcp_shm_ptr->lv.snum)==1) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==2 && (int)round(tcp_shm_ptr->lv.snum)==3) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==3 && (int)round(tcp_shm_ptr->lv.snum)==2) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==4 && (int)round(tcp_shm_ptr->lv.snum)==6) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==5 && (int)round(tcp_shm_ptr->lv.snum)==4) { is_done = true; break; }
                if ((int)round(tcp_shm_ptr->lv.gfiltn)==6 && (int)round(tcp_shm_ptr->lv.snum)==5) { is_done = true; break; }
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)");
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
        (void) logtime(" moving guider focus by %.4f\n", fval);

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          if (fval != NAN) {
            (void) sleep(BOK_NG_GUIDER_FOCUS_TIME);
            (void) strcat(outgoing, " OK");
          } else {
            (void) strcat(outgoing, " ERROR (number out of range)");
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
            (void) strcat(outgoing, " ERROR (invalid tcp memory)");

          /* check udp memory */
          } else if (udp_shm_fd<0 || udp_shm_ptr==(udp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid udp memory)");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)");

          /* check input is valid */
          } else if (fval == NAN) {
            (void) strcat(outgoing, " ERROR (invalid number)");

          /* execute */
          } else {
            is_done = false;
            float gfocend = (float)udp_shm_ptr->eaxis_reference_position + fval;
            (void) xq_hx;
            if ((gstat=xq_gfocus(fval)) == G_NO_ERROR) {
              countdown = BOK_NG_GUIDER_FOCUS_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                (void) logtime(" checking gfocend %.4f with eaxis_reference_position %.4f\n", gfocend, (float)udp_shm_ptr->eaxis_reference_position);
                if (abs(gfocend - (float)udp_shm_ptr->eaxis_reference_position) < 0.1) { is_done = true; break; }
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)");
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
        (void) logtime(" initializing instrument filter wheel\n");

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          (void) sleep(BOK_NG_INSTRUMENT_INIT_TIME);
          (void) strcat(outgoing, " OK");

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);

          /* check memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)");

          /* cannot initialize when a filter is loaded */
          } else if (((int)round(tcp_shm_ptr->lv.filtisin)) == 1) {
              (void) strcat(outgoing, " ERROR (filter in beam)");

          /* execute */
          } else {
            is_done = false;
            (void) xq_hx();
            if ((gstat=xq_filtldm()) == G_NO_ERROR) {
              countdown = BOK_NG_INSTRUMENT_INIT_TIME;
              while (--countdown > 0) {
                /* there is nothing to check on the initialization! */
                (void) sleep(1);
                (void) logtime(" waiting for filtldm() to finish %d\n", countdown);
              }
            }
            (void) xq_hx();
            if ((gstat=xq_filtrd()) == G_NO_ERROR) {
              countdown = BOK_NG_INSTRUMENT_FILTER_TIME;
              while (--countdown > 0) {
                /* there is nothing to check on the initialization! */
                (void) sleep(1);
                (void) logtime(" waiting for filtrd() to finish %d\n", countdown);
              }
              is_done = true;
            }
            if (is_done) {
              (void) strcat(outgoing, " OK");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)");
            }
            (void) xq_hx();
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
        (void) logtime(" loading instrument filter\n");

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          (void) sleep(BOK_NG_INSTRUMENT_LOAD_TIME);
          (void) strcat(outgoing, " OK");

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);

          /* check memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)");

          /* do nothing if filter already loaded */
          } else if (((int)round(tcp_shm_ptr->lv.filtisin)) == 1) {
              (void) strcat(outgoing, " OK");

          /* talk to hardware */
          } else {
            is_done = false;
            (void) xq_hx();
            if ((gstat=xq_filtin()) == G_NO_ERROR) {
              countdown = BOK_NG_INSTRUMENT_LOAD_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                (void) logtime(" checking filtisin %d\n", (int)round(tcp_shm_ptr->lv.filtisin));
                if (((int)round(tcp_shm_ptr->lv.filtisin)) == 1) { is_done = true; break; }
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)");
            }
            (void) xq_hx();
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
        (void) logtime(" moving instrument filter to '%s' (%d)\n", bok_ng_commands[6], ival);

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          if (ival>0 && ival<BOK_IFILTER_SLOTS) {
            (void) sleep(BOK_NG_INSTRUMENT_FILTER_TIME);
            (void) strcat(outgoing, " OK");
          } else {
            (void) strcat(outgoing, " ERROR (number out of range)");
          }

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);

          /* check memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)");

          /* check filter wheel has been initialized */
          } else if ((int)abs(round(tcp_shm_ptr->lv.initfilt)) != 1) {
            (void) strcat(outgoing, " ERROR (instrument filter wheel not initialized)");

          /* check filter wheel has no filter loaded */
          } else if ((int)abs(round(tcp_shm_ptr->lv.filtisin)) == 1) {
            (void) strcat(outgoing, " ERROR (instrument filter wheel loaded)");

          /* check decoded value is good */
          } else if (ival == INT_MIN) {
            (void) strcat(outgoing, " ERROR (number bad value)");

          /* check decoded value is valid */
          } else if (ival<0 || ival>BOK_IFILTER_SLOTS) {
            (void) strcat(outgoing, " ERROR (number out of range)");

          /* execute */
          } else {
            is_done = false;
            (void) xq_hx();
            if ((gstat=xq_reqfilt((float)ival))==G_NO_ERROR && (gstat=xq_filtmov())==G_NO_ERROR) {
              countdown = BOK_NG_INSTRUMENT_FILTER_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                (void) logtime(" checking reqfilt %d and filtval %d\n", (int)round(tcp_shm_ptr->lv.reqfilt), (int)round(tcp_shm_ptr->lv.filtval));
                if ((int)round(tcp_shm_ptr->lv.reqfilt) == (int)round(tcp_shm_ptr->lv.filtval)) { is_done = true; break; }
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)");
            }
            (void) xq_hx();
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
          (void) logtime(" moving instrument filter to %d ('%s')\n", ival, bok_ifilters[ival].name);
        } else {
          (void) logtime(" moving instrument filter to %d\n", ival);
        }

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          if (ival>0 && ival<BOK_IFILTER_SLOTS) {
            (void) sleep(BOK_NG_INSTRUMENT_FILTER_TIME);
            (void) strcat(outgoing, " OK");
          } else {
            (void) strcat(outgoing, " ERROR (number out of range)");
          }

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);

          /* check memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)");

          /* check filter wheel has been initialized */
          } else if ((int)abs(round(tcp_shm_ptr->lv.initfilt)) != 1) {
            (void) strcat(outgoing, " ERROR (instrument filter wheel not initialized)");

          /* check filter wheel has no filter loaded */
          } else if ((int)abs(round(tcp_shm_ptr->lv.filtisin)) == 1) {
            (void) strcat(outgoing, " ERROR (instrument filter wheel loaded)");

          /* check decoded value is good */
          } else if (ival == INT_MIN) {
            (void) strcat(outgoing, " ERROR (number bad value)");

          /* check decoded value is valid */
          } else if (ival<0 || ival>BOK_IFILTER_SLOTS) {
            (void) strcat(outgoing, " ERROR (number out of range)");

          /* execute */
          } else {
            is_done = false;
            (void) xq_hx();
            if ((gstat=xq_reqfilt((float)ival))==G_NO_ERROR && (gstat=xq_filtmov())==G_NO_ERROR) {
              countdown = BOK_NG_INSTRUMENT_FILTER_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                (void) logtime(" checking reqfilt %d and filtval %d\n", (int)round(tcp_shm_ptr->lv.reqfilt), (int)round(tcp_shm_ptr->lv.filtval));
                if ((int)round(tcp_shm_ptr->lv.reqfilt) == (int)round(tcp_shm_ptr->lv.filtval)) { is_done = true; break; }
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)");
            }
            (void) xq_hx();
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
        (void) logtime(" unloading instrument filter\n");

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          (void) sleep(BOK_NG_INSTRUMENT_UNLOAD_TIME);
          (void) strcat(outgoing, " OK");

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);

          /* check memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)");

          /* do nothing if filter already unloaded */
          } else if (((int)round(tcp_shm_ptr->lv.filtisin)) == 0) {
              (void) strcat(outgoing, " OK");

          /* talk to hardware */
          } else {
            is_done = false;
            (void) xq_hx();
            if ((gstat=xq_filtout()) == G_NO_ERROR) {
              countdown = BOK_NG_INSTRUMENT_UNLOAD_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                (void) logtime(" checking filtisin %d\n", (int)round(tcp_shm_ptr->lv.filtisin));
                if (((int)round(tcp_shm_ptr->lv.filtisin)) == 0) { is_done = true; break; }
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)");
            }
            (void) xq_hx();
          }

          /* close memory */
          if (tcp_shm_ptr != (tcp_val_p)NULL) { (void) munmap(tcp_shm_ptr, TCP_VAL_SIZE); }
          if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }
        }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND IFOCUS A <float> B <float> C <float> T <float>
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
                 (istat=strncasecmp(bok_ng_commands[4], "IFOCUS", strlen("IFOCUS"))==0) &&
                 (istat=strncasecmp(bok_ng_commands[5], "A", strlen("A"))==0) &&
                 (istat=(int)strlen(bok_ng_commands[6])>0) &&
                 (istat=strncasecmp(bok_ng_commands[7], "B", strlen("B"))==0) &&
                 (istat=(int)strlen(bok_ng_commands[8])>0) &&
                 (istat=strncasecmp(bok_ng_commands[9], "C", strlen("C"))==0) &&
                 (istat=(int)strlen(bok_ng_commands[10])>0) &&
                 (istat=strncasecmp(bok_ng_commands[11], "T", strlen("T"))==0) &&
                 (istat=(int)strlen(bok_ng_commands[12])>0) ) {

        /* output message */
        decode_float(bok_ng_commands[6], &focus_a);
        decode_float(bok_ng_commands[8], &focus_b);
        decode_float(bok_ng_commands[10], &focus_c);
        decode_float(bok_ng_commands[12], &tolerance);
        (void) logtime(" setting instrument focus to %.4f %.4f %.4f within tolerance %.4f\n", focus_a, focus_b, focus_c, tolerance);

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          (void) sleep(BOK_NG_INSTRUMENT_FOCUS_TIME);
          (void) strcat(outgoing, " OK");

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);
          udp_shm_fd = shm_open(BOK_SHM_UDP_NAME, O_RDONLY, 0666);
          udp_shm_ptr = (udp_val_p)mmap(0, UDP_VAL_SIZE, PROT_READ, MAP_SHARED, udp_shm_fd, 0);

          /* check tcp memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)");

          /* check tcp memory */
          } else if (udp_shm_fd<0 || udp_shm_ptr==(udp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid udp memory)");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)");

          /* check inputs are valid */
          } else if (focus_a==NAN || focus_b==NAN || focus_c==NAN || tolerance==NAN) {
            (void) strcat(outgoing, " ERROR (invalid number)");

          /* execute */
          } else {
            is_done = false;
            float cur_a = round(((float)udp_shm_ptr->a_position * 1000.0));
            float cur_b = round(((float)udp_shm_ptr->b_position * 1000.0));
            float cur_c = round(((float)udp_shm_ptr->c_position * 1000.0));
            float dista = round((focus_a/1000.0 - cur_a/1000.0) * BOK_LVDT_ATOD);
            float distb = round((focus_b/1000.0 - cur_b/1000.0) * BOK_LVDT_ATOD);
            float distc = round((focus_c/1000.0 - cur_c/1000.0) * BOK_LVDT_ATOD);
            float new_a = focus_a;
            float new_b = focus_b;
            float new_c = focus_c;
            (void) logtime(" instrument focus cur_a %.4f new_a %.4f within tolerance %.4f\n", cur_a, new_a, tolerance);
            (void) logtime(" instrument focus cur_b %.4f new_b %.4f within tolerance %.4f\n", cur_b, new_b, tolerance);
            (void) logtime(" instrument focus cur_c %.4f new_c %.4f within tolerance %.4f\n", cur_c, new_c, tolerance);
            (void) logtime(" instrument focus focus_a %.4f dista %.4f\n", focus_a, dista); 
            (void) logtime(" instrument focus focus_b %.4f distb %.4f\n", focus_b, distb); 
            (void) logtime(" instrument focus focus_c %.4f distc %.4f\n", focus_c, distc); 
            (void) xq_hx();
            if ((gstat=xq_focusind(dista, distb, distc)) == G_NO_ERROR) {
              countdown = BOK_NG_INSTRUMENT_FOCUS_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                cur_a = round(((float)udp_shm_ptr->a_position * 1000.0));
                cur_b = round(((float)udp_shm_ptr->b_position * 1000.0));
                cur_c = round(((float)udp_shm_ptr->c_position * 1000.0));
                (void) logtime(" instrument focus cur_a %.4f new_a %.4f within tolerance %.4f\n", cur_a, new_a, tolerance);
                (void) logtime(" instrument focus cur_b %.4f new_b %.4f within tolerance %.4f\n", cur_b, new_b, tolerance);
                (void) logtime(" instrument focus cur_c %.4f new_c %.4f within tolerance %.4f\n", cur_c, new_c, tolerance);
                if ( (abs(round(cur_a - new_a)) <= tolerance) &&
                     (abs(round(cur_b - new_b)) <= tolerance) &&
                     (abs(round(cur_c - new_c)) <= tolerance) ) { is_done = true; break; }
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)");
            }
            (void) xq_hx();
          }

          /* close memory */
          if (tcp_shm_ptr != (tcp_val_p)NULL) { (void) munmap(tcp_shm_ptr, TCP_VAL_SIZE); }
          if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }
          if (udp_shm_ptr != (udp_val_p)NULL) { (void) munmap(udp_shm_ptr, UDP_VAL_SIZE); }
          if (udp_shm_fd >= 0) { (void) close(udp_shm_fd); }
        }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND IFOCUSALL <float> T <float>
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
                 (istat=strncasecmp(bok_ng_commands[4], "IFOCUS", strlen("IFOCUS"))==0) &&
                 (istat=strncasecmp(bok_ng_commands[5], "DELTA", strlen("DELTA"))==0) &&
                 (istat=(int)strlen(bok_ng_commands[6])>0) &&
                 (istat=strncasecmp(bok_ng_commands[7], "T", strlen("T"))==0) &&
                 (istat=(int)strlen(bok_ng_commands[8])>0) ) {

        /* output message */
        decode_float(bok_ng_commands[6], &fval);
        decode_float(bok_ng_commands[8], &tolerance);
        (void) logtime(" setting instrument focusall to %.4f within tolerance %.4f\n", fval, tolerance);

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          (void) sleep(BOK_NG_INSTRUMENT_FOCUS_TIME);
          (void) strcat(outgoing, " OK");

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);
          udp_shm_fd = shm_open(BOK_SHM_UDP_NAME, O_RDONLY, 0666);
          udp_shm_ptr = (udp_val_p)mmap(0, UDP_VAL_SIZE, PROT_READ, MAP_SHARED, udp_shm_fd, 0);

          /* check tcp memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)");

          /* check tcp memory */
          } else if (udp_shm_fd<0 || udp_shm_ptr==(udp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid udp memory)");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)");

          /* check inputs are valid */
          } else if (fval==NAN || tolerance==NAN) {
            (void) strcat(outgoing, " ERROR (invalid number)");

          /* execute */
          } else {
            is_done = false;
            float distall = round((fval / 1000.0) * BOK_LVDT_ATOD);
            float cur_a = round(((float)udp_shm_ptr->a_position * 1000.0));
            float cur_b = round(((float)udp_shm_ptr->b_position * 1000.0));
            float cur_c = round(((float)udp_shm_ptr->c_position * 1000.0));
            float new_a = cur_a + fval;
            float new_b = cur_b + fval;
            float new_c = cur_c + fval;
            (void) logtime(" instrument focus cur_a %.4f new_a %.4f within tolerance %.4f\n", cur_a, new_a, tolerance);
            (void) logtime(" instrument focus cur_b %.4f new_b %.4f within tolerance %.4f\n", cur_b, new_b, tolerance);
            (void) logtime(" instrument focus cur_c %.4f new_c %.4f within tolerance %.4f\n", cur_c, new_c, tolerance);
            (void) logtime(" instrument focus delta %.4f distall %.4f\n", fval, distall); 
            (void) xq_hx();
            if ((gstat=xq_focusind(distall, distall, distall)) == G_NO_ERROR) {
              countdown = BOK_NG_INSTRUMENT_FOCUS_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                cur_a = round(((float)udp_shm_ptr->a_position * 1000.0));
                cur_b = round(((float)udp_shm_ptr->b_position * 1000.0));
                cur_c = round(((float)udp_shm_ptr->c_position * 1000.0));
                (void) logtime(" instrument focus cur_a %.4f new_a %.4f within tolerance %.4f\n", cur_a, new_a, tolerance);
                (void) logtime(" instrument focus cur_b %.4f new_b %.4f within tolerance %.4f\n", cur_b, new_b, tolerance);
                (void) logtime(" instrument focus cur_c %.4f new_c %.4f within tolerance %.4f\n", cur_c, new_c, tolerance);
                if ( (abs(round(cur_a - new_a)) <= tolerance) &&
                     (abs(round(cur_b - new_b)) <= tolerance) &&
                     (abs(round(cur_c - new_c)) <= tolerance) ) { is_done = true; break; }
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)");
            }
            (void) xq_hx();
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
                 (istat=(int)strlen(bok_ng_commands[10])>0) &&
                 (istat=strncasecmp(bok_ng_commands[11], "T", strlen("T"))==0) &&
                 (istat=(int)strlen(bok_ng_commands[12])>0) ) {

        /* output message */
        decode_float(bok_ng_commands[6], &lvdt_a);
        decode_float(bok_ng_commands[8], &lvdt_b);
        decode_float(bok_ng_commands[10], &lvdt_c);
        decode_float(bok_ng_commands[12], &tolerance);
        (void) logtime(" setting instrument lvdt to %.4f %.4f %.4f within tolerance %.4f\n", lvdt_a, lvdt_b, lvdt_c, tolerance);

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          (void) sleep(BOK_NG_INSTRUMENT_LVDT_TIME);
          (void) strcat(outgoing, " OK");

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);
          udp_shm_fd = shm_open(BOK_SHM_UDP_NAME, O_RDONLY, 0666);
          udp_shm_ptr = (udp_val_p)mmap(0, UDP_VAL_SIZE, PROT_READ, MAP_SHARED, udp_shm_fd, 0);

          /* check tcp memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)");

          /* check tcp memory */
          } else if (udp_shm_fd<0 || udp_shm_ptr==(udp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid udp memory)");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)");

          /* check inputs are valid */
          } else if (lvdt_a==NAN || lvdt_b==NAN || lvdt_c==NAN || tolerance==NAN) {
            (void) strcat(outgoing, " ERROR (invalid number)");

          /* execute */
          } else {
            is_done = false;
            float vala = (float)udp_shm_ptr->baxis_analog_in * BOK_LVDT_STEPS;
            float valb = (float)udp_shm_ptr->daxis_analog_in * BOK_LVDT_STEPS;
            float valc = (float)udp_shm_ptr->faxis_analog_in * BOK_LVDT_STEPS;
            float dista = round((lvdt_a / 1000.0 - vala) * BOK_LVDT_ATOD);
            float distb = round((lvdt_b / 1000.0 - valb) * BOK_LVDT_ATOD);
            float distc = round((lvdt_c / 1000.0 - valc) * BOK_LVDT_ATOD);
            (void) xq_hx();
            if ((gstat=xq_focusind(dista, distb, distc)) == G_NO_ERROR) {
              countdown = BOK_NG_INSTRUMENT_LVDT_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                float vala = (float)udp_shm_ptr->baxis_analog_in * BOK_LVDT_STEPS;
                float valb = (float)udp_shm_ptr->daxis_analog_in * BOK_LVDT_STEPS;
                float valc = (float)udp_shm_ptr->faxis_analog_in * BOK_LVDT_STEPS;
                (void) logtime(" checking instrument focus a %.4f\n", vala);
                (void) logtime(" checking instrument focus b %.4f\n", valb);
                (void) logtime(" checking instrument focus c %.4f\n", valc);
                /* ??? */
                is_done = true;
                break;
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)");
            }
            (void) xq_hx();
          }

          /* close memory */
          if (tcp_shm_ptr != (tcp_val_p)NULL) { (void) munmap(tcp_shm_ptr, TCP_VAL_SIZE); }
          if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }
          if (udp_shm_ptr != (udp_val_p)NULL) { (void) munmap(udp_shm_ptr, UDP_VAL_SIZE); }
          if (udp_shm_fd >= 0) { (void) close(udp_shm_fd); }
        }

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> COMMAND LVDTALL <float> T <float>
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_COMMAND, strlen(BOK_NG_COMMAND))==0) &&
                 (istat=strncasecmp(bok_ng_commands[4], "LVDTALL", strlen("LVDTALL"))==0) &&
                 (istat=(int)strlen(bok_ng_commands[5])>0) &&
                 (istat=strncasecmp(bok_ng_commands[6], "T", strlen("T"))==0) &&
                 (istat=(int)strlen(bok_ng_commands[7])>0) ) {

        /* output message */
        decode_float(bok_ng_commands[5], &fval);
        decode_float(bok_ng_commands[7], &tolerance);
        (void) logtime(" setting instrument lvdtall to %.4f within tolerance %.4f\n", fval, tolerance);

        /* in simulation, wait and return success */
        if ((istat=strncasecmp(bok_ng_commands[2], "SIMULATE", strlen("SIMULATE"))) == 0) {
          (void) sleep(BOK_NG_INSTRUMENT_LVDT_TIME);
          (void) strcat(outgoing, " OK");

        /* talk to hardware */
        } else {

          /* read memory */
          tcp_shm_fd = shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666);
          tcp_shm_ptr = (tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0);
          udp_shm_fd = shm_open(BOK_SHM_UDP_NAME, O_RDONLY, 0666);
          udp_shm_ptr = (udp_val_p)mmap(0, UDP_VAL_SIZE, PROT_READ, MAP_SHARED, udp_shm_fd, 0);

          /* check tcp memory */
          if (tcp_shm_fd<0 || tcp_shm_ptr==(tcp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid tcp memory)");

          /* check tcp memory */
          } else if (udp_shm_fd<0 || udp_shm_ptr==(udp_val_p)NULL) {
            (void) strcat(outgoing, " ERROR (invalid udp memory)");

          /* check hardware is idle */
          } else if (IS_BIT_SET(tcp_shm_ptr->status, 7) == 1) {
            (void) strcat(outgoing, " ERROR (hardware busy)");

          /* check inputs are valid */
          } else if (fval==NAN || tolerance==NAN) {
            (void) strcat(outgoing, " ERROR (invalid number)");

          /* execute */
          } else {
            is_done = false;
            float distall = round((fval / 1000.0) * BOK_LVDT_ATOD);
            (void) xq_hx();
            if ((gstat=xq_focusind(distall, distall, distall)) == G_NO_ERROR) {
              countdown = BOK_NG_INSTRUMENT_LVDT_TIME;
              while (--countdown > 0) {
                (void) sleep(1);
                float vala = (float)udp_shm_ptr->baxis_analog_in * BOK_LVDT_STEPS;
                float valb = (float)udp_shm_ptr->daxis_analog_in * BOK_LVDT_STEPS;
                float valc = (float)udp_shm_ptr->faxis_analog_in * BOK_LVDT_STEPS;
                (void) logtime(" checking instrument focus a %.4f\n", vala);
                (void) logtime(" checking instrument focus b %.4f\n", valb);
                (void) logtime(" checking instrument focus c %.4f\n", valc);
                /* ??? */
                is_done = true;
                break;
              }
            }
            if (is_done) {
              (void) strcat(outgoing, " OK");
            } else {
              (void) strcat(outgoing, " ERROR (timeout or hardware unresponsive)");
            }
            (void) xq_hx();
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
        (void) xq_hx();
        (void) strcat(outgoing, " OK");

      /*******************************************************************************
       * BOK 90PRIME <cmd-id> REQUEST ENCODERS
       ******************************************************************************/
      } else if ((istat=strncasecmp(bok_ng_commands[3], BOK_NG_REQUEST, strlen(BOK_NG_REQUEST))==0) &&
                 (istat=strncasecmp(bok_ng_commands[4], "ENCODERS", strlen("ENCODERS"))==0) ) {

        /* read memory */
        udp_shm_fd = shm_open(BOK_SHM_UDP_NAME, O_RDONLY, 0666);
        udp_shm_ptr = (udp_val_p)mmap(0, UDP_VAL_SIZE, PROT_READ, MAP_SHARED, udp_shm_fd, 0);

        if (udp_shm_fd<0 || udp_shm_ptr==(udp_val_p)NULL) {
          (void) strcat(outgoing, " ERROR (invalid udp memory)");

        } else {

          /* report encoder(s) */
          (void) memset(buffer, '\0', sizeof(buffer));
          (void) sprintf(buffer, " OK A=%.4f B=%.4f C=%.4f", (float)udp_shm_ptr->aaxis_motor_position,
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
          (void) strcat(outgoing, " ERROR (invalid tcp memory)");

        } else if (udp_shm_fd<0 || udp_shm_ptr==(udp_val_p)NULL) {
          (void) strcat(outgoing, " ERROR (invalid udp memory)");

        } else {

          /* read filter(s) file */
          (void) memset(buffer, '\0', sizeof(buffer));
          if ((p=getenv("BOK_GALIL_DOCS")) == (char *)NULL) {
            (void) sprintf(buffer, "%s", BOK_SFILTER_FILE);
          } else {
            (void) sprintf(buffer, "%s/%s", p, BOK_SFILTER_FILE);
          }
          for (int i=0; i<BOK_SFILTER_SLOTS; i++) {(void) memset((void *)&bok_sfilters[i], 0, sizeof(filter_file_t));}
          read_filters_from_file(buffer, (filter_file_t *)bok_sfilters, BOK_SFILTER_SLOTS, BOK_SFILTER_COLUMNS);

          /* report filters */
          (void) memset(buffer, '\0', sizeof(buffer));
          istat = (int)round(tcp_shm_ptr->lv.snum);
          (void) sprintf(buffer, " OK SNUM=%d:%s ROTATING=%s",
            istat, bok_sfilters[istat].name,
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
          (void) strcat(outgoing, " ERROR (invalid udp memory)");

        } else {

          /* report gfocus(s) */
          (void) memset(buffer, '\0', sizeof(buffer));
          (void) sprintf(buffer, " OK GFOCUS=%.4f", (float)udp_shm_ptr->eaxis_reference_position);
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
          (void) strcat(outgoing, " ERROR (invalid tcp memory)");

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
          (void) strcat(outgoing, " ERROR (invalid tcp memory)");

        } else if (udp_shm_fd<0 || udp_shm_ptr==(udp_val_p)NULL) {
          (void) strcat(outgoing, " ERROR (invalid udp memory)");

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
          (void) sprintf(buffer, " OK FILTVAL=%d:%s INBEAM=%s ROTATING=%s TRANSLATING=%s ERRFILT=%d FILTTSC=%d",
            istat, bok_ifilters[istat].name,
            ((int)round(tcp_shm_ptr->lv.filtisin)==1 ? "True" : "False"),
            ((int)round(udp_shm_ptr->faxis_moving)==1 ? "True" : "False"),
            ((int)round(udp_shm_ptr->gaxis_moving)==1 ? "True" : "False"),
            (int)round(tcp_shm_ptr->lv.errfilt), (int)round(tcp_shm_ptr->lv.filttsc));
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
          (void) strcat(outgoing, " ERROR (invalid udp memory)");

        } else {

          /* report ifocus(s) */
          (void) memset(buffer, '\0', sizeof(buffer));
          (void) sprintf(buffer, " OK A=%.4f B=%.4f C=%.4f MEAN=%.4f",
            (float)udp_shm_ptr->a_position*1000.0,
            (float)udp_shm_ptr->b_position*1000.0,
            (float)udp_shm_ptr->c_position*1000.0,
            ((float)udp_shm_ptr->a_position*1000.0 + (float)udp_shm_ptr->b_position*1000.0 +
             (float)udp_shm_ptr->c_position*1000.0)/3.0);
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
        (void) strcat(outgoing, " ERROR (unsupported command)");
      }
    }

    /* write response */
    (void) logtime(" sent to client: '%s'\n", outgoing);
    (void) strcat(outgoing, "\n");
    if ((wstat=write(handler_fd, outgoing, strlen(outgoing))) < 0) {
      (void) printf("Server thread handler write() failed\n");
      break;
    }

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
    (void) logtime(" server socket() failed\n");
    return socket_fd;
  }

  /* assign IP, PORT */
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(BOK_NG_PORT);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  /* bind socket */
  if ((istat=bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0) {
    (void) logtime(" server socket bind() failed\n");
    return istat;
  }

  /* listen socket */
  printf("%s is listening for clients\n", _NAME_);
  if ((istat=listen(socket_fd, 5)) < 0) {
    (void) logtime(" server listen() failed\n");
    return istat;
  }

  /* accept connection via thread */
  client_len = sizeof(client_addr);
  while ((client_fd=accept(socket_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_len))) {
    pthread_t this_thread;
    (void) memset(&this_thread, '\0', sizeof(this_thread));
    new_sock = malloc(sizeof(int));
    *new_sock = client_fd;
    if ((istat=pthread_create(&this_thread, NULL, thread_handler, (void *)new_sock)) < 0) {
      (void) logtime(" server pthread_create() failed\n");
      (void) free(new_sock);
      return istat;
    } else {
      (void) logtime(" server pthread_create() success\n");
      (void) logtime(" server handling client '%s'\n", inet_ntop(AF_INET, &client_addr.sin_addr, clientname, sizeof(clientname)));
    }
  }
  if (client_fd < 0) {
    (void) logtime(" server accept() failed\n");
    return istat;
  }

  /* return */  
  return 0;
}
