#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gclibo.h"

int main( int argc, char *argv[] ) {

  /* declare some variables and initialize them */
  GCon gfd = G_NO_ERROR;
  GReturn gstat = G_NO_ERROR;
  int simulate = 1;

  char addresses[2048] = {'\0'};
  char info[2048] = {'\0'};
  char response[2048] = {'\0'};
  char version[2048] = {'\0'};

  /* get command line parameter(s) */
  while ( argc>1L && argv[1][0]=='-' ) {
    switch (argv[1][1]) {
      case 's': case 'S':
        simulate = atoi(&argv[1][2]);
        break;
      default:
        (void) fprintf(stdout, "Use: %s [-s<int> -h]\n", argv[0]);
        (void) fprintf(stdout, "\t-s<int>  : set simulate [default=%d]\n", simulate);
        (void) fflush(stdout);
        exit (0);
        break;
    }
    argc--; argv++;
  }

  /* execute */ 
  if (simulate > 0) {
    gstat = GOpen("192.168.0.100 --command TCP", &gfd);
    (void) printf("GOpen('192.168.0.100 --command TCP') called, gstat=%d, gfd=%ld\n", (int)gstat, (long)gfd);
  } else {
    gstat = GOpen("10.30.3.31 --command TCP", &gfd);
    (void) printf("GOpen('10.30.3.31 --command TCP') called, gstat=%d, gfd=%ld\n", (int)gstat, (long)gfd);
  }

  (void) memset((void *)version, '\0', 2048);
  gstat = GVersion(version, sizeof(version));
  (void) printf("GVersion() called, gstat=%d, version='%s'\n", (int)gstat, version);

  (void) memset((void *)addresses, '\0', 2048);
  gstat = GAddresses(addresses, sizeof(addresses));
  (void) printf("GAddresses() called, gstat=%d, addresses='%s'\n", (int)gstat, addresses);

  if (gfd) {
    (void) memset((void *)info, '\0', 2048);
    gstat = GInfo(gfd, info, sizeof(info));
    (void) printf("GInfo() called, gstat=%d, info='%s'\n", (int)gstat, info);

    (void) memset((void *)response, '\0', 2048);
    gstat = GCommand(gfd, "LV;", response, sizeof(response), 0);
    (void) printf("GCommand('LV;') called, gstat=%d, response='%s'\n", (int)gstat, response);

    (void) memset((void *)response, '\0', 2048);
    gstat = GCommand(gfd, "TH;", response, sizeof(response), 0);
    (void) printf("GCommand('TH;') called, gstat=%d, response='%s'\n", (int)gstat, response);

    gstat = GClose(gfd);
    (void) printf("GClose() called, gstat=%d\n", (int)gstat);
  }

  /* return */
  return 0;
}
