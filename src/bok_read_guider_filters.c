/*******************************************************************************
 *
 * bok_read_guider_filters.c
 *
 ******************************************************************************/


/*******************************************************************************
 * include(s)
 ******************************************************************************/
#include "bokGalil.h"


/*******************************************************************************
 * define(s)
 ******************************************************************************/
#define _HELP_ "Reads BOK guider filters from file"
#define _NAME_ "bok_read_guider_filters"


/*******************************************************************************
 * function: main()
 ******************************************************************************/
int main( int argc, char *argv[] ) {

  /* declare some variables and initialize them */
  filter_file_t bok_gfilters[BOK_GFILTER_SLOTS];

  /* initialize structure(s) */
  for (int i=0; i<BOK_GFILTER_SLOTS; i++) { (void) memset((void *)&bok_gfilters[i], '\0', sizeof(filter_file_t)); }

  /* read */
  read_filters_from_file(BOK_GFILTER_FILE, (filter_file_t *)bok_gfilters, BOK_GFILTER_SLOTS, BOK_GFILTER_COLUMNS);

  /* dump */
  for (int j=0; j<BOK_GFILTER_SLOTS; j++) {
    if (strlen(bok_gfilters[j].code)>0 && strlen(bok_gfilters[j].name)>0) {
      (void) printf("index=%d, code='%s', name='%s'\n", j, bok_gfilters[j].code, bok_gfilters[j].name);
    }
  }

  /* return */
  return 0;
}
