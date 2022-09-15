/*******************************************************************************
 *
 * bokGalil.h
 *
 ******************************************************************************/


/*******************************************************************************
 * include(s)
 ******************************************************************************/
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include "gclibo.h"


/*******************************************************************************
 * define(s)
 ******************************************************************************/
#define _AUTHOR_             "P. N. Daly"
#define _DATE_               "26-January-2022"
#define _EMAIL_              "pndaly@arizona.edu"
#define _VERSION_            "1.1.0"

#define BOK_AXES             8
#define BOK_BAD_FLOAT        (float)123456789.0
#define BOK_IFILTERS         6
#define BOK_IFILTER_FILE     "bok_ifilters.txt"
#define BOK_IFILTER_COLUMNS  3
#define BOK_IFILTER_SLOTS    256
#define BOK_GFILTERS         6
#define BOK_GFILTER_FILE     "bok_gfilters.txt"
#define BOK_GFILTER_COLUMNS  3
#define BOK_GFILTER_SLOTS    7
#define BOK_SFILTERS         6
#define BOK_SFILTER_FILE     "bok_sfilters.txt"
#define BOK_SFILTER_COLUMNS  3
#define BOK_SFILTER_SLOTS    7
#define BOK_IAXIS_INPUTS     10
#define BOK_NG_TELESCOPE     "BOK"
#define BOK_NG_INSTRUMENT    "90PRIME"
#define BOK_NG_COMMAND       "COMMAND"
#define BOK_NG_REQUEST       "REQUEST"
#define BOK_NG_STRING        1024
#define BOK_NG_BUCKETS       64
#define BOK_NG_WORD          256
#define BOK_LVDT_STEPS       (10.0/32767.0)    /* In the old code this is $adconv?        */
#define BOK_LVDT_ATOD        (-1.0/0.00055)    /* In the old code this is $stepsperlvdtu? */
#define BOK_MAXINT           2147483647
#define BOK_NPLANE_FILE      "bok_nominal_plane.txt"
#define BOK_SHM_FLAG         0
#define BOK_SHM_TCP_NAME     "tcp_shm"
#define BOK_SHM_UDP_NAME     "udp_shm"
#define BOK_STR_4            4
#define BOK_STR_8            8
#define BOK_STR_16           16
#define BOK_STR_32           32
#define BOK_STR_64           64
#define BOK_STR_128          128
#define BOK_STR_256          256
#define BOK_STR_512          512
#define BOK_STR_1024         1024
#define BOK_STR_2048         2048
#define BOK_TCP_DELAY_MS     500
#define BOK_TIME_STAMP       26
#define BOK_UDP_DELAY_MS     500
#define BOK_UDP_FAUCET_ON    "DR0;IHE=>-3;IHE=192,168,0,1<5078>1;DR1000,4;"
#define BOK_UDP_FAUCET_OFF   "DR0;IHE=>-3;"
#define BOK_MAX_LVDT_DIFF     200.0
#define BOK_MAX_LVDT         2500.0
#define BOK_MIN_LVDT         -150.0


/*******************************************************************************
 * macro(s)
 ******************************************************************************/
#define IS_BIT_SET(x, y) ((x) & (1<<(y)))


/*******************************************************************************
 * typedef(s)
 ******************************************************************************/
typedef struct filter_file_data {
  char code[BOK_STR_16];
  char name[BOK_STR_16];
} filter_file_t, *filter_file_p, **filter_file_s;

typedef signed char byte_t;

typedef unsigned char ubyte_t;

typedef union word_type {
  signed char c[2];
  signed short s;
} word_t, *word_p, **word_s;

typedef union uword_type {
  unsigned char uc[2];
  unsigned short us;
} uword_t, *uword_p, **uword_s;

typedef union long_type {
  signed char c[4];
  signed short s[2];
  signed int i;
} long_t, *long_p, **long_s;

typedef union ulong_type {
  unsigned char uc[4];
  unsigned short us[2];
  unsigned int ui;
} ulong_t, *ulong_p, **ulong_s;

typedef union header_record {
  /* 4 bytes */
  ulong_t header;
} header_record_t, *header_record_p, **header_record_s;
#define HEADER_RECORD_SIZE sizeof(header_record_t)

typedef struct iaxis_record {
  /* 24 bytes */
  uword_t sample_number;
  ubyte_t general_input[10];
  ubyte_t general_output[10];
  ubyte_t error_code;
  ubyte_t general_status;
} iaxis_record_t, *iaxis_record_p, **iaxis_record_s;
#define IAXIS_RECORD_SIZE sizeof(iaxis_record_t)

typedef struct staxis_record {
  /* 8 bytes per axis (expect 2 axes) */
  uword_t segment_count;
  uword_t move_status;
  long_t distance_traveled;
} staxis_record_t, *staxis_record_p, **staxis_record_s;
#define STAXIS_RECORD_SIZE sizeof(staxis_record_t)

typedef struct axis_record {
  /* 28 bytes per axis (expect 8 axes) */
  uword_t axis_status;
  ubyte_t axis_switches;
  ubyte_t stop_code;
  long_t reference_position;
  long_t motor_position;
  long_t position_error;
  long_t auxiliary_position;
  long_t velocity;
  word_t torque;
  word_t analog;
} axis_record_t, *axis_record_p, **axis_record_s;
#define AXIS_RECORD_SIZE sizeof(axis_record_t)

typedef struct qr_record {
  /* (4 + 24 + 8*2 + 28*8) 268 bytes */
  header_record_t header;
  iaxis_record_t iaxis;
  staxis_record_t saxis;
  staxis_record_t taxis;
  axis_record_t aaxis;
  axis_record_t baxis;
  axis_record_t caxis;
  axis_record_t daxis;
  axis_record_t eaxis;
  axis_record_t faxis;
  axis_record_t gaxis;
  axis_record_t haxis;
} qr_record_t, *qr_record_p, **qr_record_s;
#define QR_RECORD_SIZE sizeof(qr_record_t)

typedef struct udp_vals_type {
  double jd;
/*******************************************************************************
*  byte 000: header: unsigned byte, axes enabled or disabled
*    bit 0 (00): A axis enabled if set
*    bit 1 (01): B axis enabled if set
*    bit 2 (02): C axis enabled if set
*    bit 3 (03): D axis enabled if set
*    bit 4 (04): E axis enabled if set
*    bit 5 (05): F axis enabled if set
*    bit 6 (06): G axis enabled if set
*    bit 7 (07): H axis enabled if set
*  byte 001: header: unsigned byte, axes enabled or disabled
*    bit 0 (08): S axis enabled if set
*    bit 1 (09): T axis enabled if set
*    bit 2 (10): I axis enabled if set
*    bit 3 (11): n/a
*    bit 4 (12): n/a
*    bit 5 (13): n/a
*    bit 6 (14): n/a
*    bit 7 (15): 1
*  byte 002: header: byte 0 of unsigned word, response length including header
*  byte 003: header: byte 1 of unsigned word, response length including header
******************************************************************************/
  int aaxis_enabled;
  int baxis_enabled;
  int caxis_enabled;
  int daxis_enabled;
  int eaxis_enabled;
  int faxis_enabled;
  int gaxis_enabled;
  int haxis_enabled;
  int saxis_enabled;
  int taxis_enabled;
  int iaxis_enabled;
  int header_length;
/*******************************************************************************
 *  byte 001: i_data: byte 0 of unsigned word, sample number
 *  byte 002: i_data: byte 1 of unsigned word, sample number
 *  byte 003: i_data: unsigned byte, general input 0
 *  byte 004: i_data: unsigned byte, general input 1
 *  byte 005: i_data: unsigned byte, general input 2
 *  byte 006: i_data: unsigned byte, general input 3
 *  byte 007: i_data: unsigned byte, general input 4
 *  byte 008: i_data: unsigned byte, general input 5
 *  byte 009: i_data: unsigned byte, general input 6
 *  byte 010: i_data: unsigned byte, general input 7
 *  byte 011: i_data: unsigned byte, general input 8
 *  byte 012: i_data: unsigned byte, general input 9
 *  byte 013: i_data: unsigned byte, general output 0
 *  byte 014: i_data: unsigned byte, general output 1
 *  byte 015: i_data: unsigned byte, general output 2
 *  byte 016: i_data: unsigned byte, general output 3
 *  byte 017: i_data: unsigned byte, general output 4
 *  byte 018: i_data: unsigned byte, general output 5
 *  byte 019: i_data: unsigned byte, general output 6
 *  byte 020: i_data: unsigned byte, general output 7
 *  byte 021: i_data: unsigned byte, general output 8
 *  byte 022: i_data: unsigned byte, general output 9
 *  byte 023: i_data: unsigned byte, error code
 *  byte 024: i_data: unsigned byte, general status
 *    bit 0 (00): Echo On
 *    bit 1 (01): Trace On
 *    bit 2 (02): Waiting for input from IN command
 *    bit 3 (03): n/a
 *    bit 4 (04): n/a
 *    bit 5 (05): n/a
 *    bit 6 (06): n/a
 *    bit 7 (07): Program running
 ******************************************************************************/
  int iaxis_sample_number;
  int iaxis_general_input[10];
  int iaxis_general_output[10];
  int iaxis_error_code;
  int iaxis_echo_on;
  int iaxis_trace_on;
  int iaxis_awaiting_input;
  int iaxis_program_running;
/*******************************************************************************
 *  byte 000: saxis: byte 0 of unsigned word, segment count of coordinated move for S plane
 *  byte 001: saxis: byte 1 of unsigned word, segment count of coordinated move for S plane
 *  byte 002: saxis: byte 0 of unsigned word, coordinated motion status for S plane
 *    bit 0 (00): n/a
 *    bit 1 (01): n/a
 *    bit 2 (02): n/a
 *    bit 3 (03): Motion is making final decel
 *    bit 4 (04): Motion is stopping due to ST or limit switch
 *    bit 5 (05): Motion is slewing
 *    bit 6 (06): n/a
 *    bit 7 (07): n/a
 *  byte 003: saxis: byte 1 of unsigned word, coordinated motion status for S plane
 *    bit 0 (08): n/a
 *    bit 1 (09): n/a
 *    bit 2 (10): n/a
 *    bit 3 (11): n/a
 *    bit 4 (12): n/a
 *    bit 5 (13): n/a
 *    bit 6 (14): n/a
 *    bit 7 (15): Move in progress
 *  byte 004: saxis: byte 0 of signed long, distance traveled in coordinated move for S plane
 *  byte 005: saxis: byte 1 of signed long, distance traveled in coordinated move for S plane
 *  byte 006: saxis: byte 2 of signed long, distance traveled in coordinated move for S plane
 *  byte 007: saxis: byte 3 of signed long, distance traveled in coordinated move for S plane
 ******************************************************************************/
  int saxis_segment_count;
  int saxis_decelerating;
  int saxis_stopping;
  int saxis_slewing;
  int saxis_moving;
  int saxis_distance_traveled;
/*******************************************************************************
 *  byte 000: taxis: byte 0 of unsigned word, segment count of coordinated move for S plane
 *  byte 001: taxis: byte 1 of unsigned word, segment count of coordinated move for S plane
 *  byte 002: taxis: byte 0 of unsigned word, coordinated motion status for S plane
 *    bit 0 (00): n/a
 *    bit 1 (01): n/a
 *    bit 2 (02): n/a
 *    bit 3 (03): Motion is making final decel
 *    bit 4 (04): Motion is stopping due to ST or limit switch
 *    bit 5 (05): Motion is slewing
 *    bit 6 (06): n/a
 *    bit 7 (07): n/a
 *  byte 003: taxis: byte 1 of unsigned word, coordinated motion status for S plane
 *    bit 0 (08): n/a
 *    bit 1 (09): n/a
 *    bit 2 (10): n/a
 *    bit 3 (11): n/a
 *    bit 4 (12): n/a
 *    bit 5 (13): n/a
 *    bit 6 (14): n/a
 *    bit 7 (15): Move in progress
 *  byte 004: taxis: byte 0 of signed long, distance traveled in coordinated move for S plane
 *  byte 005: taxis: byte 1 of signed long, distance traveled in coordinated move for S plane
 *  byte 006: taxis: byte 2 of signed long, distance traveled in coordinated move for S plane
 *  byte 007: taxis: byte 3 of signed long, distance traveled in coordinated move for S plane
 ******************************************************************************/
  int taxis_segment_count;
  int taxis_decelerating;
  int taxis_stopping;
  int taxis_slewing;
  int taxis_moving;
  int taxis_distance_traveled;
/*******************************************************************************
 *  byte 001: axis: byte 0 of unsigned word, A axis status
 *    bit 0 (00): Motor off
 *    bit 1 (01): Off-On error armed
 *    bit 2 (02): Latch is armed
 *    bit 3 (03): Motion is making final decel
 *    bit 4 (04): Motion is stopping due to ST or limit
 *    bit 5 (05): Motion is slewing
 *    bit 6 (06): Mode of motion contour
 *    bit 7 (07): Negative direction move
 *  byte 002: axis: byte 1 of unsigned word, A axis status
 *    bit 0 (08): Mode of motion coord motion
 *    bit 1 (09): 2nd phase of HM complete or FI command issued
 *    bit 2 (10): 1st phase of HM complete
 *    bit 3 (11): Home (HM) in progress
 *    bit 4 (12): Find edge (FE) in progress
 *    bit 5 (13): Mode of motion PA only
 *    bit 6 (14): Mode of motion PA or PR
 *    bit 7 (15): Move in progress
 *  byte 003: axis: unsigned byte, A axis switches
 *    bit 0: SM jumper
 *    bit 1: State of home input
 *    bit 2: State of reverse limit
 *    bit 3: State of forward limit
 *    bit 4: n/a
 *    bit 5: n/a
 *    bit 6: State of latch input
 *    bit 7: Latch occurred
 *  byte 004: axis: unsigned byte, A axis stop code
 *  byte 005: axis: byte 0 of signed long, A axis reference position   # RP;
 *  byte 006: axis: byte 1 of signed long, A axis reference position   # RP;
 *  byte 007: axis: byte 2 of signed long, A axis reference position   # RP;
 *  byte 008: axis: byte 3 of signed long, A axis reference position   # RP;
 *  byte 009: axis: byte 0 of signed long, A axis motor position       # TP;
 *  byte 010: axis: byte 1 of signed long, A axis motor position       # TP;
 *  byte 011: axis: byte 2 of signed long, A axis motor position       # TP;
 *  byte 012: axis: byte 3 of signed long, A axis motor position       # TP;
 *  byte 013: axis: byte 0 of signed long, A axis position error       # TE;  ??
 *  byte 014: axis: byte 1 of signed long, A axis position error       # TE;  ??
 *  byte 015: axis: byte 2 of signed long, A axis position error       # TE;  ??
 *  byte 016: axis: byte 3 of signed long, A axis position error       # TE;  ??
 *  byte 017: axis: byte 0 of signed long, A axis auxiliary position   # TD;
 *  byte 018: axis: byte 1 of signed long, A axis auxiliary position   # TD;
 *  byte 019: axis: byte 2 of signed long, A axis auxiliary position   # TD;
 *  byte 020: axis: byte 3 of signed long, A axis auxiliary position   # TD;
 *  byte 021: axis: byte 0 of signed long, A axis velocity             # TV; ??
 *  byte 022: axis: byte 1 of signed long, A axis velocity             # TV; ??
 *  byte 023: axis: byte 2 of signed long, A axis velocity             # TV; ??
 *  byte 024: axis: byte 3 of signed long, A axis velocity             # TV; ??
 *  byte 025: axis: byte 0 of signed word, A axis torque               # TT; ??
 *  byte 026: axis: byte 1 of signed word, A axis torque               # TT; ??
 *  byte 027: axis: byte 0 of signed word, A axis analog input
 *  byte 028: axis: byte 1 of signed word, A axis analog input
 ******************************************************************************/
  /* aaxis */
  int aaxis_motor_off;
  int aaxis_error_armed;
  int aaxis_latch_armed;
  int aaxis_decelerating;
  int aaxis_stopping;
  int aaxis_slewing;
  int aaxis_contour_mode;
  int aaxis_negative_direction;
  int aaxis_motion_mode;
  int aaxis_home_1;
  int aaxis_home_2;
  int aaxis_homing;
  int aaxis_finding_edge;
  int aaxis_pa_motion;
  int aaxis_pr_motion;
  int aaxis_moving;
  int aaxis_sm_jumper;
  int aaxis_state_home;
  int aaxis_state_reverse;
  int aaxis_state_forward;
  int aaxis_state_latch;
  int aaxis_latch_occurred;
  int aaxis_stop_code;
  int aaxis_reference_position;
  int aaxis_motor_position;
  int aaxis_position_error;
  int aaxis_auxiliary_position;
  int aaxis_velocity;
  int aaxis_torque;
  int aaxis_analog_in;
  /* baxis */
  int baxis_motor_off;
  int baxis_error_armed;
  int baxis_latch_armed;
  int baxis_decelerating;
  int baxis_stopping;
  int baxis_slewing;
  int baxis_contour_mode;
  int baxis_negative_direction;
  int baxis_motion_mode;
  int baxis_home_1;
  int baxis_home_2;
  int baxis_homing;
  int baxis_finding_edge;
  int baxis_pa_motion;
  int baxis_pr_motion;
  int baxis_moving;
  int baxis_sm_jumper;
  int baxis_state_home;
  int baxis_state_reverse;
  int baxis_state_forward;
  int baxis_state_latch;
  int baxis_latch_occurred;
  int baxis_stop_code;
  int baxis_reference_position;
  int baxis_motor_position;
  int baxis_position_error;
  int baxis_auxiliary_position;
  int baxis_velocity;
  int baxis_torque;
  int baxis_analog_in;
  /* caxis */
  int caxis_motor_off;
  int caxis_error_armed;
  int caxis_latch_armed;
  int caxis_decelerating;
  int caxis_stopping;
  int caxis_slewing;
  int caxis_contour_mode;
  int caxis_negative_direction;
  int caxis_motion_mode;
  int caxis_home_1;
  int caxis_home_2;
  int caxis_homing;
  int caxis_finding_edge;
  int caxis_pa_motion;
  int caxis_pr_motion;
  int caxis_moving;
  int caxis_sm_jumper;
  int caxis_state_home;
  int caxis_state_reverse;
  int caxis_state_forward;
  int caxis_state_latch;
  int caxis_latch_occurred;
  int caxis_stop_code;
  int caxis_reference_position;
  int caxis_motor_position;
  int caxis_position_error;
  int caxis_auxiliary_position;
  int caxis_velocity;
  int caxis_torque;
  int caxis_analog_in;
  /* daxis */
  int daxis_motor_off;
  int daxis_error_armed;
  int daxis_latch_armed;
  int daxis_decelerating;
  int daxis_stopping;
  int daxis_slewing;
  int daxis_contour_mode;
  int daxis_negative_direction;
  int daxis_motion_mode;
  int daxis_home_1;
  int daxis_home_2;
  int daxis_homing;
  int daxis_finding_edge;
  int daxis_pa_motion;
  int daxis_pr_motion;
  int daxis_moving;
  int daxis_sm_jumper;
  int daxis_state_home;
  int daxis_state_reverse;
  int daxis_state_forward;
  int daxis_state_latch;
  int daxis_latch_occurred;
  int daxis_stop_code;
  int daxis_reference_position;
  int daxis_motor_position;
  int daxis_position_error;
  int daxis_auxiliary_position;
  int daxis_velocity;
  int daxis_torque;
  int daxis_analog_in;
  /* eaxis */
  int eaxis_motor_off;
  int eaxis_error_armed;
  int eaxis_latch_armed;
  int eaxis_decelerating;
  int eaxis_stopping;
  int eaxis_slewing;
  int eaxis_contour_mode;
  int eaxis_negative_direction;
  int eaxis_motion_mode;
  int eaxis_home_1;
  int eaxis_home_2;
  int eaxis_homing;
  int eaxis_finding_edge;
  int eaxis_pa_motion;
  int eaxis_pr_motion;
  int eaxis_moving;
  int eaxis_sm_jumper;
  int eaxis_state_home;
  int eaxis_state_reverse;
  int eaxis_state_forward;
  int eaxis_state_latch;
  int eaxis_latch_occurred;
  int eaxis_stop_code;
  int eaxis_reference_position;
  int eaxis_motor_position;
  int eaxis_position_error;
  int eaxis_auxiliary_position;
  int eaxis_velocity;
  int eaxis_torque;
  int eaxis_analog_in;
  /* faxis */
  int faxis_motor_off;
  int faxis_error_armed;
  int faxis_latch_armed;
  int faxis_decelerating;
  int faxis_stopping;
  int faxis_slewing;
  int faxis_contour_mode;
  int faxis_negative_direction;
  int faxis_motion_mode;
  int faxis_home_1;
  int faxis_home_2;
  int faxis_homing;
  int faxis_finding_edge;
  int faxis_pa_motion;
  int faxis_pr_motion;
  int faxis_moving;
  int faxis_sm_jumper;
  int faxis_state_home;
  int faxis_state_reverse;
  int faxis_state_forward;
  int faxis_state_latch;
  int faxis_latch_occurred;
  int faxis_stop_code;
  int faxis_reference_position;
  int faxis_motor_position;
  int faxis_position_error;
  int faxis_auxiliary_position;
  int faxis_velocity;
  int faxis_torque;
  int faxis_analog_in;
  /* gaxis */
  int gaxis_motor_off;
  int gaxis_error_armed;
  int gaxis_latch_armed;
  int gaxis_decelerating;
  int gaxis_stopping;
  int gaxis_slewing;
  int gaxis_contour_mode;
  int gaxis_negative_direction;
  int gaxis_motion_mode;
  int gaxis_home_1;
  int gaxis_home_2;
  int gaxis_homing;
  int gaxis_finding_edge;
  int gaxis_pa_motion;
  int gaxis_pr_motion;
  int gaxis_moving;
  int gaxis_sm_jumper;
  int gaxis_state_home;
  int gaxis_state_reverse;
  int gaxis_state_forward;
  int gaxis_state_latch;
  int gaxis_latch_occurred;
  int gaxis_stop_code;
  int gaxis_reference_position;
  int gaxis_motor_position;
  int gaxis_position_error;
  int gaxis_auxiliary_position;
  int gaxis_velocity;
  int gaxis_torque;
  int gaxis_analog_in;
  /* haxis */
  int haxis_motor_off;
  int haxis_error_armed;
  int haxis_latch_armed;
  int haxis_decelerating;
  int haxis_stopping;
  int haxis_slewing;
  int haxis_contour_mode;
  int haxis_negative_direction;
  int haxis_motion_mode;
  int haxis_home_1;
  int haxis_home_2;
  int haxis_homing;
  int haxis_finding_edge;
  int haxis_pa_motion;
  int haxis_pr_motion;
  int haxis_moving;
  int haxis_sm_jumper;
  int haxis_state_home;
  int haxis_state_reverse;
  int haxis_state_forward;
  int haxis_state_latch;
  int haxis_latch_occurred;
  int haxis_stop_code;
  int haxis_reference_position;
  int haxis_motor_position;
  int haxis_position_error;
  int haxis_auxiliary_position;
  int haxis_velocity;
  int haxis_torque;
  int haxis_analog_in;
/*******************************************************************************
 * housekeeping
 ******************************************************************************/
  int a_encoder;
  int b_encoder;
  int c_encoder;
  float a_position;
  float b_position;
  float c_position;
  int simulate;
  int shutdown;
  int counter;
  char timestamp[BOK_TIME_STAMP];
} udp_val_t, *udp_val_p, **udp_val_s;
#define UDP_VAL_SIZE sizeof(udp_val_t)


/*******************************************************************************
 * structure(s)
 ******************************************************************************/
typedef struct tcp_command_type {
  char name[BOK_STR_64];
  char help[BOK_STR_128];
} tcp_cmd_t, *tcp_cmd_p, **tcp_cmd_s;

typedef struct lv_vals_type {
  float dista;
  float distall;
  float distb;
  float distc;
  float distgcam;
  float errfilt;
  float filtbit1;
  float filtbit2;
  float filtbit3;
  float filtbit4;
  float filtbit5;
  float filtbit6;
  float filtbit7;
  float filtbit8;
  float filthigh;
  float filtisin;
  float filtlow;
  float filtnum;
  float filtrac;
  float filtrbl;
  float filtrdc;
  float filtrfl;
  float filtrsp;
  float filttac;
  float filttdc;
  float filttdis;
  float filttnud;
  float filttsc;
  float filttsp;
  float filtval;
  float fnum;
  float fnum_in;
  float focac;
  float focbl;
  float focdc;
  float focfl;
  float focrefa;
  float focrefb;
  float focrefc;
  float focrfset;
  float focsp;
  float gfac;
  float gfcent;
  float gfdc;
  float gfiltac;
  float gfiltdc;
  float gfiltn;
  float gfiltq;
  float gfiltreq;
  float gfiltsp;
  float gfsp;
  float gifltn;
  float initfilt;
  float nmoves;
  float nrot;
  float reqfilt;
  float snum;
  float snum_in;
  float totfoca;
  float totfocb;
  float totfocc;
  float vecac;
  float vecdc;
  float vecsp;
} lv_val_t, *lv_val_p, **lv_val_s;

typedef struct tcp_vals_type {
  double jd;
  int status;
  float filtvals[BOK_IFILTERS];
  float position[BOK_AXES];
  lv_val_t lv;
  GReturn gstatus;
  int simulate;
  int shutdown;
  int counter;
  char timestamp[BOK_TIME_STAMP];
  char hardware[BOK_STR_64];
  char software[BOK_STR_64];
} tcp_val_t, *tcp_val_p, **tcp_val_s;
#define TCP_VAL_SIZE sizeof(tcp_val_t)


/*******************************************************************************
 * tcp command(s)
 ******************************************************************************/
const tcp_cmd_t TcpCmds[] = {
  {"FILTVALS[0]=?;",     "filter number in slot 0",            },
  {"FILTVALS[1]=?;",     "filter number in slot 1",            },
  {"FILTVALS[2]=?;",     "filter number in slot 2",            },
  {"FILTVALS[3]=?;",     "filter number in slot 3",            },
  {"FILTVALS[4]=?;",     "filter number in slot 4",            },
  {"FILTVALS[5]=?;",     "filter number in slot 5",            },
  {"LV;",                "list variable",                      },
  {"TB;",                "tell status byte",                   },
  {"TP;",                "tell (motor) position",              }
};
#define TCP_CMD_NELMS (sizeof(TcpCmds)/sizeof(tcp_cmd_t))


/*******************************************************************************
 * prototype(s)
 ******************************************************************************/
void chomp                       (char *, char *);
void cliDump                     (unsigned int, char *[]);
void cliFree                     (unsigned int, char *[]);
void cliInit                     (unsigned int, size_t, char *[]);
void cliMalloc                   (unsigned int, size_t, char *[]);
void cliParse                    (const char *, const char *, const size_t,unsigned int *, size_t, char *[]);
void decode_fvals                (char *, float *, int, char);
void decode_float                (char *, float *);
void decode_integer              (char *, int *);
void dump_tcp_structure          (tcp_val_t *);
void dump_udp_structure          (udp_val_t *);
double get_jd                    (struct tm *);
void galil_check                 (GReturn, GCon *, bool, bool);
void logtime                     (const char *, ...);
void read_filters_from_file      (char *, filter_file_t [], int, int);
void read_nominal_plane_from_file(char *, float *, float *, float *);
void save_nominal_plane_to_file  (char *, float, float, float);
void replace                     (char *, char *, char *);
void replace_word                (char *, size_t, const char *, const char *);

GReturn xq                       (char *);
GReturn xq_dista                 (float);
GReturn xq_distall               (float);
GReturn xq_distb                 (float);
GReturn xq_distc                 (float);
GReturn xq_distgcam              (float);
GReturn xq_coordmv               (void);
GReturn xq_filtin                (void);
GReturn xq_filtldm               (void);
GReturn xq_filtmov               (void);
GReturn xq_filtout               (void);
GReturn xq_filtrd                (void);
GReturn xq_focgcam               (void);
GReturn xq_focind                (void);
GReturn xq_gfiltn                (float);
GReturn xq_gfwmov                (void);
GReturn xq_gfwinit               (void);
GReturn xq_hx                    (void);
GReturn xq_reqfilt               (float);
GReturn xq_focusall              (float);
GReturn xq_focusind              (float, float, float);
GReturn xq_gfocus                (float);
