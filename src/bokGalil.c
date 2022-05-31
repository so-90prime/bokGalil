/*******************************************************************************
 *
 * bokGalil.c
 *
 ******************************************************************************/


/*******************************************************************************
 * include(s)
 ******************************************************************************/
#include "bokGalil.h"


/*******************************************************************************
 * define(s)
 ******************************************************************************/
#define _HELP_ "Common code for the Galil_DMC_22x0"
#define _NAME_ "bokGalil"


/*******************************************************************************
 * chomp( ... )
 ******************************************************************************/
void chomp(char *s, char *x) {
  while (*s++) {
    if (*s!=EOF && *s==*x) *s = ' ';
  }
}


/*******************************************************************************
 * cliDump ( ... )
 *******************************************************************************/
void cliDump (unsigned int argc, char *argv[]) {
  for (int ic=0; ic<argc; ic++) {
    if (strlen(argv[ic]) > 0) {
      (void) fprintf(stderr, "argv[%d]=\"%s\"\n", ic, argv[ic]);
      (void) fflush(stderr);
    }
  }
}

/*******************************************************************************
 * cliFree ( ... )
 *******************************************************************************/
void cliFree (unsigned int argc, char *argv[]) {
  for (int ic=0; ic<argc; ic++) {
    if ( argv[ic] != (char *)NULL ) free((void *)argv[ic]);
    argv[ic] = (char *)NULL;
  }
}


/*******************************************************************************
 * cliInit ( ... )
 *******************************************************************************/
void cliInit (unsigned int argc, size_t arg_size, char *argv[]) {
  for (int ic=0; ic<argc; ic++ ) {
    if ( argv[ic] != (char *)NULL ) (void) memset((void *)argv[ic], '\0', arg_size);
  }
}


/*******************************************************************************
 * cliMalloc ( ... )
 *******************************************************************************/
void cliMalloc (unsigned int argc, size_t arg_size, char *argv[]) {
  for (int ic=0; ic<argc; ic++) {
    if ( (argv[ic]=(char *)malloc(arg_size)) != (char *)NULL ) {
      (void) memset((void *)argv[ic], '\0', arg_size);
    }
  }
}


/*******************************************************************************
 * cliParse ( ... )
 *******************************************************************************/
void cliParse (const char *cli_delimiter, const char *cli_string, const size_t cli_size,
               unsigned int *argc, size_t arg_size, char *argv[]) {

  /* declare some variables and initialize them */
  char *sv = (char *)NULL;
  char *tp = (char *)NULL;
  char strlocal[cli_size];
  (void) memset((void *)strlocal, '\0', cli_size);

  /* copy the string char for char */
  (void) memmove(strlocal, cli_string, cli_size);

  /* parse the string and stop when there are no more tokens */
  *argc = 0L;
  while ( (tp=strtok_r(((*argc==0L)?strlocal:(char *)NULL),cli_delimiter,&sv)) != (char *)NULL ) {
     if ( argv[*argc] == (char *)NULL ) {return;}
     (void) memset((void *)argv[*argc], '\0', arg_size);
     (void) memmove((void *)argv[*argc], (void *)tp, strlen(tp));
     (*argc)++;
  }
}


/*******************************************************************************
 * decode_fvals( ... )
 ******************************************************************************/
void decode_fvals( char *b, float f[], int nelms, char delim ) {
  char *ep = (char *)NULL;
  for (int j=0; j<nelms; j++) {
    f[j] = strtof(b, &ep);
    if ((*ep == delim || *ep == EOF) && errno != 0) { f[j] = NAN; }
    b = ++ep;
  }
}


/*******************************************************************************
 * decode_float( ... )
 ******************************************************************************/
void decode_float( char *b, float *fval ) {
  char * ep = (char *)NULL;
  replace_word(b, sizeof(b), " ", "");
  *fval = strtof(b, &ep);
  if (ep == (char *)NULL || errno != 0) { *fval = NAN; }
}


/*******************************************************************************
 * decode_integer( ... )
 ******************************************************************************/
void decode_integer( char *b, int *ival ) {
  char * ep = (char *)NULL;
  replace_word(b, sizeof(b), " ", "");
  *ival = strtof(b, &ep);
  if (ep == (char *)NULL || errno != 0) { *ival = INT_MIN; }
}


/*******************************************************************************
 * dump_tcp_structure( ... )
 ******************************************************************************/
void dump_tcp_structure(tcp_val_t *p) {
  (void) fprintf(stderr,
    "\n%s"
    "status=%d, "
    "filtvals[0]=%.1f, "
    "filtvals[1]=%.1f, "
    "filtvals[2]=%.1f, "
    "filtvals[3]=%.1f, "
    "filtvals[4]=%.1f, "
    "filtvals[5]=%.1f, "
    "position[0]=%.1f, "
    "position[1]=%.1f, "
    "position[2]=%.1f, "
    "position[3]=%.1f, "
    "position[4]=%.1f, "
    "position[5]=%.1f, "
    "position[6]=%.1f, "
    "position[7]=%.1f, "
    "lv.dista=%.1f, "
    "lv.distall=%.1f, "
    "lv.distb=%.1f, "
    "lv.distc=%.1f, "
    "lv.distgcam=%.1f, "
    "lv.errfilt=%.1f, "
    "lv.filtbit1=%.1f, "
    "lv.filtbit2=%.1f, "
    "lv.filtbit3=%.1f, "
    "lv.filtbit4=%.1f, "
    "lv.filtbit5=%.1f, "
    "lv.filtbit6=%.1f, "
    "lv.filtbit7=%.1f, "
    "lv.filtbit8=%.1f, "
    "lv.filthigh=%.1f, "
    "lv.filtisin=%.1f, "
    "lv.filtlow=%.1f, "
    "lv.filtnum=%.1f, "
    "lv.filtrac=%.1f, "
    "lv.filtrbl=%.1f, "
    "lv.filtrdc=%.1f, "
    "lv.filtrfl=%.1f, "
    "lv.filtrsp=%.1f, "
    "lv.filttac=%.1f, "
    "lv.filttdc=%.1f, "
    "lv.filttdis=%.1f, "
    "lv.filttnud=%.1f, "
    "lv.filttsc=%.1f, "
    "lv.filttsp=%.1f, "
    "lv.filtval=%.1f, "
    "lv.fnum=%.1f, "
    "lv.fnum_in=%.1f, "
    "lv.focac=%.1f, "
    "lv.focbl=%.1f, "
    "lv.focdc=%.1f, "
    "lv.focfl=%.1f, "
    "lv.focrefa=%.1f, "
    "lv.focrefb=%.1f, "
    "lv.focrefc=%.1f, "
    "lv.focrfset=%.1f, "
    "lv.focsp=%.1f, "
    "lv.gfac=%.1f, "
    "lv.gfcent=%.1f, "
    "lv.gfdc=%.1f, "
    "lv.gfiltac=%.1f, "
    "lv.gfiltdc=%.1f, "
    "lv.gfiltn=%.1f, "
    "lv.gfiltq=%.1f, "
    "lv.gfiltreq=%.1f, "
    "lv.gfiltsp=%.1f, "
    "lv.gfsp=%.1f, "
    "lv.gifltn=%.1f, "
    "lv.initfilt=%.1f, "
    "lv.nmoves=%.1f, "
    "lv.nrot=%.1f, "
    "lv.reqfilt=%.1f, "
    "lv.snum=%.1f, "
    "lv.snum_in=%.1f, "
    "lv.totfoca=%.1f, "
    "lv.totfocb=%.1f, "
    "lv.totfocc=%.1f, "
    "lv.vecac=%.1f, "
    "lv.vecdc=%.1f, "
    "lv.vecsp=%.1f, "
    "gstatus=%d, "
    "simulate=%d, "
    "shutdown=%d, "
    "counter=%d, "
    "jd=%.8f, "
    "timestamp='%s', "
    "hardware='%s', "
    "software='%s'\n",
    (p->simulate > 0 ? "<SIMULATE> " : "<DIRECT> "),
    p->status,
    p->filtvals[0],
    p->filtvals[1],
    p->filtvals[2],
    p->filtvals[3],
    p->filtvals[4],
    p->filtvals[5],
    p->position[0],
    p->position[1],
    p->position[2],
    p->position[3],
    p->position[4],
    p->position[5],
    p->position[6],
    p->position[7],
    p->lv.dista,
    p->lv.distall,
    p->lv.distb,
    p->lv.distc,
    p->lv.distgcam,
    p->lv.errfilt,
    p->lv.filtbit1,
    p->lv.filtbit2,
    p->lv.filtbit3,
    p->lv.filtbit4,
    p->lv.filtbit5,
    p->lv.filtbit6,
    p->lv.filtbit7,
    p->lv.filtbit8,
    p->lv.filthigh,
    p->lv.filtisin,
    p->lv.filtlow,
    p->lv.filtnum,
    p->lv.filtrac,
    p->lv.filtrbl,
    p->lv.filtrdc,
    p->lv.filtrfl,
    p->lv.filtrsp,
    p->lv.filttac,
    p->lv.filttdc,
    p->lv.filttdis,
    p->lv.filttnud,
    p->lv.filttsc,
    p->lv.filttsp,
    p->lv.filtval,
    p->lv.fnum,
    p->lv.fnum_in,
    p->lv.focac,
    p->lv.focbl,
    p->lv.focdc,
    p->lv.focfl,
    p->lv.focrefa,
    p->lv.focrefb,
    p->lv.focrefc,
    p->lv.focrfset,
    p->lv.focsp,
    p->lv.gfac,
    p->lv.gfcent,
    p->lv.gfdc,
    p->lv.gfiltac,
    p->lv.gfiltdc,
    p->lv.gfiltn,
    p->lv.gfiltq,
    p->lv.gfiltreq,
    p->lv.gfiltsp,
    p->lv.gfsp,
    p->lv.gifltn,
    p->lv.initfilt,
    p->lv.nmoves,
    p->lv.nrot,
    p->lv.reqfilt,
    p->lv.snum,
    p->lv.snum_in,
    p->lv.totfoca,
    p->lv.totfocb,
    p->lv.totfocc,
    p->lv.vecac,
    p->lv.vecdc,
    p->lv.vecsp,
    (int)p->gstatus,
    p->simulate,
    p->shutdown,
    p->counter,
    p->jd,
    p->timestamp,
    p->hardware,
    p->software);
  (void) fflush(stderr);
}


/*******************************************************************************
 * dump_udp_structure( ... )
 ******************************************************************************/
void dump_udp_structure(udp_val_t *p) {
  (void) fprintf(stderr,
    "\n%s"
    "aaxis_enabled=%d, "
    "baxis_enabled=%d, "
    "caxis_enabled=%d, "
    "daxis_enabled=%d, "
    "eaxis_enabled=%d, "
    "faxis_enabled=%d, "
    "gaxis_enabled=%d, "
    "haxis_enabled=%d, "
    "saxis_enabled=%d, "
    "taxis_enabled=%d, "
    "iaxis_enabled=%d, "
    "header_length=%d, "
    "iaxis_sample_number=%d, "
    "iaxis_general_input[0]=%d, "
    "iaxis_general_input[1]=%d, "
    "iaxis_general_input[2]=%d, "
    "iaxis_general_input[3]=%d, "
    "iaxis_general_input[4]=%d, "
    "iaxis_general_input[5]=%d, "
    "iaxis_general_input[6]=%d, "
    "iaxis_general_input[7]=%d, "
    "iaxis_general_input[8]=%d, "
    "iaxis_general_input[9]=%d, "
    "iaxis_general_output[0]=%d, "
    "iaxis_general_output[1]=%d, "
    "iaxis_general_output[2]=%d, "
    "iaxis_general_output[3]=%d, "
    "iaxis_general_output[4]=%d, "
    "iaxis_general_output[5]=%d, "
    "iaxis_general_output[6]=%d, "
    "iaxis_general_output[7]=%d, "
    "iaxis_general_output[8]=%d, "
    "iaxis_general_output[9]=%d, "
    "iaxis_error_code=%d, "
    "iaxis_echo_on=%d, "
    "iaxis_trace_on=%d, "
    "iaxis_awaiting_input=%d, "
    "iaxis_program_running=%d, "
    "saxis_segment_count=%d, "
    "saxis_decelerating=%d, "
    "saxis_stopping=%d, "
    "saxis_slewing=%d, "
    "saxis_moving=%d, "
    "saxis_distance_traveled=%d, "
    "taxis_segment_count=%d, "
    "taxis_decelerating=%d, "
    "taxis_stopping=%d, "
    "taxis_slewing=%d, "
    "taxis_moving=%d, "
    "taxis_distance_traveled=%d, "
    "aaxis_motor_off=%d, "
    "aaxis_error_armed=%d, "
    "aaxis_latch_armed=%d, "
    "aaxis_decelerating=%d, "
    "aaxis_stopping=%d, "
    "aaxis_slewing=%d, "
    "aaxis_contour_mode=%d, "
    "aaxis_negative_direction=%d, "
    "aaxis_motion_mode=%d, "
    "aaxis_home_1=%d, "
    "aaxis_home_2=%d, "
    "aaxis_homing=%d, "
    "aaxis_finding_edge=%d, "
    "aaxis_pa_motion=%d, "
    "aaxis_pr_motion=%d, "
    "aaxis_moving=%d, "
    "aaxis_sm_jumper=%d, "
    "aaxis_state_home=%d, "
    "aaxis_state_reverse=%d, "
    "aaxis_state_forward=%d, "
    "aaxis_state_latch=%d, "
    "aaxis_latch_occurred=%d, "
    "aaxis_stop_code=%d, "
    "aaxis_reference_position=%d, "
    "aaxis_motor_position=%d, "
    "aaxis_position_error=%d, "
    "aaxis_auxiliary_position=%d, "
    "aaxis_velocity=%d, "
    "aaxis_torque=%d, "
    "aaxis_analog_in=%d, "
    "baxis_motor_off=%d, "
    "baxis_error_armed=%d, "
    "baxis_latch_armed=%d, "
    "baxis_decelerating=%d, "
    "baxis_stopping=%d, "
    "baxis_slewing=%d, "
    "baxis_contour_mode=%d, "
    "baxis_negative_direction=%d, "
    "baxis_motion_mode=%d, "
    "baxis_home_1=%d, "
    "baxis_home_2=%d, "
    "baxis_homing=%d, "
    "baxis_finding_edge=%d, "
    "baxis_pa_motion=%d, "
    "baxis_pr_motion=%d, "
    "baxis_moving=%d, "
    "baxis_sm_jumper=%d, "
    "baxis_state_home=%d, "
    "baxis_state_reverse=%d, "
    "baxis_state_forward=%d, "
    "baxis_state_latch=%d, "
    "baxis_latch_occurred=%d, "
    "baxis_stop_code=%d, "
    "baxis_reference_position=%d, "
    "baxis_motor_position=%d, "
    "baxis_position_error=%d, "
    "baxis_auxiliary_position=%d, "
    "baxis_velocity=%d, "
    "baxis_torque=%d, "
    "baxis_analog_in=%d, "
    "caxis_motor_off=%d, "
    "caxis_error_armed=%d, "
    "caxis_latch_armed=%d, "
    "caxis_decelerating=%d, "
    "caxis_stopping=%d, "
    "caxis_slewing=%d, "
    "caxis_contour_mode=%d, "
    "caxis_negative_direction=%d, "
    "caxis_motion_mode=%d, "
    "caxis_home_1=%d, "
    "caxis_home_2=%d, "
    "caxis_homing=%d, "
    "caxis_finding_edge=%d, "
    "caxis_pa_motion=%d, "
    "caxis_pr_motion=%d, "
    "caxis_moving=%d, "
    "caxis_sm_jumper=%d, "
    "caxis_state_home=%d, "
    "caxis_state_reverse=%d, "
    "caxis_state_forward=%d, "
    "caxis_state_latch=%d, "
    "caxis_latch_occurred=%d, "
    "caxis_stop_code=%d, "
    "caxis_reference_position=%d, "
    "caxis_motor_position=%d, "
    "caxis_position_error=%d, "
    "caxis_auxiliary_position=%d, "
    "caxis_velocity=%d, "
    "caxis_torque=%d, "
    "caxis_analog_in=%d, "
    "daxis_motor_off=%d, "
    "daxis_error_armed=%d, "
    "daxis_latch_armed=%d, "
    "daxis_decelerating=%d, "
    "daxis_stopping=%d, "
    "daxis_slewing=%d, "
    "daxis_contour_mode=%d, "
    "daxis_negative_direction=%d, "
    "daxis_motion_mode=%d, "
    "daxis_home_1=%d, "
    "daxis_home_2=%d, "
    "daxis_homing=%d, "
    "daxis_finding_edge=%d, "
    "daxis_pa_motion=%d, "
    "daxis_pr_motion=%d, "
    "daxis_moving=%d, "
    "daxis_sm_jumper=%d, "
    "daxis_state_home=%d, "
    "daxis_state_reverse=%d, "
    "daxis_state_forward=%d, "
    "daxis_state_latch=%d, "
    "daxis_latch_occurred=%d, "
    "daxis_stop_code=%d, "
    "daxis_reference_position=%d, "
    "daxis_motor_position=%d, "
    "daxis_position_error=%d, "
    "daxis_auxiliary_position=%d, "
    "daxis_velocity=%d, "
    "daxis_torque=%d, "
    "daxis_analog_in=%d, "
    "eaxis_motor_off=%d, "
    "eaxis_error_armed=%d, "
    "eaxis_latch_armed=%d, "
    "eaxis_decelerating=%d, "
    "eaxis_stopping=%d, "
    "eaxis_slewing=%d, "
    "eaxis_contour_mode=%d, "
    "eaxis_negative_direction=%d, "
    "eaxis_motion_mode=%d, "
    "eaxis_home_1=%d, "
    "eaxis_home_2=%d, "
    "eaxis_homing=%d, "
    "eaxis_finding_edge=%d, "
    "eaxis_pa_motion=%d, "
    "eaxis_pr_motion=%d, "
    "eaxis_moving=%d, "
    "eaxis_sm_jumper=%d, "
    "eaxis_state_home=%d, "
    "eaxis_state_reverse=%d, "
    "eaxis_state_forward=%d, "
    "eaxis_state_latch=%d, "
    "eaxis_latch_occurred=%d, "
    "eaxis_stop_code=%d, "
    "eaxis_reference_position=%d, "
    "eaxis_motor_position=%d, "
    "eaxis_position_error=%d, "
    "eaxis_auxiliary_position=%d, "
    "eaxis_velocity=%d, "
    "eaxis_torque=%d, "
    "eaxis_analog_in=%d, "
    "faxis_motor_off=%d, "
    "faxis_error_armed=%d, "
    "faxis_latch_armed=%d, "
    "faxis_decelerating=%d, "
    "faxis_stopping=%d, "
    "faxis_slewing=%d, "
    "faxis_contour_mode=%d, "
    "faxis_negative_direction=%d, "
    "faxis_motion_mode=%d, "
    "faxis_home_1=%d, "
    "faxis_home_2=%d, "
    "faxis_homing=%d, "
    "faxis_finding_edge=%d, "
    "faxis_pa_motion=%d, "
    "faxis_pr_motion=%d, "
    "faxis_moving=%d, "
    "faxis_sm_jumper=%d, "
    "faxis_state_home=%d, "
    "faxis_state_reverse=%d, "
    "faxis_state_forward=%d, "
    "faxis_state_latch=%d, "
    "faxis_latch_occurred=%d, "
    "faxis_stop_code=%d, "
    "faxis_reference_position=%d, "
    "faxis_motor_position=%d, "
    "faxis_position_error=%d, "
    "faxis_auxiliary_position=%d, "
    "faxis_velocity=%d, "
    "faxis_torque=%d, "
    "faxis_analog_in=%d, "
    "gaxis_motor_off=%d, "
    "gaxis_error_armed=%d, "
    "gaxis_latch_armed=%d, "
    "gaxis_decelerating=%d, "
    "gaxis_stopping=%d, "
    "gaxis_slewing=%d, "
    "gaxis_contour_mode=%d, "
    "gaxis_negative_direction=%d, "
    "gaxis_motion_mode=%d, "
    "gaxis_home_1=%d, "
    "gaxis_home_2=%d, "
    "gaxis_homing=%d, "
    "gaxis_finding_edge=%d, "
    "gaxis_pa_motion=%d, "
    "gaxis_pr_motion=%d, "
    "gaxis_moving=%d, "
    "gaxis_sm_jumper=%d, "
    "gaxis_state_home=%d, "
    "gaxis_state_reverse=%d, "
    "gaxis_state_forward=%d, "
    "gaxis_state_latch=%d, "
    "gaxis_latch_occurred=%d, "
    "gaxis_stop_code=%d, "
    "gaxis_reference_position=%d, "
    "gaxis_motor_position=%d, "
    "gaxis_position_error=%d, "
    "gaxis_auxiliary_position=%d, "
    "gaxis_velocity=%d, "
    "gaxis_torque=%d, "
    "gaxis_analog_in=%d, "
    "haxis_motor_off=%d, "
    "haxis_error_armed=%d, "
    "haxis_latch_armed=%d, "
    "haxis_decelerating=%d, "
    "haxis_stopping=%d, "
    "haxis_slewing=%d, "
    "haxis_contour_mode=%d, "
    "haxis_negative_direction=%d, "
    "haxis_motion_mode=%d, "
    "haxis_home_1=%d, "
    "haxis_home_2=%d, "
    "haxis_homing=%d, "
    "haxis_finding_edge=%d, "
    "haxis_pa_motion=%d, "
    "haxis_pr_motion=%d, "
    "haxis_moving=%d, "
    "haxis_sm_jumper=%d, "
    "haxis_state_home=%d, "
    "haxis_state_reverse=%d, "
    "haxis_state_forward=%d, "
    "haxis_state_latch=%d, "
    "haxis_latch_occurred=%d, "
    "haxis_stop_code=%d, "
    "haxis_reference_position=%d, "
    "haxis_motor_position=%d, "
    "haxis_position_error=%d, "
    "haxis_auxiliary_position=%d, "
    "haxis_velocity=%d, "
    "haxis_torque=%d, "
    "haxis_analog_in=%d, "
    "a_encoder=%d, "
    "b_encoder=%d, "
    "c_encoder=%d, "
    "a_position=%.3f, "
    "b_position=%.3f, "
    "c_position=%.3f, "
    "simulate=%d, "
    "shutdown=%d, "
    "counter=%d, "
    "jd=%.8f, "
    "timestamp='%s'\n",
    (p->simulate > 0 ? "<SIMULATE> " : "<DIRECT> "),
    p->aaxis_enabled,
    p->baxis_enabled,
    p->caxis_enabled,
    p->daxis_enabled,
    p->eaxis_enabled,
    p->faxis_enabled,
    p->gaxis_enabled,
    p->haxis_enabled,
    p->saxis_enabled,
    p->taxis_enabled,
    p->iaxis_enabled,
    p->header_length,
    p->iaxis_sample_number,
    p->iaxis_general_input[0],
    p->iaxis_general_input[1],
    p->iaxis_general_input[2],
    p->iaxis_general_input[3],
    p->iaxis_general_input[4],
    p->iaxis_general_input[5],
    p->iaxis_general_input[6],
    p->iaxis_general_input[7],
    p->iaxis_general_input[8],
    p->iaxis_general_input[9],
    p->iaxis_general_output[0],
    p->iaxis_general_output[1],
    p->iaxis_general_output[2],
    p->iaxis_general_output[3],
    p->iaxis_general_output[4],
    p->iaxis_general_output[5],
    p->iaxis_general_output[6],
    p->iaxis_general_output[7],
    p->iaxis_general_output[8],
    p->iaxis_general_output[9],
    p->iaxis_error_code,
    p->iaxis_echo_on,
    p->iaxis_trace_on,
    p->iaxis_awaiting_input,
    p->iaxis_program_running,
    p->saxis_segment_count,
    p->saxis_decelerating,
    p->saxis_stopping,
    p->saxis_slewing,
    p->saxis_moving,
    p->saxis_distance_traveled,
    p->taxis_segment_count,
    p->taxis_decelerating,
    p->taxis_stopping,
    p->taxis_slewing,
    p->taxis_moving,
    p->taxis_distance_traveled,
    p->aaxis_motor_off,
    p->aaxis_error_armed,
    p->aaxis_latch_armed,
    p->aaxis_decelerating,
    p->aaxis_stopping,
    p->aaxis_slewing,
    p->aaxis_contour_mode,
    p->aaxis_negative_direction,
    p->aaxis_motion_mode,
    p->aaxis_home_1,
    p->aaxis_home_2,
    p->aaxis_homing,
    p->aaxis_finding_edge,
    p->aaxis_pa_motion,
    p->aaxis_pr_motion,
    p->aaxis_moving,
    p->aaxis_sm_jumper,
    p->aaxis_state_home,
    p->aaxis_state_reverse,
    p->aaxis_state_forward,
    p->aaxis_state_latch,
    p->aaxis_latch_occurred,
    p->aaxis_stop_code,
    p->aaxis_reference_position,
    p->aaxis_motor_position,
    p->aaxis_position_error,
    p->aaxis_auxiliary_position,
    p->aaxis_velocity,
    p->aaxis_torque,
    p->aaxis_analog_in,
    p->baxis_motor_off,
    p->baxis_error_armed,
    p->baxis_latch_armed,
    p->baxis_decelerating,
    p->baxis_stopping,
    p->baxis_slewing,
    p->baxis_contour_mode,
    p->baxis_negative_direction,
    p->baxis_motion_mode,
    p->baxis_home_1,
    p->baxis_home_2,
    p->baxis_homing,
    p->baxis_finding_edge,
    p->baxis_pa_motion,
    p->baxis_pr_motion,
    p->baxis_moving,
    p->baxis_sm_jumper,
    p->baxis_state_home,
    p->baxis_state_reverse,
    p->baxis_state_forward,
    p->baxis_state_latch,
    p->baxis_latch_occurred,
    p->baxis_stop_code,
    p->baxis_reference_position,
    p->baxis_motor_position,
    p->baxis_position_error,
    p->baxis_auxiliary_position,
    p->baxis_velocity,
    p->baxis_torque,
    p->baxis_analog_in,
    p->caxis_motor_off,
    p->caxis_error_armed,
    p->caxis_latch_armed,
    p->caxis_decelerating,
    p->caxis_stopping,
    p->caxis_slewing,
    p->caxis_contour_mode,
    p->caxis_negative_direction,
    p->caxis_motion_mode,
    p->caxis_home_1,
    p->caxis_home_2,
    p->caxis_homing,
    p->caxis_finding_edge,
    p->caxis_pa_motion,
    p->caxis_pr_motion,
    p->caxis_moving,
    p->caxis_sm_jumper,
    p->caxis_state_home,
    p->caxis_state_reverse,
    p->caxis_state_forward,
    p->caxis_state_latch,
    p->caxis_latch_occurred,
    p->caxis_stop_code,
    p->caxis_reference_position,
    p->caxis_motor_position,
    p->caxis_position_error,
    p->caxis_auxiliary_position,
    p->caxis_velocity,
    p->caxis_torque,
    p->caxis_analog_in,
    p->daxis_motor_off,
    p->daxis_error_armed,
    p->daxis_latch_armed,
    p->daxis_decelerating,
    p->daxis_stopping,
    p->daxis_slewing,
    p->daxis_contour_mode,
    p->daxis_negative_direction,
    p->daxis_motion_mode,
    p->daxis_home_1,
    p->daxis_home_2,
    p->daxis_homing,
    p->daxis_finding_edge,
    p->daxis_pa_motion,
    p->daxis_pr_motion,
    p->daxis_moving,
    p->daxis_sm_jumper,
    p->daxis_state_home,
    p->daxis_state_reverse,
    p->daxis_state_forward,
    p->daxis_state_latch,
    p->daxis_latch_occurred,
    p->daxis_stop_code,
    p->daxis_reference_position,
    p->daxis_motor_position,
    p->daxis_position_error,
    p->daxis_auxiliary_position,
    p->daxis_velocity,
    p->daxis_torque,
    p->daxis_analog_in,
    p->eaxis_motor_off,
    p->eaxis_error_armed,
    p->eaxis_latch_armed,
    p->eaxis_decelerating,
    p->eaxis_stopping,
    p->eaxis_slewing,
    p->eaxis_contour_mode,
    p->eaxis_negative_direction,
    p->eaxis_motion_mode,
    p->eaxis_home_1,
    p->eaxis_home_2,
    p->eaxis_homing,
    p->eaxis_finding_edge,
    p->eaxis_pa_motion,
    p->eaxis_pr_motion,
    p->eaxis_moving,
    p->eaxis_sm_jumper,
    p->eaxis_state_home,
    p->eaxis_state_reverse,
    p->eaxis_state_forward,
    p->eaxis_state_latch,
    p->eaxis_latch_occurred,
    p->eaxis_stop_code,
    p->eaxis_reference_position,
    p->eaxis_motor_position,
    p->eaxis_position_error,
    p->eaxis_auxiliary_position,
    p->eaxis_velocity,
    p->eaxis_torque,
    p->eaxis_analog_in,
    p->faxis_motor_off,
    p->faxis_error_armed,
    p->faxis_latch_armed,
    p->faxis_decelerating,
    p->faxis_stopping,
    p->faxis_slewing,
    p->faxis_contour_mode,
    p->faxis_negative_direction,
    p->faxis_motion_mode,
    p->faxis_home_1,
    p->faxis_home_2,
    p->faxis_homing,
    p->faxis_finding_edge,
    p->faxis_pa_motion,
    p->faxis_pr_motion,
    p->faxis_moving,
    p->faxis_sm_jumper,
    p->faxis_state_home,
    p->faxis_state_reverse,
    p->faxis_state_forward,
    p->faxis_state_latch,
    p->faxis_latch_occurred,
    p->faxis_stop_code,
    p->faxis_reference_position,
    p->faxis_motor_position,
    p->faxis_position_error,
    p->faxis_auxiliary_position,
    p->faxis_velocity,
    p->faxis_torque,
    p->faxis_analog_in,
    p->gaxis_motor_off,
    p->gaxis_error_armed,
    p->gaxis_latch_armed,
    p->gaxis_decelerating,
    p->gaxis_stopping,
    p->gaxis_slewing,
    p->gaxis_contour_mode,
    p->gaxis_negative_direction,
    p->gaxis_motion_mode,
    p->gaxis_home_1,
    p->gaxis_home_2,
    p->gaxis_homing,
    p->gaxis_finding_edge,
    p->gaxis_pa_motion,
    p->gaxis_pr_motion,
    p->gaxis_moving,
    p->gaxis_sm_jumper,
    p->gaxis_state_home,
    p->gaxis_state_reverse,
    p->gaxis_state_forward,
    p->gaxis_state_latch,
    p->gaxis_latch_occurred,
    p->gaxis_stop_code,
    p->gaxis_reference_position,
    p->gaxis_motor_position,
    p->gaxis_position_error,
    p->gaxis_auxiliary_position,
    p->gaxis_velocity,
    p->gaxis_torque,
    p->gaxis_analog_in,
    p->haxis_motor_off,
    p->haxis_error_armed,
    p->haxis_latch_armed,
    p->haxis_decelerating,
    p->haxis_stopping,
    p->haxis_slewing,
    p->haxis_contour_mode,
    p->haxis_negative_direction,
    p->haxis_motion_mode,
    p->haxis_home_1,
    p->haxis_home_2,
    p->haxis_homing,
    p->haxis_finding_edge,
    p->haxis_pa_motion,
    p->haxis_pr_motion,
    p->haxis_moving,
    p->haxis_sm_jumper,
    p->haxis_state_home,
    p->haxis_state_reverse,
    p->haxis_state_forward,
    p->haxis_state_latch,
    p->haxis_latch_occurred,
    p->haxis_stop_code,
    p->haxis_reference_position,
    p->haxis_motor_position,
    p->haxis_position_error,
    p->haxis_auxiliary_position,
    p->haxis_velocity,
    p->haxis_torque,
    p->haxis_analog_in,
    p->a_encoder,
    p->b_encoder,
    p->c_encoder,
    p->a_position,
    p->b_position,
    p->c_position,
    p->simulate,
    p->shutdown,
    p->counter,
    p->jd,
    p->timestamp);
  (void) fflush(stderr);
}


/*******************************************************************************
 * gclib_check( ... )
 ******************************************************************************/
void gclib_check(GReturn rc, GCon *g, bool close_on_error, bool exit_on_error) {
  if (rc != G_NO_ERROR) {
    (void) fprintf(stderr, "<ERROR> %d\n", (int)rc);
    (void) fflush(stderr);
    if (close_on_error && *g) { (void) GClose(*g); }
    if (exit_on_error) { exit(rc); }
  }
}


/*******************************************************************************
 * get_jd( ... )
 ******************************************************************************/
double get_jd(struct tm *tm) {

   /* declare some variables and initialize them */
   double dyear = (double)tm->tm_year + 1900.0;
   double dmonth = (double)tm->tm_mon + 1.0;
   double dday = (double)tm->tm_mday;
   double dhour = (double)tm->tm_hour;
   double dminute = (double)tm->tm_min;
   double dsecond = (double)tm->tm_sec;
   double Gregorian = 1582.0 + 10.0/12.0 + 15.0/365.25;
   double A = NAN;
   double B = NAN;
   double C = NAN;
   double JD = NAN;

   /* calculate */
   if (tm == (struct tm *)NULL) { return (double)NAN; }
   if (dmonth < 3.0) {
      dyear  -= 1.0;
      dmonth += 12.0;
   }

   if ((dyear + dmonth/12.0 + dday/365.25) >= Gregorian) {
      A = (double)((int)(dyear/100.0));
      B = 2.0 - A + (double)((int)(A/4.0));
   } else {
      B = 0.0;
   }

   if (dyear >= 0.0) {
      JD = (double)((int)( 365.25 * dyear)) + (double)((int)( 30.6001 * (dmonth + 1.0))) + dday + 1720994.5 + B;
   } else {
      JD = (double)((int)( 365.25 * dyear - 0.75)) + (double)((int)( 30.6001 * (dmonth + 1.0))) + dday + 1720994.5 + B;
   }
   C = (dhour + dminute/60.0 + dsecond/3600.0) / 24.0;

   /* return */
   return (JD + C);
}


/*******************************************************************************
 * read_filters_from_file( ... )
 ******************************************************************************/
void read_filters_from_file(char *fname, filter_file_t filters[], int max_filters, int ncolumns) {

  /* declare some variables and initialize them */
  char buffer[BOK_STR_256] = {'\0'};
  char *p = (char *)NULL;
  FILE *fp = (FILE *)NULL;
  int istat = 0;
  int idx = 0;
  filter_file_t this_filter;

  /* open the file and read it */
  (void) memset((void *)buffer, '\0', sizeof(buffer));
  if ((fp=fopen(fname, "r")) != (FILE *)NULL) {
    while ((p=fgets(buffer, sizeof(buffer), fp)) != (char *)NULL) {
      (void) memset((void *)&this_filter, '\0', sizeof(this_filter));
      if (strlen(buffer)>0 && buffer[0]!='#') {
        chomp(buffer, "\n");
        if ((istat=sscanf(buffer, "%d\t%s\t%s", &idx, this_filter.code, this_filter.name)) == ncolumns) {
          (void) strcpy(filters[idx].code, this_filter.code);
          (void) strcpy(filters[idx].name, this_filter.name);
        }
      }
      (void) memset((void *)buffer, '\0', sizeof(buffer));
    }
  }
}


/*******************************************************************************
 * save_nominal_plane_to_file( ... )
 ******************************************************************************/
void save_nominal_plane_to_file(char *fname, float np1, float np2, float np3) {

  /* declare some variables and initialize them */
  char buffer[BOK_STR_64] = {'\0'};
  FILE *fp = (FILE *)NULL;

  /* open the file and write it */
  (void) memset((void *)buffer, '\0', sizeof(buffer));
  (void) sprintf(buffer, "%.3f\t%.3f\t%.3f\n", np1, np2, np3);
  if ((fp=fopen(fname, "w+")) != (FILE *)NULL) {
    (void) fputs(buffer, fp);
    (void) fclose(fp);
  }
}


/*******************************************************************************
 * read_nominal_plane_from_file( ... )
 ******************************************************************************/
void read_nominal_plane_from_file(char *fname, float *np1, float *np2, float *np3) {

  /* declare some variables and initialize them */
  char buffer[BOK_STR_256] = {'\0'};
  char *p = (char *)NULL;
  FILE *fp = (FILE *)NULL;
  int istat = 0;

  /* open the file and read it */
  (void) memset((void *)buffer, '\0', sizeof(buffer));
  if ((fp=fopen(fname, "r")) != (FILE *)NULL) {
    while ((p=fgets(buffer, sizeof(buffer), fp)) != (char *)NULL) {
      if (buffer[0] != '#' && strlen(buffer)>0) {
        chomp(buffer, "\n");
        if ((istat=sscanf(buffer, "%f\t%f\t%f", np1, np2, np3)) != 3) {
          *np1 = NAN;
          *np2 = NAN;
          *np3 = NAN;
        }
      }
    }
  }
}


/*******************************************************************************
 * replace( ... )
 ******************************************************************************/
void replace(char *s, char *x, char *z) {
  while (*s++) {
    if (*s!=EOF && *s==*x && strlen(z)!=0) *s = *z;
  }
}


/*******************************************************************************
 * replace_word( ... )
 ******************************************************************************/
void replace_word(char *haystack, size_t haystack_len, const char *needle, const char *newpin) {

  /* declare some variables and initialize them */
  char *original = haystack;
  char* result = (char *)NULL;
  int i = 0;
  int cnt = 0;
  int newpin_len = strlen(newpin);
  int needle_len = strlen(needle);

  /* counting the number of times needle occurs in haystack */
  for (i=0; i<haystack_len; i++) {
    if (strstr(&haystack[i], needle) == &haystack[i]) {
      cnt++;
      i += needle_len - 1;
    }
  }

  /* create new string of enough length */
  result = (char *)malloc(i + cnt * (newpin_len - needle_len) + 1);

  /* process */
  i = 0;
  while (*haystack) {
      if (strstr(haystack, needle) == haystack) {
        strcpy(&result[i], newpin);
        i += newpin_len;
        haystack += needle_len;
      } else {
        result[i++] = *haystack++;
      }
  }
  result[i] = '\0';

  /* copy string back to original */
  (void) memset((void *)original, '\0', haystack_len);
  (void) memcpy((void *)original, (void *)result, haystack_len);

  /* free memory */
  free(result);
}


/*******************************************************************************
 * logtime( ... )
 ******************************************************************************/
void logtime(const char * format, ...) {
    
  /* obtain current time */
  time_t ctime_s;
  (void) memset((void *)&ctime_s, '\0', sizeof(ctime_s));
  if ((ctime_s=time(NULL)) < (time_t)0) { return; }

  /* convert to local time format */
  char *ctime_p = (char *)NULL;
  if ((ctime_p=ctime(&ctime_s)) == (char *)NULL) { return; }

  /* create buffer */
  char buffer[BOK_STR_32];
  (void) memset((void *)&buffer, '\0', sizeof(buffer));
  (void) snprintf(buffer, strlen(ctime_p), "%s", ctime_p);
  chomp(buffer, "\n");
  chomp(buffer, "\r");
  (void) printf("%s", buffer);

  /* format */
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  (void) fflush(stdout);
}


/*******************************************************************************
 * xq( ... )
 ******************************************************************************/
GReturn xq(char *cmd) {

  /* declare some variables and initialize them */
  GCon gfd = (GCon)0;
  GReturn gstat = (GReturn)0;
  GReturn gerror = (GReturn)0;
  char resp[BOK_STR_2048] = {'\0'};
  (void) memset((void *)resp, 0, sizeof(resp));

  /* execute command */
  if ((gstat=GOpen(BOK_GALIL_CMD_BOK, &gfd)) == G_NO_ERROR) {
    if ((gstat=GCommand(gfd, cmd, resp, sizeof(resp), 0)) == G_NO_ERROR) {
      (void) fprintf(stdout, "Executed '%s' OK, gstat=%d, response='%s'\n", cmd, gstat, resp); (void) fflush(stdout);
    } else {
      (void) fprintf(stderr, "<ERROR> Failed to execute '%s', gstat=%d, response='%s'\n", cmd, gstat, resp); (void) fflush(stderr);
      (void) memset((void *)resp, 0, sizeof(resp));
      gerror = GCommand(gfd, "TC 1;", resp, sizeof(resp), 0);
      (void) fprintf(stderr, "<INFO> '%s', gerror=%d, response='%s'\n", "TC 1", gerror, resp); (void) fflush(stderr);
    }
  } else {
    (void) fprintf(stderr, "<ERROR> failed to open Galil_DMC_22x0 for '%s', gstat=%d\n", cmd, gstat); (void) fflush(stderr);
  }
  if (gfd) { (void) GClose(gfd); }

  /* return */
  return gstat;
}


/*******************************************************************************
 * xq_coordmv( ... )
 ******************************************************************************/
#define XQ_COORDMV "XQ #COORDMV;"
GReturn xq_coordmv(void) { return xq(XQ_COORDMV); }


/*******************************************************************************
 * xq_filtin( ... )
 ******************************************************************************/
#define XQ_FILTIN "XQ #FILTIN,3;"
GReturn xq_filtin(void) { return xq(XQ_FILTIN); }


/*******************************************************************************
 * xq_filtldm( ... )
 ******************************************************************************/
#define XQ_FILTLDM "XQ #FILTLDM;"
GReturn xq_filtldm(void) { return xq(XQ_FILTLDM); }


/*******************************************************************************
 * xq_filtmov( ... )
 ******************************************************************************/
#define XQ_FILTMOV "XQ #FILTMOV;"
GReturn xq_filtmov(void) { return xq(XQ_FILTMOV); }


/*******************************************************************************
 * xq_filtout( ... )
 ******************************************************************************/
#define XQ_FILTOUT "XQ #FILTOUT;"
GReturn xq_filtout(void) { return xq(XQ_FILTOUT); }


/*******************************************************************************
 * xq_filtrd( ... )
 ******************************************************************************/
#define XQ_FILTRD "XQ #FILTRD;"
GReturn xq_filtrd(void) { return xq(XQ_FILTRD); }


/*******************************************************************************
 * xq_focind( ... )
 ******************************************************************************/
#define XQ_FOCIND "XQ #FOCIND;"
GReturn xq_focind(void) { return xq(XQ_FOCIND); }


/*******************************************************************************
 * xq_focgcam( ... )
 ******************************************************************************/
#define XQ_FOCGCAM "XQ #FOCGCAM;"
GReturn xq_focgcam(void) { return xq(XQ_FOCGCAM); }


/*******************************************************************************
 * xq_gfwinit( ... )
 ******************************************************************************/
#define XQ_GFWINIT "XQ #GFWINIT;"
GReturn xq_gfwinit(void) { return xq(XQ_GFWINIT); }


/*******************************************************************************
 * xq_gfwmov( ... )
 ******************************************************************************/
#define XQ_GFWMOV "XQ #GFWMOV;"
GReturn xq_gfwmov(void) { return xq(XQ_GFWMOV); }


/*******************************************************************************
 * xq_hx( ... )
 ******************************************************************************/
#define XQ_HX "HX;"
GReturn xq_hx(void) { return xq(XQ_HX); }


/*******************************************************************************
 * xq_dista( ... )
 ******************************************************************************/
GReturn xq_dista(float fval) {
  char dista[BOK_STR_64] = {'\0'};
  (void) memset((void *)dista, 0, sizeof(dista));
  (void) sprintf(dista, "DISTA=%f;", fval);
  return xq(dista);
}


/*******************************************************************************
 * xq_distall( ... )
 ******************************************************************************/
GReturn xq_distall(float fval) {
  char distall[BOK_STR_64] = {'\0'};
  (void) memset((void *)distall, 0, sizeof(distall));
  (void) sprintf(distall, "DISTALL=%f;", fval);
  return xq(distall);
}


/*******************************************************************************
 * xq_distb( ... )
 ******************************************************************************/
GReturn xq_distb(float fval) {
  char distb[BOK_STR_64] = {'\0'};
  (void) memset((void *)distb, 0, sizeof(distb));
  (void) sprintf(distb, "DISTB=%f;", fval);
  return xq(distb);
}


/*******************************************************************************
 * xq_distc( ... )
 ******************************************************************************/
GReturn xq_distc(float fval) {
  char distc[BOK_STR_64] = {'\0'};
  (void) memset((void *)distc, 0, sizeof(distc));
  (void) sprintf(distc, "DISTC=%f;", fval);
  return xq(distc);
}


/*******************************************************************************
 * xq_distgcam( ... )
 ******************************************************************************/
GReturn xq_distgcam(float fval) {
  char distgcam[BOK_STR_64] = {'\0'};
  (void) memset((void *)distgcam, 0, sizeof(distgcam));
  (void) sprintf(distgcam, "DISTGCAM=%f;", fval);
  return xq(distgcam);
}


/*******************************************************************************
 * xq_gfiltn( ... )
 ******************************************************************************/
GReturn xq_gfiltn(float fval) {
  char gfiltn[BOK_STR_64] = {'\0'};
  (void) memset((void *)gfiltn, 0, sizeof(gfiltn));
  (void) sprintf(gfiltn, "GFILTN=%f;", fval);
  return xq(gfiltn);
}


/*******************************************************************************
 * xq_reqfilt( ... )
 ******************************************************************************/
GReturn xq_reqfilt(float fval) {
  char reqfilt[BOK_STR_64] = {'\0'};
  (void) memset((void *)reqfilt, 0, sizeof(reqfilt));
  (void) sprintf(reqfilt, "REQFILT=%f;", fval);
  return xq(reqfilt);
}


/*******************************************************************************
 * xq_focusall( ... )
 ******************************************************************************/
GReturn xq_focusall(float a) {
  GReturn gstat = (GReturn)0;
  if ((gstat=xq_distall(a)) != G_NO_ERROR) { return gstat; }
  return xq_coordmv();
}


/*******************************************************************************
 * xq_focusind( ... )
 ******************************************************************************/
GReturn xq_focusind(float a, float b, float c) {
  GReturn gstat = (GReturn)0;
  if ((gstat=xq_dista(a)) != G_NO_ERROR) { return gstat; }
  if ((gstat=xq_distb(b)) != G_NO_ERROR) { return gstat; }
  if ((gstat=xq_distc(c)) != G_NO_ERROR) { return gstat; }
  return xq_focind();
}

/*******************************************************************************
 * xq_gfocus( ... )
 ******************************************************************************/
GReturn xq_gfocus(float a) {
  GReturn gstat = (GReturn)0;
  if ((gstat=xq_distgcam(a)) != G_NO_ERROR) { return gstat; }
  return xq_focgcam();
}
