/*******************************************************************************
 *
 * Galil_DMC_22x0_UDP_Write.c
 *
 ******************************************************************************/


/*******************************************************************************
 * include(s)
 ******************************************************************************/
#include "__hosts__.h"
#include "bokGalil.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>


/*******************************************************************************
 * signal(s)
 ******************************************************************************/
static int running = 1;
void galil_handler(int mysig) { running = 0; }


/*******************************************************************************
 * define(s)
 ******************************************************************************/
#define _HELP_ "UDP record(s) for the Galil_DMC_22x0 via gclib"
#define _NAME_ "Galil_DMC_22x0_UDP_Write"


/*******************************************************************************
 * function: main()
 ******************************************************************************/
int main( int argc, char *argv[] ) {

  /* declare some variables and initialize them */
  bool simulate = false;
  char buffer[BOK_STR_2048] = {'\0'};
  char cmnd[BOK_STR_64] = {'\0'};
  GCon gfd = G_NO_ERROR;
  GReturn gstat = G_NO_ERROR;
  int bok = 1;
  int counter = -1;
  int delay = BOK_UDP_DELAY_MS;
  int istat = 0;
  int port = BOK_UDP_PORT;
  int sfd = 0;
  int udp_shm_fd = -1;
  int udp_shm_stat = -1;
  socklen_t sin_len = (socklen_t)-1;
  ssize_t rx = (ssize_t)-1;
  time_t now = (time_t)NULL;
  udp_val_p udp_shm_p = (udp_val_p)NULL;

  /* structure(s) */
  qr_record_t qr;
  struct sockaddr_in sin;
  udp_val_t udp_val;

  sin_len = sizeof(sin);
  (void) memset(&cmnd, 0, sizeof(cmnd));
  (void) memset(&qr, 0, sizeof(qr));
  (void) memset(&sin, 0, sizeof(sin));
  (void) memset(&udp_val, 0, sizeof(udp_val));

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

  /* create shared memory segment */
  (void) shm_unlink(BOK_SHM_UDP_NAME);
  if ((udp_shm_fd=shm_open(BOK_SHM_UDP_NAME, O_CREAT | O_RDWR, 0777)) < 0) {
    (void) fprintf(stderr, "%s <ERROR> failed to execute '%s' exiting, udp_shm_fd=%d\n", _NAME_, "shm_open()", udp_shm_fd);
    (void) fflush(stderr);
    exit(udp_shm_fd);
  }

  /* (re)size shared memory segment */
  if ((udp_shm_stat=ftruncate(udp_shm_fd, UDP_VAL_SIZE)) < 0) {
    (void) fprintf(stderr, "%s <ERROR> failed to execute '%s' exiting, udp_shm_stat=%d\n", _NAME_, "ftruncate()", udp_shm_stat);
    (void) fflush(stderr);
    exit(udp_shm_stat);
  }
  
  /* memory map the shared memory object */
  if ((udp_shm_p=(udp_val_p)mmap(0, UDP_VAL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, udp_shm_fd, 0)) == (udp_val_p)NULL) {
    (void) fprintf(stderr, "%s <ERROR> failed to execute '%s' exiting, udp_shm_ptr=%p\n", _NAME_, "mmap()", udp_shm_p);
    (void) fflush(stderr);
    exit(-1);
  }

  /* open the ip_addr */
  if (gfd) { (void) GClose(gfd); }
  gstat = G_NO_ERROR;
  if ( ! simulate ) {
    if (bok > 0) {
      (void) sprintf(cmnd, "%s --command TCP", BOK_GALIL_CMD_BOK);
      (void) fprintf(stdout, "%s <%s> executing '%s', gstat=%d, gfd=%p\n", _NAME_, (simulate == true ? "SIM" : "OK"), cmnd, gstat, gfd);
      (void) fflush(stdout);
      if ((gstat=GOpen(cmnd, &gfd)) != G_NO_ERROR) { simulate = true; }
      (void) printf("GOpen('%s') called, gstat=%d, gfd=%ld\n", cmnd, (int)gstat, (long)gfd);
      (void) fprintf(stdout, "%s <%s> executed '%s', gstat=%d, gfd=%p\n", _NAME_, (simulate == true ? "SIM" : "OK"), cmnd, gstat, gfd);
      (void) fflush(stdout);
    } else {
      (void) sprintf(cmnd, "%s --command TCP", BOK_GALIL_CMD_LAB);
      (void) fprintf(stdout, "%s <%s> executing '%s', gstat=%d, gfd=%p\n", _NAME_, (simulate == true ? "SIM" : "OK"), cmnd, gstat, gfd);
      (void) fflush(stdout);
      if ((gstat=GOpen(cmnd, &gfd)) != G_NO_ERROR) { simulate = true; }
      (void) printf("GOpen('%s') called, gstat=%d, gfd=%ld\n", cmnd, (int)gstat, (long)gfd);
      (void) fprintf(stdout, "%s <%s> executed '%s', gstat=%d, gfd=%p\n", _NAME_, (simulate == true ? "SIM" : "OK"), cmnd, gstat, gfd);
      (void) fflush(stdout);
    }
  }

  /* turn on the UDP faucet */
  if ( ! simulate) {
    (void) memset((void *)buffer, 0, sizeof(buffer));
    if ((gstat=GCommand(gfd, BOK_UDP_FAUCET_ON, buffer, sizeof(buffer), 0)) != G_NO_ERROR) {
      (void) fprintf(stderr, "%s <ERROR> failed to execute '%s', simulating, gstat=%d\n", _NAME_, BOK_UDP_FAUCET_ON, (int)gstat);
      (void) fflush(stderr);
      simulate = true;
    }
  }
  if (gfd) { (void) GClose(gfd); }

  /* create socket */
  if (!simulate && (sfd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    (void) fprintf(stderr, "%s <ERROR> failed to open socket, error='%s'\n", _NAME_, strerror(errno));
    (void) fflush(stderr);
    simulate = true;
  }

  /* initialize structure(s) */
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = inet_addr(BOK_UDP_ADDR);

  /* bind socket */
  if (!simulate && (istat=bind(sfd, (struct sockaddr *)&sin, sizeof(sin))) < 0 ) {
    (void) fprintf(stderr, "%s <ERROR> failed to bind socket, error='%s'\n", _NAME_, strerror(errno));
    (void) fflush(stderr);
    simulate = true;
  }

  /* loop for command(s) */
  while (running > 0) {

    /* reset record */
    (void) memset((void *)&udp_val, 0, sizeof(udp_val));

    /* initialize element(s) */
    now = time(NULL);
    (void) strftime(udp_val.timestamp, sizeof(udp_val.timestamp), "%Y-%m-%dT%X.00", localtime(&now));
    udp_val.jd = get_jd(localtime(&now));
    udp_val.shutdown = 0;
    udp_val.counter = ++counter;
    udp_val.simulate = simulate == true ? 1 : 0;

    /* create a simulated record using snapshot data */
    if (simulate == true) {
      /* header */
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

    /* create record from udp packet(s) */
    } else {
      (void) memset((void *)buffer, '\0', sizeof(buffer));
      if ((rx=recvfrom(sfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&sin, (socklen_t *)&sin_len)) > (ssize_t)0) {
        chomp(buffer, "\n");
        chomp(buffer, "\r");
        (void) memset(&qr, 0, sizeof(qr));
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
      }
    }

    /* move record to shared memory */
    (void) memmove(udp_shm_p, &udp_val, UDP_VAL_SIZE);

    /* dump shared memory */
    dump_udp_structure(udp_shm_p);

    /* delay */
    (void) usleep((useconds_t)delay*1000);
  }

  /* close device */
  if (sfd) { (void) close(sfd); }

  /* open the ip_addr */
  if (gfd) { (void) GClose(gfd); }
  gstat = G_NO_ERROR;
  if ( ! simulate ) {
    if (bok > 0) {
      (void) fprintf(stdout, "%s <%s> executing '%s', gstat=%d, gfd=%p\n", _NAME_, (simulate == true ? "SIM" : "OK"), BOK_GALIL_CMD_BOK, gstat, gfd);
      (void) fflush(stdout);
      if ((gstat=GOpen(BOK_GALIL_CMD_BOK, &gfd)) != G_NO_ERROR) { simulate = true; }
      (void) printf("GOpen('%s') called, gstat=%d, gfd=%ld\n", BOK_GALIL_CMD_BOK, (int)gstat, (long)gfd);
      (void) fprintf(stdout, "%s <%s> executed '%s', gstat=%d, gfd=%p\n", _NAME_, (simulate == true ? "SIM" : "OK"), BOK_GALIL_CMD_BOK, gstat, gfd);
      (void) fflush(stdout);
    } else {
      (void) fprintf(stdout, "%s <%s> executing '%s', gstat=%d, gfd=%p\n", _NAME_, (simulate == true ? "SIM" : "OK"), BOK_GALIL_CMD_LAB, gstat, gfd);
      (void) fflush(stdout);
      if ((gstat=GOpen(BOK_GALIL_CMD_LAB, &gfd)) != G_NO_ERROR) { simulate = true; }
      (void) printf("GOpen('%s') called, gstat=%d, gfd=%ld\n", BOK_GALIL_CMD_LAB, (int)gstat, (long)gfd);
      (void) fprintf(stdout, "%s <%s> executed '%s', gstat=%d, gfd=%p\n", _NAME_, (simulate == true ? "SIM" : "OK"), BOK_GALIL_CMD_LAB, gstat, gfd);
      (void) fflush(stdout);
    }
  }

  /* turn off the UDP faucet */
  if ( ! simulate) {
    (void) memset((void *)buffer, 0, sizeof(buffer));
    if ((gstat=GCommand(gfd, BOK_UDP_FAUCET_OFF, buffer, sizeof(buffer), 0)) != G_NO_ERROR) {
      (void) fprintf(stderr, "%s <ERROR> failed to execute '%s', simulating, gstat=%d\n", _NAME_, BOK_UDP_FAUCET_ON, (int)gstat);
      (void) fflush(stderr);
      simulate = true;
    }
  }
  if (gfd) { (void) GClose(gfd); }

  /* close shared memory */
  udp_val.shutdown = 1;
  (void) memmove(udp_shm_p, &udp_val, UDP_VAL_SIZE);
  if (udp_shm_p != (udp_val_p)NULL) { (void) munmap(udp_shm_p, UDP_VAL_SIZE); }
  if (udp_shm_fd >= 0) { (void) close(udp_shm_fd); }

  /* return */
  return 0;
}
