/*******************************************************************************
 *
 * bok_read_instrument_filters.c
 *
 ******************************************************************************/


/*******************************************************************************
 * include(s)
 ******************************************************************************/
#include "bokGalil.h"


/*******************************************************************************
 * define(s)
 ******************************************************************************/
#define _HELP_ "Reads BOK instrument filters from file"
#define _NAME_ "bok_read_instrument_filters"


/*******************************************************************************
 * function: main()
 ******************************************************************************/
int main( int argc, char *argv[] ) {

  /* declare some variables and initialize them */
  filter_file_t bok_ifilters[BOK_IFILTER_SLOTS];
  /* declare some variables and initialize them */
  char *ipath = (char *)NULL;
  char iname[BOK_STR_1024] = {'\0'};

  /* get instrument filters file */
  (void) memset((void *)&iname, '\0', sizeof(iname));
  if ((ipath=getenv("BOK_GALIL_DOCS")) == (char *)NULL) {
    (void) sprintf(iname, "%s", BOK_IFILTER_FILE);
  } else {
    (void) sprintf(iname, "%s/%s", ipath, BOK_IFILTER_FILE);
  }

  /* initialize structure(s) */
  for (int i=0; i<BOK_IFILTER_SLOTS; i++) { (void) memset((void *)&bok_ifilters[i], '\0', sizeof(filter_file_t)); }

  /* read */
  read_filters_from_file(iname, (filter_file_t *)bok_ifilters, BOK_IFILTER_SLOTS, BOK_IFILTER_COLUMNS);

  /* dump */
  for (int j=0; j<BOK_IFILTER_SLOTS; j++) {
    if (strlen(bok_ifilters[j].code)>0 && strlen(bok_ifilters[j].name)>0) {
      (void) printf("index=%d, code='%s', name='%s'\n", j, bok_ifilters[j].code, bok_ifilters[j].name);
    }
  }

  /* return */
  return 0;
}
