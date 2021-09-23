/*******************************************************************************
 *
 * bok_read_filters.c
 *
 ******************************************************************************/


/*******************************************************************************
 * include(s)
 ******************************************************************************/
#include "bokGalil.h"


/*******************************************************************************
 * define(s)
 ******************************************************************************/
#define _HELP_ "Reads BOK filters from file"
#define _NAME_ "bok_read_filters"


/*******************************************************************************
 * function: main()
 ******************************************************************************/
int main( int argc, char *argv[] ) {

  /* declare some variable(s) and initialize them */
  char fname[BOK_STR_1024] = {'\0'};
  int ncols = BOK_IFILTER_COLUMNS;
  int nslots = BOK_IFILTER_SLOTS;
  filter_file_t bok_ifilters[BOK_IFILTER_SLOTS];

  /* initialize structure(s) */
  (void) memset((void *)&fname, '\0', sizeof(fname));
  for (int i=0; i<BOK_IFILTER_SLOTS; i++) { (void) memset((void *)&bok_ifilters[i], '\0', sizeof(filter_file_t)); }

  /* get command line parameter(s) */
  while ( argc>1L && argv[1][0]=='-' ) {
    switch (argv[1][1]) {
      case 'a': case 'A': (void) printf("author=%s\n", _AUTHOR_); exit (0); break;
      case 'c': case 'C': ncols = atoi(&argv[1][2]); break;
      case 'd': case 'D': (void) printf("date=%s\n", _DATE_); exit (0); break;
      case 'e': case 'E': (void) printf("email=%s\n", _EMAIL_); exit (0); break;
      case 'n': case 'N': nslots = atoi(&argv[1][2]); break;
      case 'v': case 'V': (void) printf("version=%s\n", _VERSION_); exit (0); break;
      case 'f': case 'F': (void) sprintf(fname, "%s", &argv[1][2]); break;
      default:
        (void) printf("%s: v%s %s %s\n", _NAME_, _VERSION_, _AUTHOR_, _DATE_);
        (void) printf("%s\n", _HELP_);
        (void) printf("\nUse: %s [-a -c<int> -d -e -f<path> -h -n<int> -v]\n", _NAME_);
        (void) printf("\t-a       : show author\n");
        (void) printf("\t-c<int>  : number of columns in file [default=%d]\n", BOK_IFILTER_COLUMNS);
        (void) printf("\t-d       : show date\n");
        (void) printf("\t-e       : show email\n");
        (void) printf("\t-f<path> : set file to <path> [default=%s]\n", BOK_IFILTER_FILE);
        (void) printf("\t-n<int>  : number of filter slots [default=%d]\n", BOK_IFILTER_SLOTS);
        (void) printf("\t-v       : show version\n");
        exit (0);
        break;
    }
    argc--; argv++;
  }

  /* check we have a filename */
  if (strlen(fname) == 0) {
    (void) memset((void *)&fname, '\0', sizeof(fname));
    (void) sprintf(fname, "%s", BOK_IFILTER_FILE);
  }

  /* read */
  read_filters_from_file(fname, (filter_file_t *)bok_ifilters, nslots, ncols);

  /* dump */
  for (int j=0; j<BOK_IFILTER_SLOTS; j++) {
    if (strlen(bok_ifilters[j].code)>0 && strlen(bok_ifilters[j].name)>0) {
      (void) printf("index=%d, code='%s', name='%s'\n", j, bok_ifilters[j].code, bok_ifilters[j].name);
    }
  }

  /* return */
  return 0;
}
