/*******************************************************************************
 *
 * Galil_DMC_22x0_CLI.c
 *
 ******************************************************************************/


/*******************************************************************************
 * include(s)
 ******************************************************************************/
#include "Galil_DMC_22x0_CLI.h"


/*******************************************************************************
 * define(s)
 ******************************************************************************/
#undef _HELP_
#define _HELP_       "Executes command(s) for the Galil_DMC_22x0 via gclib"
#undef _NAME_
#define _NAME_       "Galil_DMC_22x0_CLI"


/*******************************************************************************
 * function: main()
 ******************************************************************************/
int main( int argc, char *argv[] ) {

  /* declare some variables and initialize them */
  bool cmd_override = false;
  bool cmd_found = false;
  bool get_command = true;
  bool simulate = false;
  char buffer[BOK_STR_2048] = {'\0'};
  char command[BOK_STR_64] = {'\0'};
  char *pr = command;
  GCon gfd = G_NO_ERROR;
  GReturn gstat = G_NO_ERROR;
  int cmd_index = G_GCLIB_ERROR;
  int rb = G_NO_ERROR;
  int wb = G_NO_ERROR;
  size_t w_char = BOK_STR_64;
  size_t r_char = G_NO_ERROR;

  (void) memset((void *)buffer,  '\0', sizeof(buffer));
  (void) memset((void *)command, '\0', sizeof(command));

  /* get command line parameter(s) */
  while ( argc>1L && argv[1][0]=='-' ) {
    switch (argv[1][1]) {
      case 'a': case 'A': author();   exit(0); break;
      case 'c': case 'C': commands(); exit(0); break;
      case 'd': case 'D': date();     exit(0); break;
      case 'e': case 'E': email();    exit(0); break;
      case 'o': case 'O': cmd_override = true; break;
      case 'v': case 'V': version();   exit(0); break;
      default:            help();      exit(0); break;
    }
    argc--; argv++;
  }

  /* print library version */
  gstat = G_NO_ERROR;
  (void) memset((void *)buffer, '\0', sizeof(buffer));
  if ((gstat=GVersion(buffer, sizeof(buffer))) == G_NO_ERROR) {
    (void) fprintf(stdout, "%s <OK> library version='%s', gstat=%d\n", _NAME_, buffer, gstat);
    (void) fflush(stdout);
  }

  /* print addresses */
  gstat = G_NO_ERROR;
  (void) memset(buffer, '\0', sizeof(buffer));
  if ((gstat=GAddresses(buffer, sizeof(buffer))) == G_NO_ERROR) {
    replace_word(buffer, sizeof(buffer), "\r", "");
    replace_word(buffer, sizeof(buffer), "\n", "");
    (void) fprintf(stdout, "%s <OK> addresses='%s', gstat=%d\n", _NAME_, buffer, gstat);
    (void) fflush(stdout);
  }

  /* open the ip_addr */
  gstat = G_NO_ERROR;
  (void) fprintf(stdout, "%s <%s> executing '%s', gstat=%d\n", _NAME_, (simulate == true ? "SIM" : "OK"), BOK_GALIL_TCP_CMD, gstat);
  (void) fflush(stdout);
  if ((gstat=GOpen(BOK_GALIL_TCP_CMD, &gfd)) != G_NO_ERROR) { simulate = true; }
  (void) fprintf(stdout, "%s <%s> executed '%s', gstat=%d\n", _NAME_, (simulate == true ? "SIM" : "OK"), BOK_GALIL_TCP_CMD, gstat);
  (void) fflush(stdout);

  /* get some info */
  (void) memset(buffer, '\0', sizeof(buffer));
  if ((gstat=GInfo(gfd, buffer, sizeof(buffer))) == G_NO_ERROR) {
    (void) fprintf(stdout, "%s <OK> info='%s', gstat=%d\n", _NAME_, buffer, gstat);
    (void) fflush(stdout);
  }

  /* loop for command */
  while (get_command) {

    /* initialize */
    (void) memset((void *)buffer, '\0', sizeof(buffer));
    (void) memset((void *)command, '\0', sizeof(command));

    /* get command from command line */
    (void) fprintf(stdout, "Galil command, '?' for help> ");
    (void) fflush(stdout);
    r_char = getline(&pr, &w_char, stdin);
    replace_word(pr, sizeof(pr), "\n", "");

    /* ignore blank lines */
    if (strlen(pr) == 0) { continue; }

    /* check the command is supported */
    cmd_found = false;
    cmd_index = G_GCLIB_ERROR;
    for (int x=0; x<CMD_NELMS; x++) {
      if (strncmp(pr, GalilCmds[x].name, strlen(GalilCmds[x].name)) == 0) {
        cmd_found = true;
        cmd_index = x;
        break;
      }
    }

    if (!cmd_override && !cmd_found) {
      cmd_index = G_GCLIB_ERROR;
      if (strlen(pr)) {
        (void) fprintf(stderr, "%s <INFO> command '%s' not supported\n", _NAME_, pr);
        (void) fflush(stderr);
      }
      continue;
    }

    /* exit/quit if requested */
    if (strncasecmp(pr, "quit", strlen("quit"))==0 || strncasecmp(pr, "exit", strlen("exit"))==0 ) {
      get_command = false;

    /* help/? if requested */
    } else if (strncasecmp(pr, "help", strlen("help"))==0 || strncasecmp(pr, "?", strlen("?"))==0 ) {
      commands();

    /* do nothing if simulating */
    } else if (simulate) {
      (void) fprintf(stderr, "%s <SIM> command '%s' not supported in simulation mode\n", _NAME_, pr);
      (void) fflush(stderr);
      continue;

    /* valid command */
    } else {
      (void) fprintf(stdout, "%s <INFO> sending '%s', length=%d, index=%d, r_char=%d, w_char=%d\n",
                     _NAME_, pr, (int)strlen(pr), cmd_index, (int)r_char, (int)w_char);
      (void) fflush(stdout);

      /* send command and get response */
      gstat = G_NO_ERROR;
      wb = (int)strlen(pr);
      if ((gstat=GCommand(gfd, pr, buffer, sizeof(buffer), 0)) == G_NO_ERROR) {
        (void) fprintf(stdout, "%s <OK> sent '%s', wb=%d, gstat=%d\n", _NAME_, pr, wb, gstat);
        (void) fflush(stdout);

        /* clean up the response */
        replace_word(buffer, sizeof(buffer), "\n", "");
        replace_word(buffer, sizeof(buffer), "\r", "");
        rb = (int)strlen(buffer);

        /* check status */
        if (strncasecmp(buffer, "?", 1)==0 && strlen(buffer)==1) {
          (void) fprintf(stderr, "%s <INFO> command rejected, recv='%s', rb=%d, gstat=%d\n", _NAME_, buffer, rb, gstat);
          (void) fflush(stderr);
        } else {
          (void) fprintf(stdout, "%s <OK> received '%s', rb=%d, gstat=%d\n", _NAME_, buffer, rb, gstat);
          (void) fflush(stdout);
        }

      /* report error */
      } else {
        gstat = G_NO_ERROR;
        (void) memset(buffer, '\0', sizeof(buffer));
        if ((gstat=GCommand(gfd, "TC 1;", buffer, sizeof(buffer), 0)) == G_NO_ERROR) {
          replace_word(buffer, sizeof(buffer), "\n", " ");
          replace_word(buffer, sizeof(buffer), "\r", " ");
        }
        (void) fprintf(stderr, "%s <ERROR> error='%s', gstat=%d\n", _NAME_, buffer, gstat);
        (void) fprintf(stderr, "%s <ERROR> try 'help' or '?'\n", _NAME_);
        (void) fflush(stderr);
      }
    }
  }

  /* close file descriptor */
  if (gfd) { (void) GClose(gfd); }

  /* return */
  return 0;
}
