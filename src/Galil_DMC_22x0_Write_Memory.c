/*******************************************************************************
 *
 * Galil_DMC_22x0_Write_Memory.c
 *
 ******************************************************************************/


/*******************************************************************************
 * include(s)
 ******************************************************************************/
#include "__hosts__.h"
#include "bokGalil.h"


/*******************************************************************************
 * define(s)
 ******************************************************************************/
#define _HELP_ "Command(s) for the Galil_DMC_22x0 via gclib"
#define _NAME_ "Galil_DMC_22x0_Write_Memory"


/*******************************************************************************
 * signal(s)
 ******************************************************************************/
static int running = 1;
void galil_handler(int mysig) { running = 0; }


/*******************************************************************************
 * function(s)
 ******************************************************************************/
int msleep(long msec) {

  /* declare some variables and initialize them */
  int res = 0;
  struct timespec ts;
  (void) memset((void *)&ts, 0, sizeof(ts));

  if (msec < 0) { return -1; }
  ts.tv_sec = msec / 1000;
  ts.tv_nsec = (msec % 1000) * 1000000;

  do {
    res = nanosleep(&ts, &ts);
  } while (res && errno == EINTR);

  return res;
}


/*******************************************************************************
 * mem command(s)
 ******************************************************************************/
typedef struct mem_command_type {
  char name[BOK_STR_64];
  char help[BOK_STR_128];
} mem_cmd_t, *mem_cmd_p, **mem_cmd_s;

const mem_cmd_t MemCmnds[] = {
  {"FILTVALS[0]=?;",     "filter number in slot 0",            },
  {"FILTVALS[1]=?;",     "filter number in slot 1",            },
  {"FILTVALS[2]=?;",     "filter number in slot 2",            },
  {"FILTVALS[3]=?;",     "filter number in slot 3",            },
  {"FILTVALS[4]=?;",     "filter number in slot 4",            },
  {"FILTVALS[5]=?;",     "filter number in slot 5",            },
  {"LV;",                "list variable",                      },
  {"TB;",                "tell status byte",                   },
  {"TP;",                "tell (motor) position",              },
  {"QR;",                "tell (data) record",                 }
};
#define MEM_CMD_NELMS (sizeof(MemCmnds)/sizeof(mem_cmd_t))


/*******************************************************************************
 * function: main()
 ******************************************************************************/
int main( int argc, char *argv[] ) {

  /* declare some variables and initialize them */
  bool simulate = false;
  char buffer[BOK_STR_2048] = {'\0'};
  char command[BOK_STR_64] = {'\0'};
  char lv_name[BOK_STR_64] = {'\0'};
  char *ep = (char *)NULL;
  char *sp = (char *)NULL;
  char *pr = (char *)NULL;
  char *tok = (char *)NULL;
  float fval = NAN;
  GCon gfd = G_NO_ERROR;
  GReturn gstat = G_NO_ERROR;
  int bok = 1;
  int counter = -1;
  int delay = BOK_TCP_DELAY_MS + BOK_UDP_DELAY_MS;
  time_t now = (time_t)NULL;

  int tcp_shm_fd = -1;
  int tcp_shm_stat = -1;
  tcp_val_p tcp_shm_p = (tcp_val_p)NULL;
  tcp_val_t tcp_val;

  int udp_shm_fd = -1;
  int udp_shm_stat = -1;
  udp_val_p udp_shm_p = (udp_val_p)NULL;
  udp_val_t udp_val;

  qr_record_t qr;

  /* get command line parameter(s) */
  while ( argc>1L && argv[1][0]=='-' ) {
    switch (argv[1][1]) {
      case 'b': case 'B': bok = atoi(&argv[1][2]); break;
      case 's': case 'S':
        if (isalpha((char)argv[1][2])) {
          simulate = (argv[1][2]=='t' || argv[1][2]=='T') ? true: false;
        } else if (isdigit((char)argv[1][2])) {
          simulate = atoi(&argv[1][2]) > 0 ? true: false;
        }
        break;
      default:
        (void) fprintf(stdout, "%s: v%s %s %s\n", _NAME_, _VERSION_, _AUTHOR_, _DATE_);
        (void) fprintf(stdout, "%s\n", _HELP_);
        (void) fprintf(stdout, "\nUse: %s [-b<int> -s<bool> -h]\n", _NAME_);
        (void) fprintf(stdout, "\t-b<int>  : 1=bok, 0=lab [default=%d]\n", bok);
        (void) fprintf(stdout, "\t-s<bool> : set simulate (t||f) [default=%s]\n", "false");
        (void) fflush(stdout);
        exit (0);
        break;
    }
    argc--; argv++;
  }

  /* install signal handler */
  signal(SIGINT, galil_handler);
  srand(time(0));

  /* create TCP shared memory segment */
  (void) shm_unlink(BOK_SHM_TCP_NAME);
  if ((tcp_shm_fd=shm_open(BOK_SHM_TCP_NAME, O_CREAT | O_RDWR, 0777)) < 0) {
    (void) fprintf(stderr, "%s <ERROR> failed to execute '%s' exiting, tcp_shm_fd=%d\n", _NAME_, "shm_open()", tcp_shm_fd);
    (void) fflush(stderr);
    exit(tcp_shm_fd);
  }

  /* (re)size TCP shared memory segment */
  if ((tcp_shm_stat=ftruncate(tcp_shm_fd, TCP_VAL_SIZE)) < 0) {
    (void) fprintf(stderr, "%s <ERROR> failed to execute '%s' exiting, tcp_shm_stat=%d\n", _NAME_, "ftruncate()", tcp_shm_stat);
    (void) fflush(stderr);
    exit(tcp_shm_stat);
  }
  
  /* memory map the TCP shared memory object */
  if ((tcp_shm_p=(tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, tcp_shm_fd, 0)) == (tcp_val_p)NULL) {
    (void) fprintf(stderr, "%s <ERROR> failed to execute '%s' exiting, tcp_shm_p=%p\n", _NAME_, "mmap()", tcp_shm_p);
    (void) fflush(stderr);
    exit(-1);
  }

  /* create UDP shared memory segment */
  (void) shm_unlink(BOK_SHM_UDP_NAME);
  if ((udp_shm_fd=shm_open(BOK_SHM_UDP_NAME, O_CREAT | O_RDWR, 0777)) < 0) {
    (void) fprintf(stderr, "%s <ERROR> failed to execute '%s' exiting, udp_shm_fd=%d\n", _NAME_, "shm_open()", udp_shm_fd);
    (void) fflush(stderr);
    exit(udp_shm_fd);
  }

  /* (re)size UDP shared memory segment */
  if ((udp_shm_stat=ftruncate(udp_shm_fd, UDP_VAL_SIZE)) < 0) {
    (void) fprintf(stderr, "%s <ERROR> failed to execute '%s' exiting, udp_shm_stat=%d\n", _NAME_, "ftruncate()", udp_shm_stat);
    (void) fflush(stderr);
    exit(udp_shm_stat);
  }
  
  /* memory map the UDP shared memory object */
  if ((udp_shm_p=(udp_val_p)mmap(0, UDP_VAL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, udp_shm_fd, 0)) == (udp_val_p)NULL) {
    (void) fprintf(stderr, "%s <ERROR> failed to execute '%s' exiting, udp_shm_ptr=%p\n", _NAME_, "mmap()", udp_shm_p);
    (void) fflush(stderr);
    exit(-1);
  }
  /* open the ip_addr */
  gstat = G_NO_ERROR;
  (void) memset((void *)command, 0, sizeof(command));
  if (! simulate ) {
    if (bok > 0) {
      (void) sprintf(command, "%s --command TCP", BOK_GALIL_CMD_BOK);
      (void) fprintf(stdout, "%s <%s> executing '%s', gstat=%d, gfd=%p\n", _NAME_, (simulate == true ? "SIM" : "OK"), command, gstat, gfd);
      (void) fflush(stdout);
      if ((gstat=GOpen(command, &gfd)) != G_NO_ERROR) { simulate = true; }
      (void) printf("GOpen('%s') called, gstat=%d, gfd=%ld\n", command, (int)gstat, (long)gfd);
      (void) fprintf(stdout, "%s <%s> executed '%s', gstat=%d, gfd=%p\n", _NAME_, (simulate == true ? "SIM" : "OK"), command, gstat, gfd);
      (void) fflush(stdout);
    } else {
      (void) sprintf(command, "%s --command TCP", BOK_GALIL_CMD_LAB);
      (void) fprintf(stdout, "%s <%s> executing '%s', gstat=%d, gfd=%p\n", _NAME_, (simulate == true ? "SIM" : "OK"), command, gstat, gfd);
      (void) fflush(stdout);
      if ((gstat=GOpen(command, &gfd)) != G_NO_ERROR) { simulate = true; }
      (void) printf("GOpen('%s') called, gstat=%d, gfd=%ld\n", command, (int)gstat, (long)gfd);
      (void) fprintf(stdout, "%s <%s> executed '%s', gstat=%d, gfd=%p\n", _NAME_, (simulate == true ? "SIM" : "OK"), command, gstat, gfd);
      (void) fflush(stdout);
    }
  }

  /* loop for command(s) */
  while (running > 0) {

    /* reset record */
    (void) memset(&qr, 0, sizeof(qr));
    (void) memset((void *)&tcp_val, 0, sizeof(tcp_val));
    (void) memset((void *)&udp_val, 0, sizeof(udp_val));

    /* initialize element(s) */
    now = time(NULL);
    (void) strftime(tcp_val.timestamp, sizeof(tcp_val.timestamp), "%Y-%m-%dT%X.00", localtime(&now));
    tcp_val.jd = get_jd(localtime(&now));
    tcp_val.shutdown = 0;
    tcp_val.counter = ++counter;
    tcp_val.simulate = simulate == true ? 1 : 0;

    (void) strftime(udp_val.timestamp, sizeof(udp_val.timestamp), "%Y-%m-%dT%X.00", localtime(&now));
    udp_val.jd = get_jd(localtime(&now));
    udp_val.shutdown = 0;
    udp_val.counter = ++counter;
    udp_val.simulate = simulate == true ? 1 : 0;

    /* deconstruct record(s) */
    if (simulate) {

      /* TCP */
      tcp_val.filtvals[0]   = 18.0;
      tcp_val.filtvals[1]   = 2.0;
      tcp_val.filtvals[2]   = 3.0;
      tcp_val.filtvals[3]   = 4.0;
      tcp_val.filtvals[4]   = 5.0;
      tcp_val.filtvals[5]   = 6.0;
      tcp_val.status        = 65;
      tcp_val.position[0]   = -436.0;
      tcp_val.position[1]   = -1178.0;
      tcp_val.position[2]   = 633.0;
      tcp_val.position[3]   = NAN;
      tcp_val.position[4]   = NAN;
      tcp_val.position[5]   = NAN;
      tcp_val.position[6]   = NAN;
      tcp_val.position[7]   = NAN;
      tcp_val.lv.dista      = 200.0;
      tcp_val.lv.distall    = 200.0;
      tcp_val.lv.distb      = 200.0;
      tcp_val.lv.distc      = 200.0;
      tcp_val.lv.distgcam   = 100.0;
      tcp_val.lv.errfilt    = 0.0;
      tcp_val.lv.filtbit1   = 0.0;
      tcp_val.lv.filtbit2   = 0.0;
      tcp_val.lv.filtbit3   = 0.0;
      tcp_val.lv.filtbit4   = 1.0;
      tcp_val.lv.filtbit5   = 0.0;
      tcp_val.lv.filtbit6   = 1.0;
      tcp_val.lv.filtbit7   = 1.0;
      tcp_val.lv.filtbit8   = 1.0;
      tcp_val.lv.filthigh   = 224.0;
      tcp_val.lv.filtisin   = 0.0;
      tcp_val.lv.filtlow    = 8.0;
      tcp_val.lv.filtnum    = 6.0;
      tcp_val.lv.filtrac    = 5120.0;
      tcp_val.lv.filtrbl    = -99999.0;
      tcp_val.lv.filtrdc    = 14336.0;
      tcp_val.lv.filtrfl    = 99999.0;
      tcp_val.lv.filtrsp    = 500.0;
      tcp_val.lv.filttac    = 5120.0;
      tcp_val.lv.filttdc    = 99999.0;
      tcp_val.lv.filttdis   = 21000.0;
      tcp_val.lv.filttnud   = 200.0;
      tcp_val.lv.filttsc    = 3.0;
      tcp_val.lv.filttsp    = 5000.0;
      tcp_val.lv.filtval    = 23.0;
      tcp_val.lv.fnum       = 0.0;
      tcp_val.lv.fnum_in    = 0.0;
      tcp_val.lv.focac      = 9216.0;
      tcp_val.lv.focbl      = -99999.0;
      tcp_val.lv.focdc      = 9216.0;
      tcp_val.lv.focfl      = 99999.0;
      tcp_val.lv.focrefa    = 0.0;
      tcp_val.lv.focrefb    = 0.0;
      tcp_val.lv.focrefc    = 0.0;
      tcp_val.lv.focrfset   = 0.0;
      tcp_val.lv.focsp      = 600.0;
      tcp_val.lv.gfac       = 1024.0;
      tcp_val.lv.gfcent     = 5.0;
      tcp_val.lv.gfdc       = 1024.0;
      tcp_val.lv.gfiltac    = 9216.0;
      tcp_val.lv.gfiltdc    = 9216.0;
      tcp_val.lv.gfiltn     = 99999.0;
      tcp_val.lv.gfiltq     = 99999.0;
      tcp_val.lv.gfiltreq   = 0.0;
      tcp_val.lv.gfiltsp    = 500.0;
      tcp_val.lv.gfsp       = 100.0;
      tcp_val.lv.gifltn     = 0.0;
      tcp_val.lv.initfilt   = 1.0;
      tcp_val.lv.nmoves     = 5.0;
      tcp_val.lv.nrot       = 1.0;
      tcp_val.lv.reqfilt    = 15.0;
      tcp_val.lv.snum       = 0.0;
      tcp_val.lv.snum_in    = 0.0;
      tcp_val.lv.totfoca    = 0.0;
      tcp_val.lv.totfocb    = 0.0;
      tcp_val.lv.totfocc    = 0.0;
      tcp_val.lv.vecac      = 9216.0;
      tcp_val.lv.vecdc      = 9216.0;
      tcp_val.lv.vecsp      = 1040.0;
      tcp_val.gstatus       = (GReturn)0;

      /* UDP */
      udp_val.aaxis_enabled = 1;
      udp_val.baxis_enabled = 1;
      udp_val.caxis_enabled = 1;
      udp_val.daxis_enabled = 0;
      udp_val.eaxis_enabled = 0;
      udp_val.faxis_enabled = 1;
      udp_val.gaxis_enabled = 1;
      udp_val.haxis_enabled = 1;
      udp_val.saxis_enabled = 1;
      udp_val.taxis_enabled = 1;
      udp_val.iaxis_enabled = 1;
      udp_val.header_length = 268;
      /* iaxis */
      udp_val.iaxis_sample_number = 24322;
      udp_val.iaxis_general_input[0] = 249;
      udp_val.iaxis_general_input[1] = 186;
      udp_val.iaxis_general_input[2] = 255;
      udp_val.iaxis_general_input[3] = 255;
      udp_val.iaxis_general_input[4] = 255;
      udp_val.iaxis_general_input[5] = 255;
      udp_val.iaxis_general_input[6] = 255;
      udp_val.iaxis_general_input[7] = 255;
      udp_val.iaxis_general_input[8] = 255;
      udp_val.iaxis_general_input[9] = 255;
      udp_val.iaxis_general_output[0] = 0;
      udp_val.iaxis_general_output[1] = 0;
      udp_val.iaxis_general_output[2] = 0;
      udp_val.iaxis_general_output[3] = 0;
      udp_val.iaxis_general_output[4] = 0;
      udp_val.iaxis_general_output[5] = 0;
      udp_val.iaxis_general_output[6] = 0;
      udp_val.iaxis_general_output[7] = 0;
      udp_val.iaxis_general_output[8] = 0;
      udp_val.iaxis_general_output[9] = 0;
      udp_val.iaxis_error_code      = 0;
      udp_val.iaxis_echo_on         = 1;
      udp_val.iaxis_trace_on        = 0;
      udp_val.iaxis_program_running = 0;
      /* saxis */
      udp_val.saxis_segment_count     = 1;
      udp_val.saxis_decelerating      = 0;
      udp_val.saxis_stopping          = 0;
      udp_val.saxis_slewing           = 0;
      udp_val.saxis_moving            = 0;
      udp_val.saxis_distance_traveled = 311;
      /* taxis */
      udp_val.taxis_segment_count     = 0;
      udp_val.taxis_decelerating      = 0;
      udp_val.taxis_stopping          = 0;
      udp_val.taxis_slewing           = 0;
      udp_val.taxis_moving            = 0;
      udp_val.taxis_distance_traveled = 0;
      /* aaxis */
      udp_val.aaxis_motor_off          = 1;
      udp_val.aaxis_error_armed        = 0;
      udp_val.aaxis_latch_armed        = 0;
      udp_val.aaxis_decelerating       = 1;
      udp_val.aaxis_stopping           = 0;
      udp_val.aaxis_slewing            = 1;
      udp_val.aaxis_contour_mode       = 0;
      udp_val.aaxis_negative_direction = 0;
      udp_val.aaxis_motion_mode        = 0;
      udp_val.aaxis_home_1             = 0;
      udp_val.aaxis_home_2             = 0;
      udp_val.aaxis_homing             = 0;
      udp_val.aaxis_finding_edge       = 0;
      udp_val.aaxis_pa_motion          = 0;
      udp_val.aaxis_pr_motion          = 1;
      udp_val.aaxis_moving             = 0;
      udp_val.aaxis_sm_jumper          = 1;
      udp_val.aaxis_state_home         = 0;
      udp_val.aaxis_state_reverse      = 1;
      udp_val.aaxis_state_forward      = 1;
      udp_val.aaxis_state_latch        = 1;
      udp_val.aaxis_latch_occurred     = 1;
      udp_val.aaxis_stop_code          = 1;
      udp_val.aaxis_reference_position = -166;
      udp_val.aaxis_motor_position     = -826;
      udp_val.aaxis_position_error     = 0;
      udp_val.aaxis_auxiliary_position = -166;
      udp_val.aaxis_velocity           = 0;
      udp_val.aaxis_torque             = 0;
      udp_val.aaxis_analog_in          = 32752;
      /* baxis */
      udp_val.baxis_motor_off          = 1;
      udp_val.baxis_error_armed        = 0;
      udp_val.baxis_latch_armed        = 0;
      udp_val.baxis_decelerating       = 1;
      udp_val.baxis_stopping           = 0;
      udp_val.baxis_slewing            = 1;
      udp_val.baxis_contour_mode       = 0;
      udp_val.baxis_negative_direction = 0;
      udp_val.baxis_motion_mode        = 0;
      udp_val.baxis_home_1             = 0;
      udp_val.baxis_home_2             = 0;
      udp_val.baxis_homing             = 0;
      udp_val.baxis_finding_edge       = 0;
      udp_val.baxis_pa_motion          = 0;
      udp_val.baxis_pr_motion          = 1;
      udp_val.baxis_moving             = 0;
      udp_val.baxis_sm_jumper          = 1;
      udp_val.baxis_state_home         = 0;
      udp_val.baxis_state_reverse      = 1;
      udp_val.baxis_state_forward      = 1;
      udp_val.baxis_state_latch        = 0;
      udp_val.baxis_latch_occurred     = 1;
      udp_val.baxis_stop_code          = 1;
      udp_val.baxis_reference_position = -182;
      udp_val.baxis_motor_position     = -902;
      udp_val.baxis_position_error     = 0;
      udp_val.baxis_auxiliary_position = -182;
      udp_val.baxis_velocity           = 0;
      udp_val.baxis_torque             = 0;
      udp_val.baxis_analog_in          = 480;
      /* caxis */
      udp_val.caxis_motor_off          = 1;
      udp_val.caxis_error_armed        = 0;
      udp_val.caxis_latch_armed        = 0;
      udp_val.caxis_decelerating       = 1;
      udp_val.caxis_stopping           = 0;
      udp_val.caxis_slewing            = 1;
      udp_val.caxis_contour_mode       = 0;
      udp_val.caxis_negative_direction = 0;
      udp_val.caxis_motion_mode        = 0;
      udp_val.caxis_home_1             = 0;
      udp_val.caxis_home_2             = 0;
      udp_val.caxis_homing             = 0;
      udp_val.caxis_finding_edge       = 0;
      udp_val.caxis_pa_motion          = 0;
      udp_val.caxis_pr_motion          = 1;
      udp_val.caxis_moving             = 0;
      udp_val.caxis_sm_jumper          = 1;
      udp_val.caxis_state_home         = 0;
      udp_val.caxis_state_reverse      = 1;
      udp_val.caxis_state_forward      = 1;
      udp_val.caxis_state_latch        = 0;
      udp_val.caxis_latch_occurred     = 1;
      udp_val.caxis_stop_code          = 1;
      udp_val.caxis_reference_position = -30;
      udp_val.caxis_motor_position     = 144;
      udp_val.caxis_position_error     = 0;
      udp_val.caxis_auxiliary_position = -30;
      udp_val.caxis_velocity           = 0;
      udp_val.caxis_torque             = 0;
      udp_val.caxis_analog_in          = 32752;
      /* daxis */
      udp_val.daxis_motor_off          = 0;
      udp_val.daxis_error_armed        = 0;
      udp_val.daxis_latch_armed        = 0;
      udp_val.daxis_decelerating       = 0;
      udp_val.daxis_stopping           = 0;
      udp_val.daxis_slewing            = 0;
      udp_val.daxis_contour_mode       = 0;
      udp_val.daxis_negative_direction = 0;
      udp_val.daxis_motion_mode        = 0;
      udp_val.daxis_home_1             = 0;
      udp_val.daxis_home_2             = 0;
      udp_val.daxis_homing             = 0;
      udp_val.daxis_finding_edge       = 0;
      udp_val.daxis_pa_motion          = 0;
      udp_val.daxis_pr_motion          = 1;
      udp_val.daxis_moving             = 0;
      udp_val.daxis_sm_jumper          = 1;
      udp_val.daxis_state_home         = 0;
      udp_val.daxis_state_reverse      = 1;
      udp_val.daxis_state_forward      = 1;
      udp_val.daxis_state_latch        = 1;
      udp_val.daxis_latch_occurred     = 1;
      udp_val.daxis_stop_code          = 1;
      udp_val.daxis_reference_position = 0;
      udp_val.daxis_motor_position     = 0;
      udp_val.daxis_position_error     = 0;
      udp_val.daxis_auxiliary_position = 0;
      udp_val.daxis_velocity           = 0;
      udp_val.daxis_torque             = 0;
      udp_val.daxis_analog_in          = 560;
      /* eaxis */
      udp_val.eaxis_motor_off          = 1;
      udp_val.eaxis_error_armed        = 0;
      udp_val.eaxis_latch_armed        = 0;
      udp_val.eaxis_decelerating       = 1;
      udp_val.eaxis_stopping           = 0;
      udp_val.eaxis_slewing            = 1;
      udp_val.eaxis_contour_mode       = 0;
      udp_val.eaxis_negative_direction = 1;
      udp_val.eaxis_motion_mode        = 0;
      udp_val.eaxis_home_1             = 0;
      udp_val.eaxis_home_2             = 0;
      udp_val.eaxis_homing             = 0;
      udp_val.eaxis_finding_edge       = 0;
      udp_val.eaxis_pa_motion          = 0;
      udp_val.eaxis_pr_motion          = 1;
      udp_val.eaxis_moving             = 0;
      udp_val.eaxis_sm_jumper          = 1;
      udp_val.eaxis_state_home         = 0;
      udp_val.eaxis_state_reverse      = 1;
      udp_val.eaxis_state_forward      = 1;
      udp_val.eaxis_state_latch        = 0;
      udp_val.eaxis_latch_occurred     = 1;
      udp_val.eaxis_stop_code          = 1;
      udp_val.eaxis_reference_position = -5055;
      udp_val.eaxis_motor_position     = 0;
      udp_val.eaxis_position_error     = 0;
      udp_val.eaxis_auxiliary_position = -5055;
      udp_val.eaxis_velocity           = 0;
      udp_val.eaxis_torque             = 0;
      udp_val.eaxis_analog_in          = 32752;
      /* faxis */
      udp_val.faxis_motor_off          = 0;
      udp_val.faxis_error_armed        = 0;
      udp_val.faxis_latch_armed        = 0;
      udp_val.faxis_decelerating       = 0;
      udp_val.faxis_stopping           = 0;
      udp_val.faxis_slewing            = 1;
      udp_val.faxis_contour_mode       = 0;
      udp_val.faxis_negative_direction = 0;
      udp_val.faxis_motion_mode        = 0;
      udp_val.faxis_home_1             = 0;
      udp_val.faxis_home_2             = 1;
      udp_val.faxis_homing             = 1;
      udp_val.faxis_finding_edge       = 0;
      udp_val.faxis_pa_motion          = 0;
      udp_val.faxis_pr_motion          = 0;
      udp_val.faxis_moving             = 0;
      udp_val.faxis_sm_jumper          = 1;
      udp_val.faxis_state_home         = 1;
      udp_val.faxis_state_reverse      = 1;
      udp_val.faxis_state_forward      = 1;
      udp_val.faxis_state_latch        = 1;
      udp_val.faxis_latch_occurred     = 1;
      udp_val.faxis_stop_code          = 10;
      udp_val.faxis_reference_position = 0;
      udp_val.faxis_motor_position     = 0;
      udp_val.faxis_position_error     = 0;
      udp_val.faxis_auxiliary_position = 0;
      udp_val.faxis_velocity           = 0;
      udp_val.faxis_torque             = 0;
      udp_val.faxis_analog_in          = 560;
      /* gaxis */
      udp_val.gaxis_motor_off          = 0;
      udp_val.gaxis_error_armed        = 0;
      udp_val.gaxis_latch_armed        = 0;
      udp_val.gaxis_decelerating       = 1;
      udp_val.gaxis_stopping           = 0;
      udp_val.gaxis_slewing            = 1;
      udp_val.gaxis_contour_mode       = 0;
      udp_val.gaxis_negative_direction = 0;
      udp_val.gaxis_motion_mode        = 0;
      udp_val.gaxis_home_1             = 0;
      udp_val.gaxis_home_2             = 0;
      udp_val.gaxis_homing             = 0;
      udp_val.gaxis_finding_edge       = 0;
      udp_val.gaxis_pa_motion          = 0;
      udp_val.gaxis_pr_motion          = 1;
      udp_val.gaxis_moving             = 0;
      udp_val.gaxis_sm_jumper          = 1;
      udp_val.gaxis_state_home         = 0;
      udp_val.gaxis_state_reverse      = 0;
      udp_val.gaxis_state_forward      = 1;
      udp_val.gaxis_state_latch        = 0;
      udp_val.gaxis_latch_occurred     = 0;
      udp_val.gaxis_stop_code          = 1;
      udp_val.gaxis_reference_position = 5;
      udp_val.gaxis_motor_position     = 0;
      udp_val.gaxis_position_error     = 0;
      udp_val.gaxis_auxiliary_position = 5;
      udp_val.gaxis_velocity           = 0;
      udp_val.gaxis_torque             = 0;
      udp_val.gaxis_analog_in          = 14880;
      /* haxis */
      udp_val.haxis_motor_off          = 0;
      udp_val.haxis_error_armed        = 0;
      udp_val.haxis_latch_armed        = 1;
      udp_val.haxis_decelerating       = 1;
      udp_val.haxis_stopping           = 1;
      udp_val.haxis_slewing            = 1;
      udp_val.haxis_contour_mode       = 0;
      udp_val.haxis_negative_direction = 0;
      udp_val.haxis_motion_mode        = 0;
      udp_val.haxis_home_1             = 0;
      udp_val.haxis_home_2             = 0;
      udp_val.haxis_homing             = 0;
      udp_val.haxis_finding_edge       = 0;
      udp_val.haxis_pa_motion          = 0;
      udp_val.haxis_pr_motion          = 1;
      udp_val.haxis_moving             = 0;
      udp_val.haxis_sm_jumper          = 1;
      udp_val.haxis_state_home         = 0;
      udp_val.haxis_state_reverse      = 1;
      udp_val.haxis_state_forward      = 1;
      udp_val.haxis_state_latch        = 1;
      udp_val.haxis_latch_occurred     = 0;
      udp_val.haxis_stop_code          = 4;
      udp_val.haxis_reference_position = 13676;
      udp_val.haxis_motor_position     = 0;
      udp_val.haxis_position_error     = 0;
      udp_val.haxis_auxiliary_position = 13676;
      udp_val.haxis_velocity           = 0;
      udp_val.haxis_torque             = 0;
      udp_val.haxis_analog_in          = 15120;
      /* miscellaneous */
      udp_val.a_encoder = -826;
      udp_val.b_encoder = -902;
      udp_val.c_encoder = -144;
      udp_val.a_position = 0.146;
      udp_val.b_position = 0.171;
      udp_val.c_position = 0.171;

    } else {

      (void) GVersion(tcp_val.software, sizeof(tcp_val.software));
      (void) GInfo(gfd, tcp_val.hardware, sizeof(tcp_val.hardware));

      for (int x=0; x<MEM_CMD_NELMS; x++) {

        /* get command */
        pr = (char *)MemCmnds[x].name;

        /* send command */
        (void) memset((void *)buffer, '\0', sizeof(buffer));
        if ((gstat=GCommand(gfd, pr, buffer, sizeof(buffer), 0)) != G_NO_ERROR) {

          /* report error */
          replace_word(buffer, sizeof(buffer), "\n", " ");
          replace_word(buffer, sizeof(buffer), "\r", " ");
          replace_word(buffer, sizeof(buffer), ":", "");
          (void) fprintf(stderr, "%s <ERROR> failed executing '%s', gstat=%d\n", _NAME_, pr, gstat);
          (void) fflush(stderr);

          tcp_val.gstatus += gstat;
          (void) memset(buffer, '\0', sizeof(buffer));
          if ((gstat=GCommand(gfd, "TC 1;", buffer, sizeof(buffer), 0)) == G_NO_ERROR) {
            chomp(buffer, "\n");
            chomp(buffer, "\r");
            (void) fprintf(stderr, "%s <INFO> executed 'TC 1;', buffer='%s', gstat=%d\n", _NAME_, buffer, gstat);
            (void) fflush(stderr);
          } else {
            tcp_val.gstatus += gstat;
            (void) fprintf(stderr, "%s <ERROR> failed executing 'TC 1;', gstat=%d\n", _NAME_, gstat);
            (void) fflush(stderr);
          }

        /* command executed ok */
        } else {

          /* check status */
          if (strncasecmp(buffer, "?", 1)==0 || strlen(buffer)<=1) {
            (void) fprintf(stderr, "%s <INFO> command '%s' rejected, gstat=%d\n", _NAME_, pr, gstat);
            (void) fflush(stderr);

          } else {

            /* {"FILTVALS[0]=?;",     "filter number in slot 0",            }, */
            if (strncasecmp(pr, "FILTVALS[0]=?;", strlen("FILTVALS[0]=?;")) == 0) {
	      tcp_val.gstatus += gstat;
              replace_word(buffer, sizeof(buffer), "\n", " ");
              replace_word(buffer, sizeof(buffer), "\r", " ");
              replace_word(buffer, sizeof(buffer), ":", "");
              decode_float(buffer, &tcp_val.filtvals[0]);
              (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, buffer, tcp_val.filtvals[0], gstat);
              (void) fflush(stdout);

            /* {"FILTVALS[1]=?;",     "filter number in slot 1",            }, */
            } else if (strncasecmp(pr, "FILTVALS[1]=?;", strlen("FILTVALS[1]=?;")) == 0) {
	      tcp_val.gstatus += gstat;
              replace_word(buffer, sizeof(buffer), "\n", " ");
              replace_word(buffer, sizeof(buffer), "\r", " ");
              replace_word(buffer, sizeof(buffer), ":", "");
              decode_float(buffer, &tcp_val.filtvals[1]);
              (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, buffer, tcp_val.filtvals[1], gstat);
              (void) fflush(stdout);

            /* {"FILTVALS[2]=?;",     "filter number in slot 2",            }, */
            } else if (strncasecmp(pr, "FILTVALS[2]=?;", strlen("FILTVALS[2]=?;")) == 0) {
	      tcp_val.gstatus += gstat;
              replace_word(buffer, sizeof(buffer), "\n", " ");
              replace_word(buffer, sizeof(buffer), "\r", " ");
              replace_word(buffer, sizeof(buffer), ":", "");
              decode_float(buffer, &tcp_val.filtvals[2]);
              (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, buffer, tcp_val.filtvals[2], gstat);
              (void) fflush(stdout);

            /* {"FILTVALS[3]=?;",     "filter number in slot 3",            }, */
            } else if (strncasecmp(pr, "FILTVALS[3]=?;", strlen("FILTVALS[3]=?;")) == 0) {
	      tcp_val.gstatus += gstat;
              replace_word(buffer, sizeof(buffer), "\n", " ");
              replace_word(buffer, sizeof(buffer), "\r", " ");
              replace_word(buffer, sizeof(buffer), ":", "");
              decode_float(buffer, &tcp_val.filtvals[3]);
              (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, buffer, tcp_val.filtvals[3], gstat);
              (void) fflush(stdout);

            /* {"FILTVALS[4]=?;",     "filter number in slot 4",            }, */
            } else if (strncasecmp(pr, "FILTVALS[4]=?;", strlen("FILTVALS[4]=?;")) == 0) {
	      tcp_val.gstatus += gstat;
              replace_word(buffer, sizeof(buffer), "\n", " ");
              replace_word(buffer, sizeof(buffer), "\r", " ");
              replace_word(buffer, sizeof(buffer), ":", "");
              decode_float(buffer, &tcp_val.filtvals[4]);
              (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, buffer, tcp_val.filtvals[4], gstat);
              (void) fflush(stdout);

            /* {"FILTVALS[5]=?;",     "filter number in slot 5",            }, */
            } else if (strncasecmp(pr, "FILTVALS[5]=?;", strlen("FILTVALS[5]=?;")) == 0) {
	      tcp_val.gstatus += gstat;
              replace_word(buffer, sizeof(buffer), "\n", " ");
              replace_word(buffer, sizeof(buffer), "\r", " ");
              replace_word(buffer, sizeof(buffer), ":", "");
              decode_float(buffer, &tcp_val.filtvals[5]);
              (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, buffer, tcp_val.filtvals[5], gstat);
              (void) fflush(stdout);

            /* {"LV;",                "list variable",                      }, */
            } else if (strncasecmp(pr, "LV;", strlen("LV;")) == 0) {
	      tcp_val.gstatus += gstat;
              replace_word(buffer, sizeof(buffer), "\n", " ");
              replace_word(buffer, sizeof(buffer), "\r", " ");
              replace_word(buffer, sizeof(buffer), ":", "");
              replace_word(buffer, sizeof(buffer), "  ", "\\");
              tok = strtok(buffer, "\\");
              while (tok != (char *)NULL) {
                if ((sp=strchr(tok, '=')) != (char *)NULL) {
                  ep = sp;
                  fval = NAN;
                  decode_float(++ep, &fval);
                  *(++sp) = '\0';
                  (void) memset(lv_name, '\0', sizeof(lv_name));
                  (void) memmove(lv_name, tok, strlen(tok));
                  if (strncasecmp(lv_name, "DISTA=", strlen("DISTA=")) == 0) {
                    tcp_val.lv.dista = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.dista, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "DISTALL=", strlen("DISTALL=")) == 0) {
                    tcp_val.lv.distall = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.distall, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "DISTB=", strlen("DISTB=")) == 0) {
                    tcp_val.lv.distb = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.distb, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "DISTC=", strlen("DISTC=")) == 0) {
                    tcp_val.lv.distc = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.distc, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "DISTGCAM=", strlen("DISTGCAM=")) == 0) {
                    tcp_val.lv.distgcam = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.distgcam, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "ERRFILT=", strlen("ERRFILT=")) == 0) {
                    tcp_val.lv.errfilt = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.errfilt, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTBIT1=", strlen("FILTBIT1=")) == 0) {
                    tcp_val.lv.filtbit1 = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filtbit1, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTBIT2=", strlen("FILTBIT2=")) == 0) {
                    tcp_val.lv.filtbit2 = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filtbit2, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTBIT3=", strlen("FILTBIT3=")) == 0) {
                    tcp_val.lv.filtbit3 = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filtbit3, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTBIT4=", strlen("FILTBIT4=")) == 0) {
                    tcp_val.lv.filtbit4 = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filtbit4, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTBIT5=", strlen("FILTBIT5=")) == 0) {
                    tcp_val.lv.filtbit5 = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filtbit5, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTBIT6=", strlen("FILTBIT6=")) == 0) {
                    tcp_val.lv.filtbit6 = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filtbit6, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTBIT7=", strlen("FILTBIT7=")) == 0) {
                    tcp_val.lv.filtbit7 = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filtbit7, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTBIT8=", strlen("FILTBIT8=")) == 0) {
                    tcp_val.lv.filtbit8 = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filtbit8, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTHIGH=", strlen("FILTHIGH=")) == 0) {
                    tcp_val.lv.filthigh = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filthigh, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTISIN=", strlen("FILTISIN=")) == 0) {
                    tcp_val.lv.filtisin = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filtisin, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTLOW=", strlen("FILTLOW=")) == 0) {
                    tcp_val.lv.filtlow = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filtlow, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTNUM=", strlen("FILTNUM=")) == 0) {
                    tcp_val.lv.filtnum = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filtnum, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTRAC=", strlen("FILTRAC=")) == 0) {
                    tcp_val.lv.filtrac = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filtrac, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTRBL=", strlen("FILTRBL=")) == 0) {
                    tcp_val.lv.filtrbl = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filtrbl, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTRDC=", strlen("FILTRDC=")) == 0) {
                    tcp_val.lv.filtrdc = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filtrdc, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTRFL=", strlen("FILTRFL=")) == 0) {
                    tcp_val.lv.filtrfl = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filtrfl, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTRSP=", strlen("FILTRSP=")) == 0) {
                    tcp_val.lv.filtrsp = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filtrsp, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTTAC=", strlen("FILTTAC=")) == 0) {
                    tcp_val.lv.filttac = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filttac, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTTDC=", strlen("FILTTDC=")) == 0) {
                    tcp_val.lv.filttdc = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filttdc, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTTDIS=", strlen("FILTTDIS=")) == 0) {
                    tcp_val.lv.filttdis = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filttdis, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTTNUD=", strlen("FILTTNUD=")) == 0) {
                    tcp_val.lv.filttnud = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filttnud, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTTSC=", strlen("FILTTSC=")) == 0) {
                    tcp_val.lv.filttsc = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filttsc, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTTSP=", strlen("FILTTSP=")) == 0) {
                    tcp_val.lv.filttsp = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filttsp, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FILTVAL=", strlen("FILTVAL=")) == 0) {
                    tcp_val.lv.filtval = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.filtval, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FNUM=", strlen("FNUM=")) == 0) {
                    tcp_val.lv.fnum = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.fnum, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FNUM_IN=", strlen("FNUM_IN=")) == 0) {
                    tcp_val.lv.fnum_in = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.fnum_in, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FOCAC=", strlen("FOCAC=")) == 0) {
                    tcp_val.lv.focac = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.focac, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FOCBL=", strlen("FOCBL=")) == 0) {
                    tcp_val.lv.focbl = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.focbl, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FOCDC=", strlen("FOCDC=")) == 0) {
                    tcp_val.lv.focdc = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.focdc, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FOCFL=", strlen("FOCFL=")) == 0) {
                    tcp_val.lv.focfl = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.focfl, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FOCREFA=", strlen("FOCREFA=")) == 0) {
                    tcp_val.lv.focrefa = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.focrefa, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FOCREFB=", strlen("FOCREFB=")) == 0) {
                    tcp_val.lv.focrefb = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.focrefb, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FOCREFC=", strlen("FOCREFC=")) == 0) {
                    tcp_val.lv.focrefc = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.focrefc, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FOCRFSET=", strlen("FOCRFSET=")) == 0) {
                    tcp_val.lv.focrfset = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.focrfset, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "FOCSP=", strlen("FOCSP=")) == 0) {
                    tcp_val.lv.focsp = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.focsp, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "GFAC=", strlen("GFAC=")) == 0) {
                    tcp_val.lv.gfac = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.gfac, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "GFCENT=", strlen("GFCENT=")) == 0) {
                    tcp_val.lv.gfcent = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.gfcent, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "GFDC=", strlen("GFDC=")) == 0) {
                    tcp_val.lv.gfdc = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.gfdc, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "GFILTAC=", strlen("GFILTAC=")) == 0) {
                    tcp_val.lv.gfiltac = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.gfiltac, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "GFILTDC=", strlen("GFILTDC=")) == 0) {
                    tcp_val.lv.gfiltdc = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.gfiltdc, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "GFILTN=", strlen("GFILTN=")) == 0) {
                    tcp_val.lv.gfiltn = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.gfiltn, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "GFILTQ=", strlen("GFILTQ=")) == 0) {
                    tcp_val.lv.gfiltq = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.gfiltq, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "GFILTREQ=", strlen("GFILTREQ=")) == 0) {
                    tcp_val.lv.gfiltreq = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.gfiltreq, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "GFILTSP=", strlen("GFILTSP=")) == 0) {
                    tcp_val.lv.gfiltsp = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.gfiltsp, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "GFSP=", strlen("GFSP=")) == 0) {
                    tcp_val.lv.gfsp = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.gfsp, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "GIFLTN=", strlen("GIFLTN=")) == 0) {
                    tcp_val.lv.gifltn = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.gfiltn, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "INITFILT=", strlen("INITFILT=")) == 0) {
                    tcp_val.lv.initfilt = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.initfilt, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "NMOVES=", strlen("NMOVES=")) == 0) {
                    tcp_val.lv.nmoves = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.nmoves, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "NROT=", strlen("NROT=")) == 0) {
                    tcp_val.lv.nrot = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.nrot, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "REQFILT=", strlen("REQFILT=")) == 0) {
                    tcp_val.lv.reqfilt = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.reqfilt, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "SNUM=", strlen("SNUM=")) == 0) {
                    tcp_val.lv.snum = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.snum, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "SNUM_IN=", strlen("SNUM_IN=")) == 0) {
                    tcp_val.lv.snum_in = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.snum_in, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "TOTFOCA=", strlen("TOTFOCA=")) == 0) {
                    tcp_val.lv.totfoca = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.totfoca, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "TOTFOCB=", strlen("TOTFOCB=")) == 0) {
                    tcp_val.lv.totfocb = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.totfocb, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "TOTFOCC=", strlen("TOTFOCC=")) == 0) {
                    tcp_val.lv.totfocc = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.totfocc, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "VECAC=", strlen("VECAC=")) == 0) {
                    tcp_val.lv.vecac = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.vecac, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "VECDC=", strlen("VECDC=")) == 0) {
                    tcp_val.lv.vecdc = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.vecdc, gstat);
                    (void) fflush(stdout);
                  } else if (strncasecmp(lv_name, "VECSP=", strlen("VECSP=")) == 0) {
                    tcp_val.lv.vecsp = fval;
                    (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%.2f, gstat=%d\n", _NAME_, pr, lv_name, tcp_val.lv.vecsp, gstat);
                    (void) fflush(stdout);
                  }
                }
                tok = strtok(NULL, "\\");
              }

            /*   {"TB;",                "tell status byte",                   }, */
            } else if (strncasecmp(pr, "TB;", strlen("TB;")) == 0) {
	      tcp_val.gstatus += gstat;
              replace_word(buffer, sizeof(buffer), "\n", " ");
              replace_word(buffer, sizeof(buffer), "\r", " ");
              replace_word(buffer, sizeof(buffer), ":", "");
              decode_integer(buffer, &tcp_val.status);
              (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', value=%d, gstat=%d\n", _NAME_, pr, buffer, tcp_val.status, gstat);
              (void) fflush(stdout);

            /*   {"TP;",                "tell (motor) position",              }, */
            } else if (strncasecmp(pr, "TP;", strlen("TP;")) == 0) {
	      tcp_val.gstatus += gstat;
              replace_word(buffer, sizeof(buffer), "\n", " ");
              replace_word(buffer, sizeof(buffer), "\r", " ");
              replace_word(buffer, sizeof(buffer), ":", "");
              decode_fvals(buffer, tcp_val.position, BOK_AXES, ',');
              (void) fprintf(stdout, "%s <INFO> executed '%s', buffer='%s', gstat=%d\n", _NAME_, pr, buffer, gstat);
              (void) fflush(stdout);

            /* {"QR;",                "tell data record",                   } */
            } else if (strncasecmp(pr, "QR;", strlen("QR;")) == 0) {
              tcp_val.gstatus += gstat;
              chomp(buffer, "\n");
              chomp(buffer, "\n");
              (void) memmove((void *)&qr, buffer, sizeof(qr));
              /* header */
              udp_val.aaxis_enabled = (IS_BIT_SET(qr.header.header.uc[0], 0) ? 1 : 0);
              udp_val.baxis_enabled = (IS_BIT_SET(qr.header.header.uc[0], 1) ? 1 : 0);
              udp_val.caxis_enabled = (IS_BIT_SET(qr.header.header.uc[0], 2) ? 1 : 0);
              udp_val.daxis_enabled = (IS_BIT_SET(qr.header.header.uc[0], 3) ? 1 : 0);
              udp_val.eaxis_enabled = (IS_BIT_SET(qr.header.header.uc[0], 4) ? 1 : 0);
              udp_val.faxis_enabled = (IS_BIT_SET(qr.header.header.uc[0], 5) ? 1 : 0);
              udp_val.gaxis_enabled = (IS_BIT_SET(qr.header.header.uc[0], 6) ? 1 : 0);
              udp_val.haxis_enabled = (IS_BIT_SET(qr.header.header.uc[0], 7) ? 1 : 0);
              udp_val.iaxis_enabled = (IS_BIT_SET(qr.header.header.uc[1], 0) ? 1 : 0);
              udp_val.saxis_enabled = (IS_BIT_SET(qr.header.header.uc[1], 1) ? 1 : 0);
              udp_val.taxis_enabled = (IS_BIT_SET(qr.header.header.uc[1], 2) ? 1 : 0);
              udp_val.header_length = (int)qr.header.header.us[1];
              /* iaxis */
              udp_val.iaxis_sample_number = (int)qr.iaxis.sample_number.us;
              for (int i=0; i<BOK_IAXIS_INPUTS; i++ ) {
                udp_val.iaxis_general_input[i]  = (int)qr.iaxis.general_input[i];
                udp_val.iaxis_general_output[i] = (int)qr.iaxis.general_output[i];
              }
              udp_val.iaxis_error_code      = (int)qr.iaxis.error_code;
              udp_val.iaxis_echo_on         = (IS_BIT_SET(qr.iaxis.general_status, 0) ? 1 : 0);
              udp_val.iaxis_trace_on        = (IS_BIT_SET(qr.iaxis.general_status, 1) ? 1 : 0);
              udp_val.iaxis_awaiting_input  = (IS_BIT_SET(qr.iaxis.general_status, 2) ? 1 : 0);
              udp_val.iaxis_program_running = (IS_BIT_SET(qr.iaxis.general_status, 7) ? 1 : 0);
              /* saxis */
              udp_val.saxis_segment_count     = (int)qr.saxis.segment_count.us;
              udp_val.saxis_decelerating      = (IS_BIT_SET(qr.saxis.move_status.uc[0], 3) ? 1 : 0);
              udp_val.saxis_stopping          = (IS_BIT_SET(qr.saxis.move_status.uc[0], 4) ? 1 : 0);
              udp_val.saxis_slewing           = (IS_BIT_SET(qr.saxis.move_status.uc[0], 5) ? 1 : 0);
              udp_val.saxis_moving            = (IS_BIT_SET(qr.saxis.move_status.uc[1], 7) ? 1 : 0);
              udp_val.saxis_distance_traveled = qr.saxis.distance_traveled.i;
              /* taxis */
              udp_val.taxis_segment_count     = (int)qr.taxis.segment_count.us;
              udp_val.taxis_decelerating      = (IS_BIT_SET(qr.taxis.move_status.uc[0], 3) ? 1 : 0);
              udp_val.taxis_stopping          = (IS_BIT_SET(qr.taxis.move_status.uc[0], 4) ? 1 : 0);
              udp_val.taxis_slewing           = (IS_BIT_SET(qr.taxis.move_status.uc[0], 5) ? 1 : 0);
              udp_val.taxis_moving            = (IS_BIT_SET(qr.taxis.move_status.uc[1], 7) ? 1 : 0);
              udp_val.taxis_distance_traveled = qr.taxis.distance_traveled.i;
              /* aaxis */
              udp_val.aaxis_motor_off          = (IS_BIT_SET(qr.aaxis.axis_status.uc[0], 0) ? 1 : 0);
              udp_val.aaxis_error_armed        = (IS_BIT_SET(qr.aaxis.axis_status.uc[0], 1) ? 1 : 0);
              udp_val.aaxis_latch_armed        = (IS_BIT_SET(qr.aaxis.axis_status.uc[0], 2) ? 1 : 0);
              udp_val.aaxis_decelerating       = (IS_BIT_SET(qr.aaxis.axis_status.uc[0], 3) ? 1 : 0);
              udp_val.aaxis_stopping           = (IS_BIT_SET(qr.aaxis.axis_status.uc[0], 4) ? 1 : 0);
              udp_val.aaxis_slewing            = (IS_BIT_SET(qr.aaxis.axis_status.uc[0], 5) ? 1 : 0);
              udp_val.aaxis_contour_mode       = (IS_BIT_SET(qr.aaxis.axis_status.uc[0], 6) ? 1 : 0);
              udp_val.aaxis_negative_direction = (IS_BIT_SET(qr.aaxis.axis_status.uc[0], 7) ? 1 : 0);
              udp_val.aaxis_motion_mode        = (IS_BIT_SET(qr.aaxis.axis_status.uc[1], 0) ? 1 : 0);
              udp_val.aaxis_home_1             = (IS_BIT_SET(qr.aaxis.axis_status.uc[1], 1) ? 1 : 0);
              udp_val.aaxis_home_2             = (IS_BIT_SET(qr.aaxis.axis_status.uc[1], 2) ? 1 : 0);
              udp_val.aaxis_homing             = (IS_BIT_SET(qr.aaxis.axis_status.uc[1], 3) ? 1 : 0);
              udp_val.aaxis_finding_edge       = (IS_BIT_SET(qr.aaxis.axis_status.uc[1], 4) ? 1 : 0);
              udp_val.aaxis_pa_motion          = (IS_BIT_SET(qr.aaxis.axis_status.uc[1], 5) ? 1 : 0);
              udp_val.aaxis_pr_motion          = (IS_BIT_SET(qr.aaxis.axis_status.uc[1], 6) ? 1 : 0);
              udp_val.aaxis_moving             = (IS_BIT_SET(qr.aaxis.axis_status.uc[1], 7) ? 1 : 0);
              udp_val.aaxis_sm_jumper          = (IS_BIT_SET(qr.aaxis.axis_switches, 0) ? 1 : 0);
              udp_val.aaxis_state_home         = (IS_BIT_SET(qr.aaxis.axis_switches, 1) ? 1 : 0);
              udp_val.aaxis_state_reverse      = (IS_BIT_SET(qr.aaxis.axis_switches, 2) ? 1 : 0);
              udp_val.aaxis_state_forward      = (IS_BIT_SET(qr.aaxis.axis_switches, 3) ? 1 : 0);
              udp_val.aaxis_state_latch        = (IS_BIT_SET(qr.aaxis.axis_switches, 6) ? 1 : 0);
              udp_val.aaxis_latch_occurred     = (IS_BIT_SET(qr.aaxis.axis_switches, 7) ? 1 : 0);
              udp_val.aaxis_stop_code          = (int)qr.aaxis.stop_code;
              udp_val.aaxis_reference_position = qr.aaxis.reference_position.i;
              udp_val.aaxis_motor_position     = qr.aaxis.motor_position.i;
              udp_val.aaxis_position_error     = qr.aaxis.position_error.i;
              udp_val.aaxis_auxiliary_position = qr.aaxis.auxiliary_position.i;
              udp_val.aaxis_velocity           = qr.aaxis.velocity.i;
              udp_val.aaxis_torque             = qr.aaxis.torque.s;
              udp_val.aaxis_analog_in          = qr.aaxis.analog.s;
              /* baxis */
              udp_val.baxis_motor_off          = (IS_BIT_SET(qr.baxis.axis_status.uc[0], 0) ? 1 : 0);
              udp_val.baxis_error_armed        = (IS_BIT_SET(qr.baxis.axis_status.uc[0], 1) ? 1 : 0);
              udp_val.baxis_latch_armed        = (IS_BIT_SET(qr.baxis.axis_status.uc[0], 2) ? 1 : 0);
              udp_val.baxis_decelerating       = (IS_BIT_SET(qr.baxis.axis_status.uc[0], 3) ? 1 : 0);
              udp_val.baxis_stopping           = (IS_BIT_SET(qr.baxis.axis_status.uc[0], 4) ? 1 : 0);
              udp_val.baxis_slewing            = (IS_BIT_SET(qr.baxis.axis_status.uc[0], 5) ? 1 : 0);
              udp_val.baxis_contour_mode       = (IS_BIT_SET(qr.baxis.axis_status.uc[0], 6) ? 1 : 0);
              udp_val.baxis_negative_direction = (IS_BIT_SET(qr.baxis.axis_status.uc[0], 7) ? 1 : 0);
              udp_val.baxis_motion_mode        = (IS_BIT_SET(qr.baxis.axis_status.uc[1], 0) ? 1 : 0);
              udp_val.baxis_home_1             = (IS_BIT_SET(qr.baxis.axis_status.uc[1], 1) ? 1 : 0);
              udp_val.baxis_home_2             = (IS_BIT_SET(qr.baxis.axis_status.uc[1], 2) ? 1 : 0);
              udp_val.baxis_homing             = (IS_BIT_SET(qr.baxis.axis_status.uc[1], 3) ? 1 : 0);
              udp_val.baxis_finding_edge       = (IS_BIT_SET(qr.baxis.axis_status.uc[1], 4) ? 1 : 0);
              udp_val.baxis_pa_motion          = (IS_BIT_SET(qr.baxis.axis_status.uc[1], 5) ? 1 : 0);
              udp_val.baxis_pr_motion          = (IS_BIT_SET(qr.baxis.axis_status.uc[1], 6) ? 1 : 0);
              udp_val.baxis_moving             = (IS_BIT_SET(qr.baxis.axis_status.uc[1], 7) ? 1 : 0);
              udp_val.baxis_sm_jumper          = (IS_BIT_SET(qr.baxis.axis_switches, 0) ? 1 : 0);
              udp_val.baxis_state_home         = (IS_BIT_SET(qr.baxis.axis_switches, 1) ? 1 : 0);
              udp_val.baxis_state_reverse      = (IS_BIT_SET(qr.baxis.axis_switches, 2) ? 1 : 0);
              udp_val.baxis_state_forward      = (IS_BIT_SET(qr.baxis.axis_switches, 3) ? 1 : 0);
              udp_val.baxis_state_latch        = (IS_BIT_SET(qr.baxis.axis_switches, 6) ? 1 : 0);
              udp_val.baxis_latch_occurred     = (IS_BIT_SET(qr.baxis.axis_switches, 7) ? 1 : 0);
              udp_val.baxis_stop_code          = (int)qr.baxis.stop_code;
              udp_val.baxis_reference_position = qr.baxis.reference_position.i;
              udp_val.baxis_motor_position     = qr.baxis.motor_position.i;
              udp_val.baxis_position_error     = qr.baxis.position_error.i;
              udp_val.baxis_auxiliary_position = qr.baxis.auxiliary_position.i;
              udp_val.baxis_velocity           = qr.baxis.velocity.i;
              udp_val.baxis_torque             = qr.baxis.torque.s;
              udp_val.baxis_analog_in          = qr.baxis.analog.s;
              /* caxis */
              udp_val.caxis_motor_off          = (IS_BIT_SET(qr.caxis.axis_status.uc[0], 0) ? 1 : 0);
              udp_val.caxis_error_armed        = (IS_BIT_SET(qr.caxis.axis_status.uc[0], 1) ? 1 : 0);
              udp_val.caxis_latch_armed        = (IS_BIT_SET(qr.caxis.axis_status.uc[0], 2) ? 1 : 0);
              udp_val.caxis_decelerating       = (IS_BIT_SET(qr.caxis.axis_status.uc[0], 3) ? 1 : 0);
              udp_val.caxis_stopping           = (IS_BIT_SET(qr.caxis.axis_status.uc[0], 4) ? 1 : 0);
              udp_val.caxis_slewing            = (IS_BIT_SET(qr.caxis.axis_status.uc[0], 5) ? 1 : 0);
              udp_val.caxis_contour_mode       = (IS_BIT_SET(qr.caxis.axis_status.uc[0], 6) ? 1 : 0);
              udp_val.caxis_negative_direction = (IS_BIT_SET(qr.caxis.axis_status.uc[0], 7) ? 1 : 0);
              udp_val.caxis_motion_mode        = (IS_BIT_SET(qr.caxis.axis_status.uc[1], 0) ? 1 : 0);
              udp_val.caxis_home_1             = (IS_BIT_SET(qr.caxis.axis_status.uc[1], 1) ? 1 : 0);
              udp_val.caxis_home_2             = (IS_BIT_SET(qr.caxis.axis_status.uc[1], 2) ? 1 : 0);
              udp_val.caxis_homing             = (IS_BIT_SET(qr.caxis.axis_status.uc[1], 3) ? 1 : 0);
              udp_val.caxis_finding_edge       = (IS_BIT_SET(qr.caxis.axis_status.uc[1], 4) ? 1 : 0);
              udp_val.caxis_pa_motion          = (IS_BIT_SET(qr.caxis.axis_status.uc[1], 5) ? 1 : 0);
              udp_val.caxis_pr_motion          = (IS_BIT_SET(qr.caxis.axis_status.uc[1], 6) ? 1 : 0);
              udp_val.caxis_moving             = (IS_BIT_SET(qr.caxis.axis_status.uc[1], 7) ? 1 : 0);
              udp_val.caxis_sm_jumper          = (IS_BIT_SET(qr.caxis.axis_switches, 0) ? 1 : 0);
              udp_val.caxis_state_home         = (IS_BIT_SET(qr.caxis.axis_switches, 1) ? 1 : 0);
              udp_val.caxis_state_reverse      = (IS_BIT_SET(qr.caxis.axis_switches, 2) ? 1 : 0);
              udp_val.caxis_state_forward      = (IS_BIT_SET(qr.caxis.axis_switches, 3) ? 1 : 0);
              udp_val.caxis_state_latch        = (IS_BIT_SET(qr.caxis.axis_switches, 6) ? 1 : 0);
              udp_val.caxis_latch_occurred     = (IS_BIT_SET(qr.caxis.axis_switches, 7) ? 1 : 0);
              udp_val.caxis_stop_code          = (int)qr.caxis.stop_code;
              udp_val.caxis_reference_position = qr.caxis.reference_position.i;
              udp_val.caxis_motor_position     = qr.caxis.motor_position.i;
              udp_val.caxis_position_error     = qr.caxis.position_error.i;
              udp_val.caxis_auxiliary_position = qr.caxis.auxiliary_position.i;
              udp_val.caxis_velocity           = qr.caxis.velocity.i;
              udp_val.caxis_torque             = qr.caxis.torque.s;
              udp_val.caxis_analog_in          = qr.caxis.analog.s;
              /* daxis */
              udp_val.daxis_motor_off          = (IS_BIT_SET(qr.daxis.axis_status.uc[0], 0) ? 1 : 0);
              udp_val.daxis_error_armed        = (IS_BIT_SET(qr.daxis.axis_status.uc[0], 1) ? 1 : 0);
              udp_val.daxis_latch_armed        = (IS_BIT_SET(qr.daxis.axis_status.uc[0], 2) ? 1 : 0);
              udp_val.daxis_decelerating       = (IS_BIT_SET(qr.daxis.axis_status.uc[0], 3) ? 1 : 0);
              udp_val.daxis_stopping           = (IS_BIT_SET(qr.daxis.axis_status.uc[0], 4) ? 1 : 0);
              udp_val.daxis_slewing            = (IS_BIT_SET(qr.daxis.axis_status.uc[0], 5) ? 1 : 0);
              udp_val.daxis_contour_mode       = (IS_BIT_SET(qr.daxis.axis_status.uc[0], 6) ? 1 : 0);
              udp_val.daxis_negative_direction = (IS_BIT_SET(qr.daxis.axis_status.uc[0], 7) ? 1 : 0);
              udp_val.daxis_motion_mode        = (IS_BIT_SET(qr.daxis.axis_status.uc[1], 0) ? 1 : 0);
              udp_val.daxis_home_1             = (IS_BIT_SET(qr.daxis.axis_status.uc[1], 1) ? 1 : 0);
              udp_val.daxis_home_2             = (IS_BIT_SET(qr.daxis.axis_status.uc[1], 2) ? 1 : 0);
              udp_val.daxis_homing             = (IS_BIT_SET(qr.daxis.axis_status.uc[1], 3) ? 1 : 0);
              udp_val.daxis_finding_edge       = (IS_BIT_SET(qr.daxis.axis_status.uc[1], 4) ? 1 : 0);
              udp_val.daxis_pa_motion          = (IS_BIT_SET(qr.daxis.axis_status.uc[1], 5) ? 1 : 0);
              udp_val.daxis_pr_motion          = (IS_BIT_SET(qr.daxis.axis_status.uc[1], 6) ? 1 : 0);
              udp_val.daxis_moving             = (IS_BIT_SET(qr.daxis.axis_status.uc[1], 7) ? 1 : 0);
              udp_val.daxis_sm_jumper          = (IS_BIT_SET(qr.daxis.axis_switches, 0) ? 1 : 0);
              udp_val.daxis_state_home         = (IS_BIT_SET(qr.daxis.axis_switches, 1) ? 1 : 0);
              udp_val.daxis_state_reverse      = (IS_BIT_SET(qr.daxis.axis_switches, 2) ? 1 : 0);
              udp_val.daxis_state_forward      = (IS_BIT_SET(qr.daxis.axis_switches, 3) ? 1 : 0);
              udp_val.daxis_state_latch        = (IS_BIT_SET(qr.daxis.axis_switches, 6) ? 1 : 0);
              udp_val.daxis_latch_occurred     = (IS_BIT_SET(qr.daxis.axis_switches, 7) ? 1 : 0);
              udp_val.daxis_stop_code          = (int)qr.daxis.stop_code;
              udp_val.daxis_reference_position = qr.daxis.reference_position.i;
              udp_val.daxis_motor_position     = qr.daxis.motor_position.i;
              udp_val.daxis_position_error     = qr.daxis.position_error.i;
              udp_val.daxis_auxiliary_position = qr.daxis.auxiliary_position.i;
              udp_val.daxis_velocity           = qr.daxis.velocity.i;
              udp_val.daxis_torque             = qr.daxis.torque.s;
              udp_val.daxis_analog_in          = qr.daxis.analog.s;
              /* eaxis */
              udp_val.eaxis_motor_off          = (IS_BIT_SET(qr.eaxis.axis_status.uc[0], 0) ? 1 : 0);
              udp_val.eaxis_error_armed        = (IS_BIT_SET(qr.eaxis.axis_status.uc[0], 1) ? 1 : 0);
              udp_val.eaxis_latch_armed        = (IS_BIT_SET(qr.eaxis.axis_status.uc[0], 2) ? 1 : 0);
              udp_val.eaxis_decelerating       = (IS_BIT_SET(qr.eaxis.axis_status.uc[0], 3) ? 1 : 0);
              udp_val.eaxis_stopping           = (IS_BIT_SET(qr.eaxis.axis_status.uc[0], 4) ? 1 : 0);
              udp_val.eaxis_slewing            = (IS_BIT_SET(qr.eaxis.axis_status.uc[0], 5) ? 1 : 0);
              udp_val.eaxis_contour_mode       = (IS_BIT_SET(qr.eaxis.axis_status.uc[0], 6) ? 1 : 0);
              udp_val.eaxis_negative_direction = (IS_BIT_SET(qr.eaxis.axis_status.uc[0], 7) ? 1 : 0);
              udp_val.eaxis_motion_mode        = (IS_BIT_SET(qr.eaxis.axis_status.uc[1], 0) ? 1 : 0);
              udp_val.eaxis_home_1             = (IS_BIT_SET(qr.eaxis.axis_status.uc[1], 1) ? 1 : 0);
              udp_val.eaxis_home_2             = (IS_BIT_SET(qr.eaxis.axis_status.uc[1], 2) ? 1 : 0);
              udp_val.eaxis_homing             = (IS_BIT_SET(qr.eaxis.axis_status.uc[1], 3) ? 1 : 0);
              udp_val.eaxis_finding_edge       = (IS_BIT_SET(qr.eaxis.axis_status.uc[1], 4) ? 1 : 0);
              udp_val.eaxis_pa_motion          = (IS_BIT_SET(qr.eaxis.axis_status.uc[1], 5) ? 1 : 0);
              udp_val.eaxis_pr_motion          = (IS_BIT_SET(qr.eaxis.axis_status.uc[1], 6) ? 1 : 0);
              udp_val.eaxis_moving             = (IS_BIT_SET(qr.eaxis.axis_status.uc[1], 7) ? 1 : 0);
              udp_val.eaxis_sm_jumper          = (IS_BIT_SET(qr.eaxis.axis_switches, 0) ? 1 : 0);
              udp_val.eaxis_state_home         = (IS_BIT_SET(qr.eaxis.axis_switches, 1) ? 1 : 0);
              udp_val.eaxis_state_reverse      = (IS_BIT_SET(qr.eaxis.axis_switches, 2) ? 1 : 0);
              udp_val.eaxis_state_forward      = (IS_BIT_SET(qr.eaxis.axis_switches, 3) ? 1 : 0);
              udp_val.eaxis_state_latch        = (IS_BIT_SET(qr.eaxis.axis_switches, 6) ? 1 : 0);
              udp_val.eaxis_latch_occurred     = (IS_BIT_SET(qr.eaxis.axis_switches, 7) ? 1 : 0);
              udp_val.eaxis_stop_code          = (int)qr.eaxis.stop_code;
              udp_val.eaxis_reference_position = qr.eaxis.reference_position.i;
              udp_val.eaxis_motor_position     = qr.eaxis.motor_position.i;
              udp_val.eaxis_position_error     = qr.eaxis.position_error.i;
              udp_val.eaxis_auxiliary_position = qr.eaxis.auxiliary_position.i;
              udp_val.eaxis_velocity           = qr.eaxis.velocity.i;
              udp_val.eaxis_torque             = qr.eaxis.torque.s;
              udp_val.eaxis_analog_in          = qr.eaxis.analog.s;
              /* faxis */
              udp_val.faxis_motor_off          = (IS_BIT_SET(qr.faxis.axis_status.uc[0], 0) ? 1 : 0);
              udp_val.faxis_error_armed        = (IS_BIT_SET(qr.faxis.axis_status.uc[0], 1) ? 1 : 0);
              udp_val.faxis_latch_armed        = (IS_BIT_SET(qr.faxis.axis_status.uc[0], 2) ? 1 : 0);
              udp_val.faxis_decelerating       = (IS_BIT_SET(qr.faxis.axis_status.uc[0], 3) ? 1 : 0);
              udp_val.faxis_stopping           = (IS_BIT_SET(qr.faxis.axis_status.uc[0], 4) ? 1 : 0);
              udp_val.faxis_slewing            = (IS_BIT_SET(qr.faxis.axis_status.uc[0], 5) ? 1 : 0);
              udp_val.faxis_contour_mode       = (IS_BIT_SET(qr.faxis.axis_status.uc[0], 6) ? 1 : 0);
              udp_val.faxis_negative_direction = (IS_BIT_SET(qr.faxis.axis_status.uc[0], 7) ? 1 : 0);
              udp_val.faxis_motion_mode        = (IS_BIT_SET(qr.faxis.axis_status.uc[1], 0) ? 1 : 0);
              udp_val.faxis_home_1             = (IS_BIT_SET(qr.faxis.axis_status.uc[1], 1) ? 1 : 0);
              udp_val.faxis_home_2             = (IS_BIT_SET(qr.faxis.axis_status.uc[1], 2) ? 1 : 0);
              udp_val.faxis_homing             = (IS_BIT_SET(qr.faxis.axis_status.uc[1], 3) ? 1 : 0);
              udp_val.faxis_finding_edge       = (IS_BIT_SET(qr.faxis.axis_status.uc[1], 4) ? 1 : 0);
              udp_val.faxis_pa_motion          = (IS_BIT_SET(qr.faxis.axis_status.uc[1], 5) ? 1 : 0);
              udp_val.faxis_pr_motion          = (IS_BIT_SET(qr.faxis.axis_status.uc[1], 6) ? 1 : 0);
              udp_val.faxis_moving             = (IS_BIT_SET(qr.faxis.axis_status.uc[1], 7) ? 1 : 0);
              udp_val.faxis_sm_jumper          = (IS_BIT_SET(qr.faxis.axis_switches, 0) ? 1 : 0);
              udp_val.faxis_state_home         = (IS_BIT_SET(qr.faxis.axis_switches, 1) ? 1 : 0);
              udp_val.faxis_state_reverse      = (IS_BIT_SET(qr.faxis.axis_switches, 2) ? 1 : 0);
              udp_val.faxis_state_forward      = (IS_BIT_SET(qr.faxis.axis_switches, 3) ? 1 : 0);
              udp_val.faxis_state_latch        = (IS_BIT_SET(qr.faxis.axis_switches, 6) ? 1 : 0);
              udp_val.faxis_latch_occurred     = (IS_BIT_SET(qr.faxis.axis_switches, 7) ? 1 : 0);
              udp_val.faxis_stop_code          = (int)qr.faxis.stop_code;
              udp_val.faxis_reference_position = qr.faxis.reference_position.i;
              udp_val.faxis_motor_position     = qr.faxis.motor_position.i;
              udp_val.faxis_position_error     = qr.faxis.position_error.i;
              udp_val.faxis_auxiliary_position = qr.faxis.auxiliary_position.i;
              udp_val.faxis_velocity           = qr.faxis.velocity.i;
              udp_val.faxis_torque             = qr.faxis.torque.s;
              udp_val.faxis_analog_in          = qr.faxis.analog.s;
              /* gaxis */
              udp_val.gaxis_motor_off          = (IS_BIT_SET(qr.gaxis.axis_status.uc[0], 0) ? 1 : 0);
              udp_val.gaxis_error_armed        = (IS_BIT_SET(qr.gaxis.axis_status.uc[0], 1) ? 1 : 0);
              udp_val.gaxis_latch_armed        = (IS_BIT_SET(qr.gaxis.axis_status.uc[0], 2) ? 1 : 0);
              udp_val.gaxis_decelerating       = (IS_BIT_SET(qr.gaxis.axis_status.uc[0], 3) ? 1 : 0);
              udp_val.gaxis_stopping           = (IS_BIT_SET(qr.gaxis.axis_status.uc[0], 4) ? 1 : 0);
              udp_val.gaxis_slewing            = (IS_BIT_SET(qr.gaxis.axis_status.uc[0], 5) ? 1 : 0);
              udp_val.gaxis_contour_mode       = (IS_BIT_SET(qr.gaxis.axis_status.uc[0], 6) ? 1 : 0);
              udp_val.gaxis_negative_direction = (IS_BIT_SET(qr.gaxis.axis_status.uc[0], 7) ? 1 : 0);
              udp_val.gaxis_motion_mode        = (IS_BIT_SET(qr.gaxis.axis_status.uc[1], 0) ? 1 : 0);
              udp_val.gaxis_home_1             = (IS_BIT_SET(qr.gaxis.axis_status.uc[1], 1) ? 1 : 0);
              udp_val.gaxis_home_2             = (IS_BIT_SET(qr.gaxis.axis_status.uc[1], 2) ? 1 : 0);
              udp_val.gaxis_homing             = (IS_BIT_SET(qr.gaxis.axis_status.uc[1], 3) ? 1 : 0);
              udp_val.gaxis_finding_edge       = (IS_BIT_SET(qr.gaxis.axis_status.uc[1], 4) ? 1 : 0);
              udp_val.gaxis_pa_motion          = (IS_BIT_SET(qr.gaxis.axis_status.uc[1], 5) ? 1 : 0);
              udp_val.gaxis_pr_motion          = (IS_BIT_SET(qr.gaxis.axis_status.uc[1], 6) ? 1 : 0);
              udp_val.gaxis_moving             = (IS_BIT_SET(qr.gaxis.axis_status.uc[1], 7) ? 1 : 0);
              udp_val.gaxis_sm_jumper          = (IS_BIT_SET(qr.gaxis.axis_switches, 0) ? 1 : 0);
              udp_val.gaxis_state_home         = (IS_BIT_SET(qr.gaxis.axis_switches, 1) ? 1 : 0);
              udp_val.gaxis_state_reverse      = (IS_BIT_SET(qr.gaxis.axis_switches, 2) ? 1 : 0);
              udp_val.gaxis_state_forward      = (IS_BIT_SET(qr.gaxis.axis_switches, 3) ? 1 : 0);
              udp_val.gaxis_state_latch        = (IS_BIT_SET(qr.gaxis.axis_switches, 6) ? 1 : 0);
              udp_val.gaxis_latch_occurred     = (IS_BIT_SET(qr.gaxis.axis_switches, 7) ? 1 : 0);
              udp_val.gaxis_stop_code          = (int)qr.gaxis.stop_code;
              udp_val.gaxis_reference_position = qr.gaxis.reference_position.i;
              udp_val.gaxis_motor_position     = qr.gaxis.motor_position.i;
              udp_val.gaxis_position_error     = qr.gaxis.position_error.i;
              udp_val.gaxis_auxiliary_position = qr.gaxis.auxiliary_position.i;
              udp_val.gaxis_velocity           = qr.gaxis.velocity.i;
              udp_val.gaxis_torque             = qr.gaxis.torque.s;
              udp_val.gaxis_analog_in          = qr.gaxis.analog.s;
              /* haxis */
              udp_val.haxis_motor_off          = (IS_BIT_SET(qr.haxis.axis_status.uc[0], 0) ? 1 : 0);
              udp_val.haxis_error_armed        = (IS_BIT_SET(qr.haxis.axis_status.uc[0], 1) ? 1 : 0);
              udp_val.haxis_latch_armed        = (IS_BIT_SET(qr.haxis.axis_status.uc[0], 2) ? 1 : 0);
              udp_val.haxis_decelerating       = (IS_BIT_SET(qr.haxis.axis_status.uc[0], 3) ? 1 : 0);
              udp_val.haxis_stopping           = (IS_BIT_SET(qr.haxis.axis_status.uc[0], 4) ? 1 : 0);
              udp_val.haxis_slewing            = (IS_BIT_SET(qr.haxis.axis_status.uc[0], 5) ? 1 : 0);
              udp_val.haxis_contour_mode       = (IS_BIT_SET(qr.haxis.axis_status.uc[0], 6) ? 1 : 0);
              udp_val.haxis_negative_direction = (IS_BIT_SET(qr.haxis.axis_status.uc[0], 7) ? 1 : 0);
              udp_val.haxis_motion_mode        = (IS_BIT_SET(qr.haxis.axis_status.uc[1], 0) ? 1 : 0);
              udp_val.haxis_home_1             = (IS_BIT_SET(qr.haxis.axis_status.uc[1], 1) ? 1 : 0);
              udp_val.haxis_home_2             = (IS_BIT_SET(qr.haxis.axis_status.uc[1], 2) ? 1 : 0);
              udp_val.haxis_homing             = (IS_BIT_SET(qr.haxis.axis_status.uc[1], 3) ? 1 : 0);
              udp_val.haxis_finding_edge       = (IS_BIT_SET(qr.haxis.axis_status.uc[1], 4) ? 1 : 0);
              udp_val.haxis_pa_motion          = (IS_BIT_SET(qr.haxis.axis_status.uc[1], 5) ? 1 : 0);
              udp_val.haxis_pr_motion          = (IS_BIT_SET(qr.haxis.axis_status.uc[1], 6) ? 1 : 0);
              udp_val.haxis_moving             = (IS_BIT_SET(qr.haxis.axis_status.uc[1], 7) ? 1 : 0);
              udp_val.haxis_sm_jumper          = (IS_BIT_SET(qr.haxis.axis_switches, 0) ? 1 : 0);
              udp_val.haxis_state_home         = (IS_BIT_SET(qr.haxis.axis_switches, 1) ? 1 : 0);
              udp_val.haxis_state_reverse      = (IS_BIT_SET(qr.haxis.axis_switches, 2) ? 1 : 0);
              udp_val.haxis_state_forward      = (IS_BIT_SET(qr.haxis.axis_switches, 3) ? 1 : 0);
              udp_val.haxis_state_latch        = (IS_BIT_SET(qr.haxis.axis_switches, 6) ? 1 : 0);
              udp_val.haxis_latch_occurred     = (IS_BIT_SET(qr.haxis.axis_switches, 7) ? 1 : 0);
              udp_val.haxis_stop_code          = (int)qr.haxis.stop_code;
              udp_val.haxis_reference_position = qr.haxis.reference_position.i;
              udp_val.haxis_motor_position     = qr.haxis.motor_position.i;
              udp_val.haxis_position_error     = qr.haxis.position_error.i;
              udp_val.haxis_auxiliary_position = qr.haxis.auxiliary_position.i;
              udp_val.haxis_velocity           = qr.haxis.velocity.i;
              udp_val.haxis_torque             = qr.haxis.torque.s;
              udp_val.haxis_analog_in          = qr.haxis.analog.s;
              /* miscellaneous */
              udp_val.a_encoder = qr.aaxis.motor_position.i;
              udp_val.b_encoder = qr.baxis.motor_position.i;
              udp_val.c_encoder = qr.caxis.motor_position.i;
              udp_val.a_position = (float)qr.baxis.analog.s * BOK_LVDT_STEPS;
              udp_val.b_position = (float)qr.daxis.analog.s * BOK_LVDT_STEPS;
              udp_val.c_position = (float)qr.faxis.analog.s * BOK_LVDT_STEPS;
            } else {
              (void) fprintf(stdout, "%s '%s' returns '%s'\n", _NAME_, pr, buffer);
              (void) fflush(stdout);
            }
          }
        }
      }
    }

    /* move record to UDP shared memory and dump */
    (void) memmove(udp_shm_p, &udp_val, UDP_VAL_SIZE);
    dump_udp_structure(udp_shm_p);

    /* move record to TCP shared memory and dump */
    (void) memmove(tcp_shm_p, &tcp_val, TCP_VAL_SIZE);
    dump_tcp_structure(tcp_shm_p);

    /* delay */
    (void) msleep(delay);
  }

  /* close device */
  if (gfd) { GClose(gfd); }

  /* close TCP shared memory */
  tcp_val.shutdown = 1;
  (void) memmove(tcp_shm_p, &tcp_val, TCP_VAL_SIZE);
  if (tcp_shm_p != (tcp_val_p)NULL) { (void) munmap(tcp_shm_p, TCP_VAL_SIZE); }
  if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }

  /* close UDP shared memory */
  udp_val.shutdown = 1;
  (void) memmove(udp_shm_p, &udp_val, UDP_VAL_SIZE);
  if (udp_shm_p != (udp_val_p)NULL) { (void) munmap(udp_shm_p, UDP_VAL_SIZE); }
  if (udp_shm_fd >= 0) { (void) close(udp_shm_fd); }

  /* return */
  return 0;
}
