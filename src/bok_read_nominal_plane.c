/*******************************************************************************
 *
 * bok_read_nominal_plane.c
 *
 ******************************************************************************/


/*******************************************************************************
 * include(s)
 ******************************************************************************/
#include "bokGalil.h"


/*******************************************************************************
 * define(s)
 ******************************************************************************/
#define _HELP_ "Reads BOK nominal_plane from file"
#define _NAME_ "bok_read_nominal_plane"


/*******************************************************************************
 * function: main()
 ******************************************************************************/
int main( int argc, char *argv[] ) {

  /* declare some variable(s) and initialize them */
  char fname[BOK_STR_1024] = {'\0'};
  float np1 = NAN;
  float np2 = NAN;
  float np3 = NAN;

  /* initialize structure(s) */
  (void) memset((void *)&fname, '\0', sizeof(fname));

  /* get command line parameter(s) */
  while ( argc>1L && argv[1][0]=='-' ) {
    switch (argv[1][1]) {
      case 'a': case 'A': (void) printf("author=%s\n", _AUTHOR_); exit (0); break;
      case 'd': case 'D': (void) printf("date=%s\n", _DATE_); exit (0); break;
      case 'e': case 'E': (void) printf("email=%s\n", _EMAIL_); exit (0); break;
      case 'v': case 'V': (void) printf("version=%s\n", _VERSION_); exit (0); break;
      case 'f': case 'F': (void) sprintf(fname, "%s", &argv[1][2]); break;
      default:
        (void) printf("%s: v%s %s %s\n", _NAME_, _VERSION_, _AUTHOR_, _DATE_);
        (void) printf("%s\n", _HELP_);
        (void) printf("\nUse: %s [-a -d -e -f<path> -h -v]\n", _NAME_);
        (void) printf("\t-a       : show author\n");
        (void) printf("\t-d       : show date\n");
        (void) printf("\t-e       : show email\n");
        (void) printf("\t-f<path> : set file to <path> [default=%s]\n", BOK_NPLANE_FILE);
        (void) printf("\t-v       : show version\n");
        exit (0);
        break;
    }
    argc--; argv++;
  }

  /* check we have a filename */
  if (strlen(fname) == 0) {
    (void) memset((void *)&fname, '\0', sizeof(fname));
    (void) sprintf(fname, "%s", BOK_NPLANE_FILE);
  }

  /* read */
  read_nominal_plane_from_file(fname, &np1, &np2, &np3);

  /* dump */
  (void) printf("np1=%f, np2=%f, np3=%f\n", np1, np2, np3);

  /* return */
  return 0;
}
