/*******************************************************************************
 *
 * Galil_DMC_22x0_TCP_Read.c
 *
 ******************************************************************************/


/*******************************************************************************
 * include(s)
 ******************************************************************************/
#include "bokGalil.h"


/*******************************************************************************
 * define(s)
 ******************************************************************************/
#define _HELP_ "Read shared memory for TCP command(s) from Galil_DMC_22x0_TCP"
#define _NAME_ "Galil_DMC_22x0_TCP_Read"


/*******************************************************************************
 * function: main()
 ******************************************************************************/
int main( int argc, char *argv[] ) {

  /* declare some variables and initialize them */
  int delay = BOK_TCP_DELAY_MS;
  int running = 1;
  int tcp_shm_fd = -1;
  tcp_val_p tcp_shm_ptr = (tcp_val_p)NULL;

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
        (void) fprintf(stdout, "\t-d<int>  : set delay (ms) [default=%d]\n", BOK_TCP_DELAY_MS);
        (void) fflush(stdout);
        exit (0);
        break;
    }
    argc--; argv++;
  }

  /* get shared memory */
  if ((tcp_shm_fd=shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666)) < 0) {
    (void) fprintf(stderr, "%s <ERROR> failed to execute '%s' exiting, tcp_shm_fd=%d\n", _NAME_, "shm_open()", tcp_shm_fd);
    (void) fflush(stderr);
    exit(tcp_shm_fd);
  }
  
  /* memory map the shared memory object */
  if ((tcp_shm_ptr=(tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, tcp_shm_fd, 0)) == (tcp_val_p)NULL) {
    (void) fprintf(stderr, "%s <ERROR> failed to execute '%s' exiting, tcp_shm_ptr=%p\n", _NAME_, "mmap()", tcp_shm_ptr);
    (void) fflush(stderr);
    exit(-1);
  }

  /* loop for command(s) */
  while (running > 0) {
    (void) usleep((useconds_t)delay*1000);
    dump_tcp_structure(tcp_shm_ptr);
    if (tcp_shm_ptr->shutdown) { running = 0; }
  }

  /* close shared memory */
  if (tcp_shm_ptr != (tcp_val_p)NULL) { (void) munmap(tcp_shm_ptr, TCP_VAL_SIZE); }
  if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }

  /* return */
  return 0;
}
