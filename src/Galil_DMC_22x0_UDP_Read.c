/*******************************************************************************
 *
 * Galil_DMC_22x0_UDP_Read.c
 *
 ******************************************************************************/


/*******************************************************************************
 * include(s)
 ******************************************************************************/
#include "bokGalil.h"


/*******************************************************************************
 * define(s)
 ******************************************************************************/
#define _HELP_ "Read shared memory for polling command(s)"
#define _NAME_ "Galil_DMC_22x0_UDP_Read"


/*******************************************************************************
 * function: main()
 ******************************************************************************/
int main( int argc, char *argv[] ) {

  /* declare some variables and initialize them */
  int delay = BOK_UDP_DELAY_MS;
  int running = 1;
  int udp_shm_fd = -1;
  udp_val_p udp_shm_ptr = (udp_val_p)NULL;

  /* get command line parameter(s) */
  while ( argc>1L && argv[1][0]=='-' ) {
    switch (argv[1][1]) {
      case 'd': case 'D':
        delay = atoi(&argv[1][2]);
        break;
      default:
        (void) fprintf(stdout, "%s: v%s %s %s\n", _NAME_, _VERSION_, _AUTHOR_, _DATE_);
        (void) fprintf(stdout, "%s\n", _HELP_);
        (void) fprintf(stdout, "\nUse: %s [-d<int> -h]\n", _NAME_);
        (void) fprintf(stdout, "\t-d<int>  : set delay (ms) [default=%d]\n", BOK_UDP_DELAY_MS);
        (void) fflush(stdout);
        exit (0);
        break;
    }
    argc--; argv++;
  }

  /* get shared memory */
  if ((udp_shm_fd=shm_open(BOK_SHM_UDP_NAME, O_RDONLY, 0666)) < 0) {
    (void) fprintf(stderr, "%s <ERROR> failed to execute '%s' exiting, udp_shm_fd=%d\n", _NAME_, "shm_open()", udp_shm_fd);
    (void) fflush(stderr);
    exit(udp_shm_fd);
  }
  
  /* memory map the shared memory object */
  if ((udp_shm_ptr=(udp_val_p)mmap(0, UDP_VAL_SIZE, PROT_READ, MAP_SHARED, udp_shm_fd, 0)) == (udp_val_p)NULL) {
    (void) fprintf(stderr, "%s <ERROR> failed to execute '%s' exiting, udp_shm_ptr=%p\n", _NAME_, "mmap()", udp_shm_ptr);
    (void) fflush(stderr);
    exit(-1);
  }

  /* loop for command(s) */
  while (running > 0) {
    (void) usleep((useconds_t)delay*1000);
    dump_udp_structure(udp_shm_ptr);
    if (udp_shm_ptr->shutdown) { running = 0; }
  }

  /* close shared memory */
  if (udp_shm_ptr != (udp_val_p)NULL) { (void) munmap(udp_shm_ptr, UDP_VAL_SIZE); }
  if (udp_shm_fd >= 0) { (void) close(udp_shm_fd); }

  /* return */
  return 0;
}
