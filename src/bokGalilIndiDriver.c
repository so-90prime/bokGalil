/******************************************************************************
 *
 * bokGalilIndiDriver.c
 *
 ******************************************************************************/


/*******************************************************************************
 * include(s)
 ******************************************************************************/
#include "bokGalil.h"


/*******************************************************************************
 * INDI include(s)
 ******************************************************************************/
#include <libindi/indiapi.h>
#include <libindi/indidevapi.h>
#include <libindi/eventloop.h>
#include <libindi/indicom.h>


/*******************************************************************************
 * define(s)
 ******************************************************************************/
#define _HELP_          "INDI driver for the Galil_DMC_22x0 via gclib"
#define _NAME_          "bokGalilIndiDriver"

#define GALIL_DEVICE    "GALIL-DMC-2280"

#define GFILTER_GROUP   "Guider Filter"
#define GFOCUS_GROUP    "Guider Focus"
#define IFILTER_GROUP   "Instrument Filter"
#define IFOCUS_GROUP    "Instrument Focus"
#define SUPPORT_GROUP   "Support"
#define TELEMETRY_GROUP "Telemetry"


/*******************************************************************************
 * typedef(s)
 ******************************************************************************/
typedef struct filternamedata {
  char filter_1[BOK_STR_32];
  char filter_2[BOK_STR_32];
  char filter_3[BOK_STR_32];
  char filter_4[BOK_STR_32];
  char filter_5[BOK_STR_32];
  char filter_6[BOK_STR_32];
} filter_name_t, *filter_name_p, **filter_name_s;

typedef struct ifocusdata {
  float enca;
  float encb;
  float encc;
  float lima;
  float limb;
  float limc;
  float noma;
  float nomb;
  float nomc;
  float refa;
  float refb;
  float refc;
  float vala;
  float valb;
  float valc;
  float tolerance;
} ifocus_t, *ifocus_p, **ifocus_s;

typedef struct supportdata {
  char author[BOK_STR_32];
  char date[BOK_STR_32];
  char email[BOK_STR_32];
  char version[BOK_STR_32];
} support_t, *support_p, **support_s;

typedef struct telemetrydata {
  char hardware[BOK_STR_64];
  char software[BOK_STR_64];
  char timestamp[BOK_TIME_STAMP];
  char jd[BOK_STR_32];
  char tcp_shm_mode[BOK_STR_32];
  char udp_shm_mode[BOK_STR_32];
  char a_encoder[BOK_STR_32];
  char b_encoder[BOK_STR_32];
  char c_encoder[BOK_STR_32];
  char a_position[BOK_STR_32];
  char b_position[BOK_STR_32];
  char c_position[BOK_STR_32];
  char a_reference[BOK_STR_32];
  char b_reference[BOK_STR_32];
  char c_reference[BOK_STR_32];
  char lvdta[BOK_STR_32];
  char lvdtb[BOK_STR_32];
  char lvdtc[BOK_STR_32];
  char tolerance[BOK_STR_32];
  char distall[BOK_STR_32];
  char dista[BOK_STR_32];
  char distb[BOK_STR_32];
  char distc[BOK_STR_32];
  char distgcam[BOK_STR_32];
  char errfilt[BOK_STR_32];
  char filttsc[BOK_STR_32];
  char filtisin[BOK_STR_32];
  char filtval[BOK_STR_32];
  char ifilter_1[BOK_STR_64];
  char ifilter_2[BOK_STR_64];
  char ifilter_3[BOK_STR_64];
  char ifilter_4[BOK_STR_64];
  char ifilter_5[BOK_STR_64];
  char ifilter_6[BOK_STR_64];
  char gfiltn[BOK_STR_64];
  char fnum_in[BOK_STR_64];
  char reqfilt[BOK_STR_64];
  char snum[BOK_STR_64];
  char snum_in[BOK_STR_64];
  char gfilter_1[BOK_STR_64];
  char gfilter_2[BOK_STR_64];
  char gfilter_3[BOK_STR_64];
  char gfilter_4[BOK_STR_64];
  char gfilter_5[BOK_STR_64];
  char gfilter_6[BOK_STR_64];
  char gfocus_position[BOK_STR_32];
  char gfocus_limit[BOK_STR_32];
} telemetry_t, *telemetry_p, **telemetry_s;


/*******************************************************************************
 * prototype(s)
 ******************************************************************************/
static void driver_init(void);
static void execute_astronomer_actions(ISState [], char *[], int);
static void execute_gfilter_switches(ISState [], char *[], int);
static void execute_gfilter_change(ISState [], char *[], int);
static void execute_ifilter_engineering(ISState [], char *[], int);
static void execute_ifilter_switches(ISState [], char *[], int);
static void execute_ifilter_change(ISState [], char *[], int);
static void execute_ifocus_reference_switches(ISState [], char *[], int);
static void scheduled_telemetry(void *);
static void unscheduled_telemetry(void *);
static void zero_telemetry(void);

GReturn ifilter_load_doit(void);
GReturn ifilter_move_doit(int);
GReturn ifilter_read_doit(void);
GReturn ifilter_unload_doit(void);
GReturn astronomer_action_doit(bool);


/*******************************************************************************
 * static variable(s)
 ******************************************************************************/
static bool busy = false;
static bool popact = false;
static bool is_done = false;
static filter_file_t bok_ifilters[BOK_IFILTER_SLOTS];
static filter_file_t bok_gfilters[BOK_GFILTER_SLOTS];
static filter_file_t bok_sfilters[BOK_SFILTER_SLOTS];
static bool initialized = false;
static int timer_id = -1;
static tcp_val_t tcp_val;
static udp_val_t udp_val;


/*******************************************************************************
 * global scope variable(s)
 ******************************************************************************/
filter_name_t gfilter_names;
filter_name_t ifilter_names;
ifocus_t ifoci;
support_t supports;
telemetry_t telemetrys;


/*******************************************************************************
 * INDI widget(s)
 ******************************************************************************/
/* GFILTER_GROUP */
static ISwitch gfilterS[] = {
  {"g_initfw", "Initialize Guider Filter", ISS_OFF, 0, 0},
};
ISwitchVectorProperty gfilterSP = {
  GALIL_DEVICE, "GFILTER_ACTIONS", "Guider Filter Actions", GFILTER_GROUP, IP_RW, ISR_1OFMANY, 0.0, IPS_IDLE, gfilterS, NARRAY(gfilterS), "", 0
};

static ISwitch gfilter_changeS[] = {
  {"g_slot_1",   "gFilter 1", ISS_OFF, 0, 0},
  {"g_slot_2",   "gFilter 2", ISS_OFF, 0, 0},
  {"g_slot_3",   "gFilter 3", ISS_OFF, 0, 0},
  {"g_slot_4",   "gFilter 4", ISS_OFF, 0, 0},
  {"g_slot_5",   "gFilter 5", ISS_OFF, 0, 0},
  {"g_slot_6",   "gFilter 6", ISS_OFF, 0, 0}
};
ISwitchVectorProperty gfilter_changeSP = {
  GALIL_DEVICE, "GFILTER_CHANGE", "Guider Filter(s)", GFILTER_GROUP, IP_RW, ISR_1OFMANY, 0.0, IPS_IDLE, gfilter_changeS, NARRAY(gfilter_changeS), "", 0
};

static IText gfilterT[] = {
  {"gfilter_1", "gFilter 1", gfilter_names.filter_1, 0, 0, 0},
  {"gfilter_2", "gFilter 2", gfilter_names.filter_2, 0, 0, 0},
  {"gfilter_3", "gFilter 3", gfilter_names.filter_3, 0, 0, 0},
  {"gfilter_4", "gFilter 4", gfilter_names.filter_4, 0, 0, 0},
  {"gfilter_5", "gFilter 5", gfilter_names.filter_5, 0, 0, 0},
  {"gfilter_6", "gFilter 6", gfilter_names.filter_6, 0, 0, 0}
};
static ITextVectorProperty gfilterTP = {
  GALIL_DEVICE, "GFILTER_NAMES", "Guider Filter Name(s)", GFILTER_GROUP, IP_RO, 0.0, IPS_IDLE, gfilterT, NARRAY(gfilterT), "", 0
};

/* GFOCUS_GROUP */
/* inner limit is +4158 (stop code 2), outer limit is -5979 (stop code 3) but these limits (-1000,1000) are to
 * stop the user sending too large a value at once */
static INumber gfocus_distN[] = {
  {"distgcam", "Delta Focus", "%5.0f", -1000.0, 1000.0, 1.0, 0.0, 0, 0, 0}
};
static INumberVectorProperty gfocus_distNP = {
  GALIL_DEVICE, "GFOCUS_DIST", "Guider Focus", GFOCUS_GROUP, IP_RW, 0.0, IPS_IDLE, gfocus_distN, NARRAY(gfocus_distN), "", 0
};

/* IFILTER_GROUP */
static ISwitch ifilter_engineeringS[] = {
  {"i_populate", "Populate",                     ISS_OFF, 0, 0},
  {"i_popdone",  "Populate Done",                ISS_OFF, 0, 0},
  {"i_initfw",   "Initialize Instrument Filter", ISS_OFF, 0, 0},
  {"i_hx",       "Halt Execution",               ISS_OFF, 0, 0}
};
ISwitchVectorProperty ifilter_engineeringSP = {
  GALIL_DEVICE, "IFILTER_ENGINEERING", "Day Crew Action(s)", IFILTER_GROUP, IP_RW, ISR_1OFMANY, 0.0, IPS_IDLE, ifilter_engineeringS, NARRAY(ifilter_engineeringS), "", 0
};

static ISwitch ifilterS[] = {
  {"i_load",   "Load Filter",   ISS_OFF, 0, 0},
  {"i_unload", "Unload Filter", ISS_OFF, 0, 0}
};
ISwitchVectorProperty ifilterSP = {
  GALIL_DEVICE, "IFILTER_ACTIONS", "Instrument Filter Action(s)", IFILTER_GROUP, IP_RW, ISR_1OFMANY, 0.0, IPS_IDLE, ifilterS, NARRAY(ifilterS), "", 0
};

static ISwitch ifilter_changeS[] = {
  {"i_slot_1", "iFilter 1", ISS_OFF, 0, 0},
  {"i_slot_2", "iFilter 2", ISS_OFF, 0, 0},
  {"i_slot_3", "iFilter 3", ISS_OFF, 0, 0},
  {"i_slot_4", "iFilter 4", ISS_OFF, 0, 0},
  {"i_slot_5", "iFilter 5", ISS_OFF, 0, 0},
  {"i_slot_6", "iFilter 6", ISS_OFF, 0, 0}
};
ISwitchVectorProperty ifilter_changeSP = {
  GALIL_DEVICE, "IFILTER_CHANGE", "Instrument Filter(s)", IFILTER_GROUP, IP_RW, ISR_1OFMANY, 0.0, IPS_IDLE, ifilter_changeS, NARRAY(ifilter_changeS), "", 0
};

static IText ifilterT[] = {
  {"ifilter_1", "iFilter 1", ifilter_names.filter_1, 0, 0, 0},
  {"ifilter_2", "iFilter 2", ifilter_names.filter_2, 0, 0, 0},
  {"ifilter_3", "iFilter 3", ifilter_names.filter_3, 0, 0, 0},
  {"ifilter_4", "iFilter 4", ifilter_names.filter_4, 0, 0, 0},
  {"ifilter_5", "iFilter 5", ifilter_names.filter_5, 0, 0, 0},
  {"ifilter_6", "iFilter 6", ifilter_names.filter_6, 0, 0, 0}
};
static ITextVectorProperty ifilterTP = {
  GALIL_DEVICE, "IFILTER_NAMES", "Instrument Filter Name(s)", IFILTER_GROUP, IP_RO, 0.0, IPS_IDLE, ifilterT, NARRAY(ifilterT), "", 0
};

static ISwitch astronomerS[] = {
  {"watchstarted",  "Start of Night", ISS_OFF, 0, 0},
  {"watchended",    "End of Night",   ISS_OFF, 0, 0}
};
ISwitchVectorProperty astronomerSP = {
  GALIL_DEVICE, "ASTRONOMER", "Astronomer Action(s)", IFILTER_GROUP, IP_RW, ISR_ATMOST1, 0.0, IPS_IDLE, astronomerS, NARRAY(astronomerS), "", 0
};

/* IFOCUS_GROUP */
static ISwitch ifocus_referenceS[] = {
  {"savfoc", "Save Reference", ISS_OFF, 0, 0},
  {"resfoc", "Restore Reference", ISS_OFF, 0, 0},
  {"savnom", "Save Nominal Plane", ISS_OFF, 0, 0},
  {"resnom", "Restore Nominal Plane", ISS_OFF, 0, 0}
};
ISwitchVectorProperty ifocus_referenceSP = {
  GALIL_DEVICE, "IFOCUS_REFERENCE", "Instrument Focus Action(s)", IFOCUS_GROUP, IP_RW, ISR_ATMOST1, 0.0, IPS_IDLE, ifocus_referenceS, NARRAY(ifocus_referenceS), "", 0
};

/* these limits came from nowhere so we should verify it when we have time */
static INumber ifocus_distN[] = {
  {"dista", "Encoder A", "%5.0f", -100.0, 100.0, 1.0, 0.0, 0, 0, 0},
  {"distb", "Encoder B", "%5.0f", -100.0, 100.0, 1.0, 0.0, 0, 0, 0},
  {"distc", "Encoder C", "%5.0f", -100.0, 100.0, 1.0, 0.0, 0, 0, 0},
  {"tolerance", "Tolerance", "%5.0f", BOK_MIN_TOLERANCE, BOK_MAX_TOLERANCE, 1.0, BOK_NOM_TOLERANCE, 0, 0, 0}
};
static INumberVectorProperty ifocus_distNP = {
  GALIL_DEVICE, "IFOCUS_DIST", "Instrument Encoder(s)", IFOCUS_GROUP, IP_WO, 0.0, IPS_IDLE, ifocus_distN, NARRAY(ifocus_distN), "", 0
};

/* these limits came from nowhere so we should verify it when we have time */
static INumber ifocus_distallN[] = {
  {"distall", "All Encoders", "%5.0f", -100.0, 100.0, 1.0, 0.0, 0, 0, 0}
};
static INumberVectorProperty ifocus_distallNP = {
  GALIL_DEVICE, "IFOCUS_DISTALL", "Instrument Encoder(s) All", IFOCUS_GROUP, IP_WO, 0.0, IPS_IDLE, ifocus_distallN, NARRAY(ifocus_distallN), "", 0
};

/* documented by Joe and Bruce limit test for LVDT (range -250 to 2850) but we include some buffer but there is an official email */
static INumber ifocus_lvdtN[] = {
  {"lvdta", "Focus A", "%5.0f", BOK_MIN_LVDT, BOK_MAX_LVDT, 1.0, 0.0, 0, 0, 0},
  {"lvdtb", "Focus B", "%5.0f", BOK_MIN_LVDT, BOK_MAX_LVDT, 1.0, 0.0, 0, 0, 0},
  {"lvdtc", "Focus C", "%5.0f", BOK_MIN_LVDT, BOK_MAX_LVDT, 1.0, 0.0, 0, 0, 0},
  {"tolerance", "Tolerance", "%5.0f", BOK_MIN_TOLERANCE, BOK_MAX_TOLERANCE, 1.0, BOK_NOM_TOLERANCE, 0, 0, 0}
};
static INumberVectorProperty ifocus_lvdtNP = {
  GALIL_DEVICE, "IFOCUS_LVDT", "Instrument Foci", IFOCUS_GROUP, IP_RW, 0.0, IPS_IDLE, ifocus_lvdtN, NARRAY(ifocus_lvdtN), "", 0
};

/* these limits came from nowhere so we should verify it when we have time */
static INumber ifocus_lvdtallN[] = {
  {"lvdtall", "Delta Focus All", "%5.0f", -500.0, 500.0, 1.0, 0.0, 0, 0, 0}
};
static INumberVectorProperty ifocus_lvdtallNP = {
  GALIL_DEVICE, "IFOCUS_LVDTALL", "Instrument Foci All", IFOCUS_GROUP, IP_WO, 0.0, IPS_IDLE, ifocus_lvdtallN, NARRAY(ifocus_lvdtallN), "", 0
};

/* SUPPORT_GROUP */
static IText supportT[] = {
  {"author",  "Author",  supports.author,  0, 0, 0},
  {"email",   "Email",   supports.email,   0, 0, 0},
  {"date",    "Date",    supports.date,    0, 0, 0},
  {"version", "Version", supports.version, 0, 0, 0}
};
static ITextVectorProperty supportTP = {
  GALIL_DEVICE, "SUPPORT", "Personnel", SUPPORT_GROUP, IP_RO, 0, IPS_IDLE, supportT, NARRAY(supportT), "", 0
};

/* TELEMETRY_GROUP */
static IText telemetry_referenceT[] = {
  {"a_reference",  "Focus A Reference", telemetrys.a_reference,  0, 0, 0},
  {"b_reference",  "Focus B Reference", telemetrys.b_reference,  0, 0, 0},
  {"c_reference",  "Focus C Reference", telemetrys.c_reference,  0, 0, 0}
};
static ITextVectorProperty telemetry_referenceTP = {
  GALIL_DEVICE, "TELEMETRY_REFERENCE", "Instrument Focus References", TELEMETRY_GROUP, IP_RO, 0, IPS_IDLE, telemetry_referenceT, NARRAY(telemetry_referenceT), "", 0
};

static IText telemetry_lvdtT[] = {
  {"lvdta",     "LVDT A",    telemetrys.lvdta,     0, 0, 0},
  {"lvdtb",     "LVDT B",    telemetrys.lvdtb,     0, 0, 0},
  {"lvdtc",     "LVDT C",    telemetrys.lvdtc,     0, 0, 0},
  {"tolerance", "Tolerance", telemetrys.tolerance, 0, 0, 0}
};
static ITextVectorProperty telemetry_lvdtTP = {
  GALIL_DEVICE, "TELEMETRY_LVDT", "LVDT Telemetry", TELEMETRY_GROUP, IP_RO, 0, IPS_IDLE, telemetry_lvdtT, NARRAY(telemetry_lvdtT), "", 0
};

static IText telemetry_gfocusT[] = {
  {"gfocus_telemetry", "Guider Focus", telemetrys.gfocus_position, 0, 0, 0}
};
static ITextVectorProperty telemetry_gfocusTP = {
  GALIL_DEVICE, "TELEMETRY_GFOCUS", "Guider Focus", GFOCUS_GROUP, IP_RO, 0, IPS_IDLE, telemetry_gfocusT, NARRAY(telemetry_gfocusT), "", 0
};

/* broken up into different groups for display purposes */
static IText telemetryT[] = {
  {"hardware",        "Hardware Version                ", telemetrys.hardware,        0, 0, 0},
  {"software",        "Software Version                ", telemetrys.software,        0, 0, 0},
  {"timestamp",       "Timestamp                       ", telemetrys.timestamp,       0, 0, 0},
  {"julian",          "Julian Date                     ", telemetrys.jd,              0, 0, 0},
  {"tcp_shm_mode",    "TCP (Shared Memory) Mode        ", telemetrys.tcp_shm_mode,    0, 0, 0},
  {"udp_shm_mode",    "UDP (Shared Memory) Mode        ", telemetrys.udp_shm_mode,    0, 0, 0},
  {"a_encoder",       "A Encoder (A axis motor pos)    ", telemetrys.a_encoder,       0, 0, 0},
  {"b_encoder",       "B Encoder (B axis motor pos)    ", telemetrys.b_encoder,       0, 0, 0},
  {"c_encoder",       "C Encoder (C axis motor pos)    ", telemetrys.c_encoder,       0, 0, 0},
  {"a_position",      "A Position (B axis analog in)   ", telemetrys.a_position,      0, 0, 0},
  {"b_position",      "B Position (D axis analog in)   ", telemetrys.b_position,      0, 0, 0},
  {"c_position",      "C Position (F axis analog in)   ", telemetrys.c_position,      0, 0, 0},
  {"a_reference",     "A Focus Reference               ", telemetrys.a_reference,     0, 0, 0},
  {"b_reference",     "B Focus Reference               ", telemetrys.b_reference,     0, 0, 0},
  {"c_reference",     "C Focus Reference               ", telemetrys.c_reference,     0, 0, 0},
  {"lvdta",           "LVDT A                          ", telemetrys.lvdta,           0, 0, 0},
  {"lvdtb",           "LVDT B                          ", telemetrys.lvdtb,           0, 0, 0},
  {"lvdtc",           "LVDT C                          ", telemetrys.lvdtc,           0, 0, 0},
  {"tolerance",       "Tolerance                       ", telemetrys.tolerance,       0, 0, 0},
  {"distall",         "distall                         ", telemetrys.distall,         0, 0, 0},
  {"dista",           "dista                           ", telemetrys.dista,           0, 0, 0},
  {"distb",           "distb                           ", telemetrys.distb,           0, 0, 0},
  {"distc",           "distc                           ", telemetrys.distc,           0, 0, 0},
  {"distgcam",        "distgcam (gfocus)               ", telemetrys.distgcam,        0, 0, 0},
  {"errfilt",         "errfilt (1.0=Error, 0.0=OK)     ", telemetrys.errfilt,         0, 0, 0},
  {"filttsc",         "filttsc (2.0=f/w, 3.0=b/w)      ", telemetrys.filttsc,         0, 0, 0},
  {"filtisin",        "filtisin  (1.0=True, 0.0=False) ", telemetrys.filtisin,        0, 0, 0},
  {"ifilter_1",       "iFilter 1 (filtvals[0])         ", telemetrys.ifilter_1,       0, 0, 0},
  {"ifilter_2",       "iFilter 2 (filtvals[1])         ", telemetrys.ifilter_2,       0, 0, 0},
  {"ifilter_3",       "iFilter 3 (filtvals[2])         ", telemetrys.ifilter_3,       0, 0, 0},
  {"ifilter_4",       "iFilter 4 (filtvals[3])         ", telemetrys.ifilter_4,       0, 0, 0},
  {"ifilter_5",       "iFilter 5 (filtvals[4])         ", telemetrys.ifilter_5,       0, 0, 0},
  {"ifilter_6",       "iFilter 6 (filtvals[5])         ", telemetrys.ifilter_6,       0, 0, 0},
  {"reqfilt",         "iFilter (requested, reqfilt)    ", telemetrys.reqfilt,         0, 0, 0},
  {"filtval",         "iFilter (selected, filtval)     ", telemetrys.filtval,         0, 0, 0},
  {"fnum_in",         "gFilter (requested, fnum_in)    ", telemetrys.fnum_in,         0, 0, 0},
  {"gfiltn",          "gFilter (selected, gfiltn)      ", telemetrys.gfiltn,          0, 0, 0},
  {"snum_in",         "gSensor (requested, snum_in)    ", telemetrys.snum_in,         0, 0, 0},
  {"snum",            "Sensor (selected, snum)         ", telemetrys.snum,            0, 0, 0},
  {"gfilter_1",       "gFilter 1                       ", telemetrys.gfilter_1,       0, 0, 0},
  {"gfilter_2",       "gFilter 2                       ", telemetrys.gfilter_2,       0, 0, 0},
  {"gfilter_3",       "gFilter 3                       ", telemetrys.gfilter_3,       0, 0, 0},
  {"gfilter_4",       "gFilter 4                       ", telemetrys.gfilter_4,       0, 0, 0},
  {"gfilter_5",       "gFilter 5                       ", telemetrys.gfilter_5,       0, 0, 0},
  {"gfilter_6",       "gFilter 6                       ", telemetrys.gfilter_6,       0, 0, 0},
  {"gfocus_position", "gFocus Position                 ", telemetrys.gfocus_position, 0, 0, 0},
  {"gfocus_limit",    "gFocus Limit                    ", telemetrys.gfocus_limit,    0, 0, 0},
};
static ITextVectorProperty telemetryTP = {
  GALIL_DEVICE, "Telemetry", "Telemetry",  TELEMETRY_GROUP, IP_RO, 0, IPS_IDLE, telemetryT, NARRAY(telemetryT), "", 0
};

static ILight telemetry_ifilterwheelL[] = {
  {"fout", "Unloaded",    ISS_OFF, 0, 0},
  {"fin",  "Loaded",      ISS_OFF, 0, 0},
  {"frot", "Rotating",    ISS_OFF, 0, 0},
  {"flin", "Translating", ISS_OFF, 0, 0},
  {"ferr", "Error",       ISS_OFF, 0, 0},
  {"flim", "Limit",       ISS_OFF, 0, 0}
};
ILightVectorProperty telemetry_ifilterwheelLP = {
  GALIL_DEVICE, "FW_LIGHTS", "Instrument Filter Status", TELEMETRY_GROUP, IPS_IDLE, telemetry_ifilterwheelL, NARRAY(telemetry_ifilterwheelL), "", 0
};

static ILight telemetry_glimitsL[] = {
  {"glimitin",  "Limit In (+)",  ISS_OFF, 0, 0},
  {"glimitout", "Limit Out (-)", ISS_OFF, 0, 0}
};
ILightVectorProperty telemetry_glimitsLP = {
  GALIL_DEVICE, "GLIMITS_LIGHTS", "Guider Focus Limits", GFOCUS_GROUP, IPS_IDLE, telemetry_glimitsL, NARRAY(telemetry_glimitsL), "", 0
};

static ILight telemetry_gfilterwheelL[] = {
  {"grot", "Rotating", ISS_OFF, 0, 0}
};
ILightVectorProperty telemetry_gfilterwheelLP = {
  GALIL_DEVICE, "GW_LIGHTS", "Guider Filter Status", TELEMETRY_GROUP, IPS_IDLE, telemetry_gfilterwheelL, NARRAY(telemetry_gfilterwheelL), "", 0
};

static ILight telemetry_engineeringL[] = {
  {"tcp",     "TCP Shared Memory", ISS_OFF, 0, 0},
  {"udp",     "UDP Shared Memory", ISS_OFF, 0, 0}
};
ILightVectorProperty telemetry_engineeringLP = {
  GALIL_DEVICE, "ENGINEERING_LIGHTS", "Memory Segments", TELEMETRY_GROUP, IPS_IDLE, telemetry_engineeringL, NARRAY(telemetry_engineeringL), "", 0
};

static ILight telemetry_lightsL[] = {
  {"swbusy", "Software Busy",  ISS_OFF, 0, 0},
  {"hwbusy", "Hardware Busy",  ISS_OFF, 0, 0},
  {"popbusy", "Populate Busy", ISS_OFF, 0, 0}
};
ILightVectorProperty telemetry_lightsLP = {
  GALIL_DEVICE, "TELEMETRY", "System Status", TELEMETRY_GROUP, IPS_IDLE, telemetry_lightsL, NARRAY(telemetry_lightsL), "", 0
};


/*******************************************************************************
 * INDI hook: ISGetProperties()
 ******************************************************************************/
void ISGetProperties(const char *dev) {

  /* check device */
  if (dev && strcmp(GALIL_DEVICE, dev)) return;

  /* define widget(s) */
  IDDefLight(&telemetry_engineeringLP, NULL);
  IDDefLight(&telemetry_gfilterwheelLP, NULL);
  IDDefLight(&telemetry_lightsLP, NULL);
  IDDefLight(&telemetry_glimitsLP, NULL);
  IDDefLight(&telemetry_ifilterwheelLP, NULL);
  IDDefText(&telemetry_referenceTP, NULL);
  IDDefText(&telemetry_gfocusTP, NULL);
  IDDefText(&telemetry_lvdtTP, NULL);
  IDDefText(&telemetryTP, NULL);
  IDDefSwitch(&ifilter_engineeringSP, NULL);
  IDDefSwitch(&ifilterSP, NULL);
  IDDefSwitch(&ifilter_changeSP, NULL);
  IDDefSwitch(&astronomerSP, NULL);
  IDDefText(&ifilterTP, NULL);
  IDDefSwitch(&ifocus_referenceSP, NULL);
  IDDefNumber(&ifocus_distNP, NULL);
  IDDefNumber(&ifocus_distallNP, NULL);
  IDDefNumber(&ifocus_lvdtNP, NULL);
  IDDefNumber(&ifocus_lvdtallNP, NULL);
  IDDefSwitch(&gfilterSP, NULL);
  IDDefSwitch(&gfilter_changeSP, NULL);
  IDDefText(&gfilterTP, NULL);
  IDDefNumber(&gfocus_distNP, NULL);
  IDDefText(&supportTP, NULL);

  /* initialize as required */
  if (initialized == false) { driver_init(); }
}


/*******************************************************************************
 * INDI hook: ISNewBLOB() - required but not used by this code
 ******************************************************************************/
void ISNewBLOB(const char *dev, const char *name, int sizes[], int blobsizes[],
               char *blobs[], char *formats[], char *names[], int n) {
  if (dev && strcmp(GALIL_DEVICE, dev)) return;
  return;
}


/*******************************************************************************
 * INDI hook: ISNewLight() - required but not used by this code
 ******************************************************************************/
void ISNewLight(const char *dev, const char *name, ISState *states, char *names[], int n) {
  if (dev && strcmp(GALIL_DEVICE, dev)) return;
  return;
}


/*******************************************************************************
 * INDI hook: ISNewNumber()
 ******************************************************************************/
void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) {

  /* declare some variables and initialize them */
  GReturn gstat = (GReturn)0;

  /* check device */
  if (dev && strcmp(GALIL_DEVICE, dev)) return;

  /* dump value(s) received */
  for (int idx=0; idx<n; idx++) {
    IDMessage(GALIL_DEVICE, "dump name='%s', value[%d]=%.3f", name, idx, values[idx]);
  }

  /* focus dist value(s) - what uses this?? */
  if (!strcmp(name, ifocus_distNP.name)) {
    IDMessage(GALIL_DEVICE, "Calling ifocus_distNP from '%s'", name);
    float dista = values[0];
    float distb = values[1];
    float distc = values[2];

    /* talk to hardware */
    busy = true;
    telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
    IDSetLight(&telemetry_lightsLP, NULL);
    ifocus_distNP.s = IPS_BUSY;
    IDMessage(GALIL_DEVICE, "Calling xq_hx() from '%s'", "ifocus dist");
    if ((gstat=xq_hx()) == G_NO_ERROR) {
      IDMessage(GALIL_DEVICE, "Called xq_hx() from '%s' OK", "ifocus dist");
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_hx() from '%s', gstat=%d", "ifocus dist", (int)gstat);
    }
    IDMessage(GALIL_DEVICE, "Calling xq_focusind(a=%.1f, b=%.1f, c=%.1f) from '%s'", dista, distb, distc, "ifocus dist");
    if ((gstat=xq_focusind(dista, distb, distc)) == G_NO_ERROR) {
      IDMessage(GALIL_DEVICE, "Called xq_focusind(a=%.1f, b=%.1f, c=%.1f) from '%s' OK", dista, distb, distc, "ifocus dist");
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_focusind(a=%.1f, b=%.1f, c=%.1f) from '%s', gstat=%d", dista, distb, distc, "ifocus dist", (int)gstat);
    }
    ifocus_distNP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
    busy = false;
    telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
    IDSetLight(&telemetry_lightsLP, NULL);

    /* update widget(s) */
    ifocus_distNP.np[0].value = dista;
    ifocus_distNP.np[1].value = distb;
    ifocus_distNP.np[2].value = distc;
    IDSetNumber(&ifocus_distNP, NULL);

  /* focus distall value(s) - what uses this?? */
  } else if (!strcmp(name, ifocus_distallNP.name)) {
    IDMessage(GALIL_DEVICE, "Calling ifocus_distallNP from '%s'", name);
    float distall = values[0];

    /* talk to hardware */
    busy = true;
    telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
    IDSetLight(&telemetry_lightsLP, NULL);
    ifocus_distallNP.s = IPS_BUSY;
    IDMessage(GALIL_DEVICE, "Calling xq_hx() from '%s'", "ifocus distall");
    if ((gstat=xq_hx()) == G_NO_ERROR) {
      IDMessage(GALIL_DEVICE, "Called xq_hx() from '%s' OK", "ifocus distall");
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_hx() from '%s', gstat=%d", "ifocus distall", (int)gstat);
    }
    IDMessage(GALIL_DEVICE, "Calling xq_focusall(a=%.1f)", distall);
    if ((gstat=xq_focusall(distall)) == G_NO_ERROR) {
      IDMessage(GALIL_DEVICE, "Called xq_focusall(a=%.1f) OK", distall);
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_focusall(a=%.1f), gstat=%d", distall, (int)gstat);
    }
    /* there is no way to check completion so we just wait a couple seconds */
    (void) sleep(2);
    ifocus_distallNP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
    busy = false;
    telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
    IDSetLight(&telemetry_lightsLP, NULL);

    /* update widget(s) */
    ifocus_distallNP.np[0].value = distall;
    IDSetNumber(&ifocus_distallNP, NULL);

  /* focus lvdt value(s) - this is the main interface in the pyINDI gui */
  } else if (!strcmp(name, ifocus_lvdtNP.name)) {
    IDMessage(GALIL_DEVICE, "Calling ifocus_lvdtNP from '%s'", name);
    IDMessage(GALIL_DEVICE, "Calling ifocus_lvdtNP values[0]=%.4f values[1]=%.4f values[2]=%.4f values[3]=%.4f", values[0], values[1], values[2], values[3]);

    float dista = 0.0;
    float distb = 0.0;
    float distc = 0.0;
    float focus_a = 0.0;
    float focus_b = 0.0;
    float focus_c = 0.0;
    float new_a = 0.0;
    float new_b = 0.0;
    float new_c = 0.0;
    float tolerance = ifoci.tolerance;

    /* we are setting 1 value, here, not all 3 */
    if (n == 1) {
      IDMessage(GALIL_DEVICE, "Moving only %s", names[0]);
      
      /* so which property are we setting? */
      if (!strcmp(names[0], "lvdta")) {
        if (abs(values[0] - ifoci.vala * 1000.0) > BOK_MAX_LVDT_DIFF) { 
          IDMessage(GALIL_DEVICE, "<ERROR> lvdta input value differs more than %.0f units", BOK_MAX_LVDT_DIFF);
          return;
        }
        dista = round((values[0] / 1000.0 - ifoci.vala) * BOK_LVDT_ATOD);
        distb = 0.0;
        distc = 0.0;
        new_a = values[0];
        new_b = 0.0;
        new_c = 0.0;
        focus_a = values[0];
        focus_b = 0.0;
        focus_c = 0.0;
      } else if (!strcmp(names[0], "lvdtb")) {
        if (abs(values[0] - ifoci.valb * 1000.0) > BOK_MAX_LVDT_DIFF) { 
          IDMessage(GALIL_DEVICE, "<ERROR> lvdtb input value differs more than %.0f units", BOK_MAX_LVDT_DIFF);
          return;
        }
        dista = 0.0;
        distb = round((values[0] / 1000.0 - ifoci.valb) * BOK_LVDT_ATOD);
        distc = 0.0;
        new_a = 0.0;
        new_b = values[0];
        new_c = 0.0;
        focus_a = 0.0;
        focus_b = values[0];
        focus_c = 0.0;
      } else if (!strcmp(names[0], "lvdtc")) {
        if (abs(values[0] - ifoci.valc * 1000.0) > BOK_MAX_LVDT_DIFF) { 
          IDMessage(GALIL_DEVICE, "<ERROR> lvdtc input value differs more than %.0f units", BOK_MAX_LVDT_DIFF);
          return;
        }
        dista = 0.0;
        distb = 0.0;
        distc = round((values[0] / 1000.0 - ifoci.valc) * BOK_LVDT_ATOD);
        new_a = 0.0;
        new_b = 0.0;
        new_c = values[0];
        focus_a = 0.0;
        focus_b = 0.0;
        focus_c = values[0];
      } else if (!strcmp(names[0], "tolerance")) {
        tolerance = abs(values[0]);
        if (tolerance<BOK_MIN_TOLERANCE || tolerance>BOK_MAX_TOLERANCE) { tolerance = BOK_NOM_TOLERANCE; }
        IDMessage(GALIL_DEVICE, "changing tolerance from %.4f to %.4f", ifoci.tolerance, tolerance);
        ifoci.tolerance = tolerance;
        return;
      }

    /* we are setting all 3 values, here, not just 1 */
    } else {
      if (abs(values[0] - ifoci.vala * 1000.0) > BOK_MAX_LVDT_DIFF || 
          abs(values[1] - ifoci.valb * 1000.0) > BOK_MAX_LVDT_DIFF || 
          abs(values[2] - ifoci.valc * 1000.0) > BOK_MAX_LVDT_DIFF) {
        IDMessage(GALIL_DEVICE, "<ERROR> lvdt inputs values differ more than %.0f units", BOK_MAX_LVDT_DIFF);
        return;
      }
      dista = round((values[0] / 1000.0 - ifoci.vala) * BOK_LVDT_ATOD);
      distb = round((values[1] / 1000.0 - ifoci.valb) * BOK_LVDT_ATOD);
      distc = round((values[2] / 1000.0 - ifoci.valc) * BOK_LVDT_ATOD);
      new_a = values[0];
      new_b = values[1];
      new_c = values[2];
      focus_a = values[0];
      focus_b = values[1];
      focus_c = values[2];
      tolerance = abs(values[3]);
      if (tolerance<BOK_MIN_TOLERANCE || tolerance>BOK_MAX_TOLERANCE) { tolerance = BOK_NOM_TOLERANCE; }
      ifoci.tolerance = tolerance;
    }

    /* talk to hardware */
    busy = true;
    telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
    IDSetLight(&telemetry_lightsLP, NULL);
    ifocus_lvdtNP.s = IPS_BUSY;
    is_done = false;

    /* talk to hardware */
    IDMessage(GALIL_DEVICE, "main ifocus pre-entry dista=%.4f new_a=%.4f focus_a=%.4f distb=%.4f new_b=%.4f focus_b=%.4f distc=%.4f new_c=%.4f focus_c=%.4f tolerance=%.4f", dista, new_a, focus_a, distb, new_b, focus_b, distc, new_c, focus_c, tolerance);
    int countdown = BOK_NG_INSTRUMENT_FOCUS_TIME;
    while (--countdown > 0) {

      /* get fresh telemetry */
      unscheduled_telemetry((void *)NULL);
      float cur_a = round(((float)udp_val.a_position * 1000.0));
      float cur_b = round(((float)udp_val.b_position * 1000.0));
      float cur_c = round(((float)udp_val.c_position * 1000.0));
      dista = round((focus_a/1000.0 - cur_a/1000.0) * BOK_LVDT_ATOD);
      distb = round((focus_b/1000.0 - cur_b/1000.0) * BOK_LVDT_ATOD);
      distc = round((focus_c/1000.0 - cur_c/1000.0) * BOK_LVDT_ATOD);
      /*
      float new_a = values[0];
      float new_b = values[1];
      float new_c = values[2];
      */
      // if (tolerance<BOK_MIN_TOLERANCE || tolerance>BOK_MAX_TOLERANCE) { tolerance = BOK_NOM_TOLERANCE; }
      IDMessage(GALIL_DEVICE, "main ifocus cur_a=%.4f dista=%.4f focus_a=%.4f new_a=%.4f tolerance=%.4f diff=%.4f timestamp='%s' countdown=%d", cur_a, dista, focus_a, new_a, tolerance, (cur_a - new_a), tcp_val.timestamp, countdown);
      IDMessage(GALIL_DEVICE, "main ifocus cur_b=%.4f distb=%.4f focus_b=%.4f new_b=%.4f tolerance=%.4f diff=%.4f timestamp='%s' countdown=%d", cur_b, distb, focus_b, new_b, tolerance, (cur_b - new_b), tcp_val.timestamp, countdown);
      IDMessage(GALIL_DEVICE, "main ifocus cur_c=%.4f distc=%.4f focus_c=%.4f new_c=%.4f tolerance=%.4f diff=%.4f timestamp='%s' countdown=%d", cur_c, distc, focus_c, new_c, tolerance, (cur_c - new_c), tcp_val.timestamp, countdown);

      /* end if within tolerance */
      if ( (abs(round(cur_a - new_a)) <= tolerance) && (abs(round(cur_b - new_b)) <= tolerance) && (abs(round(cur_c - new_c)) <= tolerance) ) { is_done = true; break; }

      /* stop current execution */
      if ((gstat=xq_hx()) == G_NO_ERROR) {
        IDMessage(GALIL_DEVICE, "Called xq_hx() from '%s' OK", "ifocus lvdt");
      } else {
        IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_hx() from '%s', gstat=%d", "ifocus lvdt", (int)gstat);
      }

      /* start new drive */
      if ((gstat=xq_focusind(dista, distb, distc)) == G_NO_ERROR) {
        IDMessage(GALIL_DEVICE, "Called xq_focusind(a=%.1f, b=%.1f, c=%.1f) from '%s' with tolerance %.2f OK", dista, distb, distc, "ifocus lvdt", tolerance);
      } else {
        IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_focusind(a=%.1f, b=%.1f, c=%.1f) from '%s' with tolerance %.2f, gstat=%d", dista, distb, distc, "ifocus lvdt", tolerance, (int)gstat);
      }
      (void) sleep(1);
    }
    if (is_done) {
      IDMessage(GALIL_DEVICE, "main ifocus reached focus_a=%.4f focus_b=%.4f focus_c=%.4f within tolerance=%.4f OK", focus_a, focus_b, focus_c, tolerance);
      ifocus_lvdtNP.s = IPS_OK;
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> main ifocus failed to reached focus_a=%.4f focus_b=%.4f focus_c=%.4f within tolerance=%.4f, countdown=%d OK", focus_a, focus_b, focus_c, tolerance, countdown);
      ifocus_lvdtNP.s = IPS_ALERT;
    }
    if ((gstat=xq_hx()) == G_NO_ERROR) {
      IDMessage(GALIL_DEVICE, "Called xq_hx() from '%s' OK", "ifocus lvdt");
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_hx() from '%s', gstat=%d", "ifocus lvdt", (int)gstat);
    }
    busy = false;
    telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
    IDSetLight(&telemetry_lightsLP, NULL);
    IDSetNumber(&ifocus_lvdtNP, NULL);
    
  /* focus lvdtall value - this is the relative movement so all motors will step by the same amount on the pyINDI gui */
  } else if (!strcmp(name, ifocus_lvdtallNP.name)) {
    IDMessage(GALIL_DEVICE, "Calling ifocus_lvdtallNP from '%s'", name);
    float distall = round((values[0] / 1000.0) * BOK_LVDT_ATOD);

    IDMessage(GALIL_DEVICE, "lvdt input values all=%.1f", values[0]);
    IDMessage(GALIL_DEVICE, "lvdt input values distall=%.1f", distall);

    /* talk to hardware */
    busy = true;
    telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
    IDSetLight(&telemetry_lightsLP, NULL);
    ifocus_lvdtallNP.s = IPS_BUSY;
    IDMessage(GALIL_DEVICE, "Calling xq_hx() from '%s'", "ifocus lvdtall");
    if ((gstat=xq_hx()) == G_NO_ERROR) {
      IDMessage(GALIL_DEVICE, "Called xq_hx() from '%s' OK", "ifocus lvdtall");
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_hx() from '%s', gstat=%d", "ifocus lvdtall", (int)gstat);
    }
    IDMessage(GALIL_DEVICE, "Calling xq_focusall(a=%.1f) from '%s'", distall, "ifocus lvdtall");
    if ((gstat=xq_focusall(distall)) == G_NO_ERROR) {
      IDMessage(GALIL_DEVICE, "Called xq_focusall(a=%.1f) from '%s' OK", distall, "ifocus lvdtall");
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_focusall(a=%.1f) from '%s', gstat=%d", distall, "ifocus lvdtall", (int)gstat);
    }
    /* there is no way to check completion so we just wait a couple seconds */
    (void) sleep(2);
    ifocus_lvdtallNP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
    busy = false; 
    telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
    IDSetLight(&telemetry_lightsLP, NULL);
    IDSetNumber(&ifocus_lvdtallNP, NULL);

  /* gfocus dist value */
  } else if (!strcmp(name, gfocus_distNP.name)) {
    IDMessage(GALIL_DEVICE, "Calling gfocus_distNP from '%s'", name);
    float distgcam = values[0];

    /* talk to hardware */
    busy = true;
    telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
    IDSetLight(&telemetry_lightsLP, NULL);
    gfocus_distNP.s = IPS_BUSY;
    IDMessage(GALIL_DEVICE, "Calling xq_hx() from '%s'", "gfocus dist");
    if ((gstat=xq_hx()) == G_NO_ERROR) {
      IDMessage(GALIL_DEVICE, "Called xq_hx() from '%s' OK", "gfocus dist");
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_hx() from '%s', gstat=%d", "gfocus dist", (int)gstat);
    }
    IDMessage(GALIL_DEVICE, "Calling xq_gfocus(a=%.1f)", distgcam);
    if ((gstat=xq_gfocus(distgcam)) == G_NO_ERROR) {
      IDMessage(GALIL_DEVICE, "Called xq_gfocus(a=%.1f) OK", distgcam);
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_gfocus(a=%.1f), gstat=%d", distgcam, (int)gstat);
    }
    /* there is no way to check completion so we just wait a couple seconds */
    (void) sleep(2);
    gfocus_distNP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
    busy = false;
    telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
    IDSetLight(&telemetry_lightsLP, NULL);

    /* update widget(s) */
    IDSetNumber(&gfocus_distNP, NULL);
  }
}


/*******************************************************************************
 * INDI hook: ISSnoopDevice() - required but not used by this code
 ******************************************************************************/
void ISSnoopDevice(XMLEle *root) {
  return;
}


/*******************************************************************************
 * INDI hook: ISNewSwitch()
 ******************************************************************************/
void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) {

  /* check device */
  if (dev && strcmp(GALIL_DEVICE, dev)) return;

  /* which switch was pressed? */
  if (! strcmp(name, ifilter_engineeringSP.name)) {
    execute_ifilter_engineering(states, names, n);
    IUResetSwitch(&ifilter_engineeringSP);
    IDSetSwitch(&ifilter_engineeringSP, NULL);
  } else if (! strcmp(name, ifilterSP.name)) {
    execute_ifilter_switches(states, names, n);
    IUResetSwitch(&ifilterSP);
    IDSetSwitch(&ifilterSP, NULL);
  } else if (! strcmp(name, ifilter_changeSP.name)) {
    execute_ifilter_change(states, names, n);
    IUResetSwitch(&ifilter_changeSP);
    IDSetSwitch(&ifilter_changeSP, NULL);
  } else if (! strcmp(name, gfilterSP.name)) {
    execute_gfilter_switches(states, names, n);
    IUResetSwitch(&gfilterSP);
    IDSetSwitch(&gfilterSP, NULL);
  } else if (! strcmp(name, gfilter_changeSP.name)) {
    execute_gfilter_change(states, names, n);
    IUResetSwitch(&gfilter_changeSP);
    IDSetSwitch(&gfilter_changeSP, NULL);
  } else if (! strcmp(name, ifocus_referenceSP.name)) {
    execute_ifocus_reference_switches(states, names, n);
    IUResetSwitch(&ifocus_referenceSP);
    IDSetSwitch(&ifocus_referenceSP, NULL);
  } else if (! strcmp(name, astronomerSP.name)) {
    execute_astronomer_actions(states, names, n);
    IUResetSwitch(&astronomerSP);
    IDSetSwitch(&astronomerSP, NULL);
  }
}


/*******************************************************************************
 * indi hook: ISNewText() - required but not used by this code
 ******************************************************************************/
void ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n) {
  if (dev && strcmp(GALIL_DEVICE, dev)) return;
  return;
}


/*******************************************************************************
 * action: driver_init()
 ******************************************************************************/
static void driver_init(void) {

  /* declare some variables and initialize them */
  char *gpath = (char *)NULL;
  char *ipath = (char *)NULL;
  char *spath = (char *)NULL;
  char gname[BOK_STR_1024] = {'\0'};
  char iname[BOK_STR_1024] = {'\0'};
  char sname[BOK_STR_1024] = {'\0'};

  /* check status */
  if (initialized == true) { return; }

  /* get guider filters file */
  (void) memset((void *)&gname, '\0', sizeof(gname));
  if ((gpath=getenv("BOK_GALIL_DOCS")) == (char *)NULL) {
    (void) sprintf(gname, "%s", BOK_GFILTER_FILE);
  } else {
    (void) sprintf(gname, "%s/%s", gpath, BOK_GFILTER_FILE);
  }

  /* get instrument filters file */
  (void) memset((void *)&iname, '\0', sizeof(iname));
  if ((ipath=getenv("BOK_GALIL_DOCS")) == (char *)NULL) {
    (void) sprintf(iname, "%s", BOK_IFILTER_FILE);
  } else {
    (void) sprintf(iname, "%s/%s", ipath, BOK_IFILTER_FILE);
  }

  /* get sensor filters file */
  (void) memset((void *)&sname, '\0', sizeof(sname));
  if ((spath=getenv("BOK_GALIL_DOCS")) == (char *)NULL) {
    (void) sprintf(sname, "%s", BOK_SFILTER_FILE);
  } else {
    (void) sprintf(sname, "%s/%s", spath, BOK_SFILTER_FILE);
  }

  /* output some message(s) */
  IDMessage(GALIL_DEVICE, "Initializing driver v%s, %s (%s), %s", _VERSION_, _AUTHOR_, _EMAIL_, _DATE_);
  IDMessage(GALIL_DEVICE, "Guider filter file %s", gname);
  IDMessage(GALIL_DEVICE, "Instrument filter file %s", iname);
  IDMessage(GALIL_DEVICE, "Sensor filter file %s", sname);

  /* read guider filter list from file */
  for (int j=0; j<BOK_GFILTER_SLOTS; j++) { (void) memset((void *)&bok_gfilters[j], 0, sizeof(filter_file_t)); }
  read_filters_from_file(gname, (filter_file_t *)bok_gfilters, BOK_GFILTER_SLOTS, BOK_GFILTER_COLUMNS);
  for (int j=0; j<BOK_GFILTER_SLOTS; j++) {
    if (strlen(bok_gfilters[j].code)>0 && strlen(bok_gfilters[j].name)>0) {
      (void) fprintf(stderr, "guider filters> index=%d, code='%s', name='%s'", j, bok_gfilters[j].code, bok_gfilters[j].name);
      (void) fflush(stderr);
    }
  }

  /* read instrument filter list from file */
  for (int i=0; i<BOK_IFILTER_SLOTS; i++) { (void) memset((void *)&bok_ifilters[i], 0, sizeof(filter_file_t)); }
  read_filters_from_file(iname, (filter_file_t *)bok_ifilters, BOK_IFILTER_SLOTS, BOK_IFILTER_COLUMNS);
  for (int j=0; j<BOK_IFILTER_SLOTS; j++) {
    if (strlen(bok_ifilters[j].code)>0 && strlen(bok_ifilters[j].name)>0) {
      (void) fprintf(stderr, "instrument filters> index=%d, code='%s', name='%s'", j, bok_ifilters[j].code, bok_ifilters[j].name);
      (void) fflush(stderr);
    }
  }

  /* read sensor filter list from file */
  for (int k=0; k<BOK_SFILTER_SLOTS; k++) { (void) memset((void *)&bok_sfilters[k], 0, sizeof(filter_file_t)); }
  read_filters_from_file(sname, (filter_file_t *)bok_sfilters, BOK_SFILTER_SLOTS, BOK_SFILTER_COLUMNS);
  for (int k=0; k<BOK_SFILTER_SLOTS; k++) {
    if (strlen(bok_sfilters[k].code)>0 && strlen(bok_sfilters[k].name)>0) {
      (void) fprintf(stderr, "sensor filters> index=%d, code='%s', name='%s'", k, bok_sfilters[k].code, bok_sfilters[k].name);
      (void) fflush(stderr);
    }
  }

  /* zero out telemetry values */
  zero_telemetry();

  /* add a background timer to wakeup the named function */
  if (timer_id < 0) {
    IDMessage(GALIL_DEVICE, "adding scheduled_telemetry(), timer_id=%d, initialized=%d", timer_id, initialized);
    if ((timer_id=IEAddTimer(BOK_TCP_DELAY_MS, scheduled_telemetry, (void *)NULL)) >= 0) {
      initialized = true;
      IDMessage(GALIL_DEVICE, "added scheduled_telemetry(), timer_id=%d, initialized=%d", timer_id, initialized);
    }
  }
}


/*******************************************************************************
 * action: astronomer_action_doit()
 ******************************************************************************/
GReturn astronomer_action_doit(bool startofnight) {
  if (startofnight) { 
    (void) ifilter_unload_doit();
    return ifilter_read_doit();
  } else {
    return ifilter_unload_doit();
  }
}


/*******************************************************************************
 * action: execute_astronomer_actions()
 ******************************************************************************/
void execute_astronomer_actions(ISState states[], char *names[], int n) {

  /* declare some variable(s) and initialize them */
  GReturn gstat = (GReturn)0;
  ISwitch *sp = (ISwitch *)NULL;
  ISState state = (ISState)NULL;
  bool state_change = false;

  /* find switches with the passed names in the astronomerSP property */
  for (int i=0; i < n; i++) {
    sp = IUFindSwitch(&astronomerSP, names[i]);
    state = states[i];
    state_change = state != sp->s;
    if (! state_change) { continue; }

    /* start of night task(s) */
    if (sp == &astronomerS[0]) {
      gstat = astronomer_action_doit(true);
      astronomerSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
      astronomerS[0].s = ISS_OFF;
      IDMessage(GALIL_DEVICE, "watch has started, da iawn");

    /* end of night task(s) */
    } else if (sp == &astronomerS[1]) {
      gstat = astronomer_action_doit(false);
      astronomerSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
      astronomerS[1].s = ISS_OFF;
      IDMessage(GALIL_DEVICE, "watch has ended, nos da");
    }
  }

  /* reset */
  IUResetSwitch(&astronomerSP);
  IDSetSwitch(&astronomerSP, NULL);
}


/*******************************************************************************
 * action: gfilter_change_doit()
 ******************************************************************************/
void gfilter_change_doit(float gfiltnum) {
  GReturn gstat = (GReturn)0;
  if (tcp_val.lv.gfiltn == gfiltnum) {
    IDMessage(GALIL_DEVICE, "gFilter %d is already selected", (int)gfiltnum);
    gfilter_changeSP.s = IPS_OK;
  } else {
    busy = true;
    telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
    IDSetLight(&telemetry_lightsLP, NULL);
    gfilter_changeSP.s = IPS_BUSY;
    IDMessage(GALIL_DEVICE, "Calling xq_gfiltn(%d)", (int)gfiltnum);
    if ((gstat=xq_gfiltn(gfiltnum)) == G_NO_ERROR) {
      IDMessage(GALIL_DEVICE, "Called xq_gfiltn(%d) OK", (int)gfiltnum);
      IDMessage(GALIL_DEVICE, "Calling xq_gfwmov()");
      if ((gstat=xq_gfwmov()) == G_NO_ERROR) {
        IDMessage(GALIL_DEVICE, "Called xq_gfwmov() OK");
      } else {
        IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_gfwmov(), gstat=%d", (int)gstat);
      }
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_gfiltn(1), gstat=%d", (int)gstat);
    }
    gfilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
    busy = false;
    telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
    IDSetLight(&telemetry_lightsLP, NULL);
  }
  gfilter_changeS[0].s = ISS_OFF;
}


/*******************************************************************************
 * action: execute_gfilter_change()
 ******************************************************************************/
void execute_gfilter_change(ISState states[], char *names[], int n) {
  /* declare some variables and initialize them */
  ISwitch *sp = (ISwitch *)NULL;
  ISState state = (ISState)NULL;
  bool state_change = false;

  /* find switches with the passed names in the gfilter_changeSP property */
  for (int i=0; i < n; i++) {
    sp = IUFindSwitch(&gfilter_changeSP, names[i]);
    state = states[i];
    state_change = state != sp->s;
    
    if (! state_change) { continue; }

    /* process 'gFilter 1' */
    if (sp == &gfilter_changeS[0]) {
      (void) gfilter_change_doit(1.0);

    /* process 'gFilter 2' */
    } else if (sp == &gfilter_changeS[1]) {
      (void) gfilter_change_doit(2.0);

    /* process 'gFilter 3' */
    } else if (sp == &gfilter_changeS[2]) {
      (void) gfilter_change_doit(3.0);

    /* process 'gFilter 4' */
    } else if (sp == &gfilter_changeS[3]) {
      (void) gfilter_change_doit(4.0);

    /* process 'gFilter 5' */
    } else if (sp == &gfilter_changeS[4]) {
      (void) gfilter_change_doit(5.0);

    /* process 'gFilter 6' */
    } else if (sp == &gfilter_changeS[5]) {
      (void) gfilter_change_doit(6.0);

/*
      if (tcp_val.lv.gfiltn == 6.0) {
        IDMessage(GALIL_DEVICE, "gFilter 6 is already selected");
        gfilter_changeSP.s = IPS_OK;
      } else {
        busy = true;
        telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
        IDSetLight(&telemetry_lightsLP, NULL);
        gfilter_changeSP.s = IPS_BUSY;
        IDMessage(GALIL_DEVICE, "Calling xq_gfiltn(6)");
        if ((gstat=xq_gfiltn(6.0)) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_gfiltn(6) OK");
          IDMessage(GALIL_DEVICE, "Calling xq_gfwmov()");
          if ((gstat=xq_gfwmov()) == G_NO_ERROR) {
            IDMessage(GALIL_DEVICE, "Called xq_gfwmov() OK");
          } else {
            IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_gfwmov(), gstat=%d", (int)gstat);
          }
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_gfiltn(6), gstat=%d", (int)gstat);
        }
        gfilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        busy = false;
        telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
        IDSetLight(&telemetry_lightsLP, NULL);
      }
      gfilter_changeS[5].s = ISS_OFF;
*/\

    }
  }

  /* reset */
  IUResetSwitch(&gfilter_changeSP);
  IDSetSwitch(&gfilter_changeSP, NULL);
}


/*******************************************************************************
 * action: execute_gfilter_switches()
 ******************************************************************************/
void execute_gfilter_switches(ISState states[], char *names[], int n) {

  /* declare some variables and initialize them */
  GReturn gstat = (GReturn)0;
  ISwitch *sp = (ISwitch *)NULL;
  ISState state = (ISState)NULL;
  bool state_change = false;

  /* find switches with the passed names in the gfilterSP property */
  for (int i=0; i < n; i++) {
    sp = IUFindSwitch(&gfilterSP, names[i]);
    state = states[i];
    state_change = state != sp->s;
    if (! state_change) { continue; }

    /* process ginitfw */
    if (sp == &gfilterS[0]) {

      /* talk to hardware */
      busy = true;
      telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
      IDSetLight(&telemetry_lightsLP, NULL);
      gfilterSP.s = IPS_BUSY;
      IDMessage(GALIL_DEVICE, "Calling xq_gfwinit()");
      if ((gstat=xq_gfwinit()) == G_NO_ERROR) {
        IDMessage(GALIL_DEVICE, "Called xq_gfwinit() OK");
      } else {
        IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_gfwinit(), gstat=%d", (int)gstat);
      }
      gfilterSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
      busy = false;
      telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
      IDSetLight(&telemetry_lightsLP, NULL);
      gfilterS[0].s = ISS_OFF;
    }
  }

  /* reset */
  IUResetSwitch(&gfilterSP);
  IDSetSwitch(&gfilterSP, NULL);
}


/*******************************************************************************
 * action: execute_ifilter_engineering()
 ******************************************************************************/
void execute_ifilter_engineering(ISState states[], char *names[], int n) {
  /* declare some variables and initialize them */
  GReturn gstat = (GReturn)0;
  ISwitch *sp = (ISwitch *)NULL;
  ISState state = (ISState)NULL;
  bool state_change = false;

  for (int i=0; i < n; i++) {
    sp = IUFindSwitch(&ifilter_engineeringSP, names[i]);
    state = states[i];
    state_change = state != sp->s;
    if (! state_change) { continue; }

    /* process 'Populate' */
    if (sp == &ifilter_engineeringS[0]) {
      if (tcp_val.lv.filtisin == 1.0) {
        (void) dump_tcp_structure(&tcp_val);
        IDMessage(GALIL_DEVICE, "Cannot populate whilst the filter is in the beam");
        ifilter_engineeringSP.s = IPS_OK;
      } else {

        /* talk to hardware */
        busy = true;
        popact = true;
        telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
        telemetry_lightsL[1].s = IPS_BUSY;
        telemetry_lightsL[2].s = (popact == true) ? IPS_BUSY : IPS_IDLE;
        IDSetLight(&telemetry_lightsLP, NULL);
        ifilter_engineeringSP.s = IPS_BUSY;
        IDMessage(GALIL_DEVICE, "Calling xq_filtldm() from 'populate'");
        if ((gstat=xq_filtldm()) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_filtldm() from 'populate' OK");
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_filtldm() from 'populate', gstat=%d", (int)gstat);
          busy = false;
          telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
          telemetry_lightsL[1].s = IPS_IDLE;
          telemetry_lightsL[2].s = IPS_IDLE;
          IDSetLight(&telemetry_lightsLP, NULL);
        }
        ifilter_engineeringSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;

      }
      ifilter_engineeringS[0].s = ISS_OFF;

    /* process 'PopDone' */
    } else if (sp == &ifilter_engineeringS[1]) {

      /* talk to hardware */
      ifilter_engineeringSP.s = IPS_BUSY;
      IDMessage(GALIL_DEVICE, "Calling xq_hx() from 'popdone'");
      if ((gstat=xq_hx()) == G_NO_ERROR) {
        IDMessage(GALIL_DEVICE, "Called xq_hx() from 'popdone' OK");
      } else {
        IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_hx() from 'popdone', gstat=%d", (int)gstat);
      }
      busy = false;
      popact = false;
      ifilter_engineeringSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
      telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
      telemetry_lightsL[1].s = IPS_IDLE;
      telemetry_lightsL[2].s = (popact == true) ? IPS_BUSY : IPS_IDLE;
      IDSetLight(&telemetry_lightsLP, NULL);
      ifilter_engineeringS[1].s = ISS_OFF;

    /* process 'Initialize' */
    } else if (sp == &ifilter_engineeringS[2]) {
      if (tcp_val.lv.filtisin == 1.0) {
        IDMessage(GALIL_DEVICE, "Cannot initialize whilst the filter is in the beam");
        ifilter_engineeringSP.s = IPS_OK;
      } else {

        /* talk to hardware */
        busy = true;
        telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
        IDSetLight(&telemetry_lightsLP, NULL);
        ifilter_engineeringSP.s = IPS_BUSY;
        IDMessage(GALIL_DEVICE, "Calling xq_reset_errfilt()");
        if ((gstat=xq_reset_errfilt()) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_reset_errfilt() OK");
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_reset_errfilt(), gstat=%d", (int)gstat);
        }
        IDMessage(GALIL_DEVICE, "Calling xq_filtrd()");
        if ((gstat=xq_filtrd()) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_filtrd() OK");
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_filtrd(), gstat=%d", (int)gstat);
        }
        ifilter_engineeringSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        busy = false;
        telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
        IDSetLight(&telemetry_lightsLP, NULL);
      }
      ifilter_engineeringS[2].s = ISS_OFF;

    /* process 'Halt Execution' */
    } else if (sp == &ifilter_engineeringS[3]) {

      /* talk to hardware */
      ifilter_engineeringSP.s = IPS_BUSY;
      IDMessage(GALIL_DEVICE, "Calling xq_hx()");
      if ((gstat=xq_hx()) == G_NO_ERROR) {
        IDMessage(GALIL_DEVICE, "Called xq_hx() OK");
      } else {
        IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_hx(), gstat=%d", (int)gstat);
      }
      busy = false;
      popact = false;
      ifilter_engineeringSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
      telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
      telemetry_lightsL[1].s = IPS_IDLE;
      telemetry_lightsL[2].s = (popact == true) ? IPS_BUSY : IPS_IDLE;
      IDSetLight(&telemetry_lightsLP, NULL);
      ifilter_engineeringS[3].s = ISS_OFF;
    }
  }

  /* reset */
  IUResetSwitch(&ifilter_engineeringSP);
  IDSetSwitch(&ifilter_engineeringSP, NULL);
}


/*******************************************************************************
 * action: ifilter_load_doit()
 ******************************************************************************/
GReturn ifilter_load_doit(void) {

  /* declare some variable(s) and initialize them */
  GReturn gstat = (GReturn)0;
  int countdown = BOK_NG_INSTRUMENT_LOAD_TIME;
  bool ifilter_load = false;

  /* load the filter */
  busy = true;
  telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
  IDSetLight(&telemetry_lightsLP, NULL);
  unscheduled_telemetry((void *)NULL);
  if (tcp_val.lv.filtisin == 0.0) {
    if ((gstat=xq_hx()) == G_NO_ERROR) {
      IDMessage(GALIL_DEVICE, "Called xq_hx() from '%s' OK", "ifilter_load_doit");
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_hx() from '%s', gstat=%d", "ifilter_load_doit", (int)gstat);
    }
    if ((gstat=xq_filtin()) == G_NO_ERROR) {
      IDMessage(GALIL_DEVICE, "Called xq_filtin() from '%s' OK", "ifilter_load_doit");
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_filtin() from '%s', gstat=%d", "ifilter_load_doit", (int)gstat);
    }
    while (--countdown > 0) {
      (void) sleep(1);
      unscheduled_telemetry((void *)NULL);
      IDMessage(GALIL_DEVICE, "tcp_val.lv.filtisin=%d timestamp='%s' countdown=%d", (int)tcp_val.lv.filtisin, tcp_val.timestamp, countdown);
      if (tcp_val.lv.filtisin==1.0 && tcp_val.lv.filttsc==2.0) { ifilter_load = true; break; }
    }
    if (ifilter_load) {
      IDMessage(GALIL_DEVICE, "ifilter loaded OK");
      ifilter_changeSP.s = IPS_OK;
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> failed to load ifilter");
      ifilter_changeSP.s = IPS_ALERT;
    }
    if ((gstat=xq_hx()) == G_NO_ERROR) {
      IDMessage(GALIL_DEVICE, "Called xq_hx() from '%s' OK", "ifilter_load_doit");
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_hx() from '%s', gstat=%d", "ifilter_load_doit", (int)gstat);
    }
  }
  busy = false;
  telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
  IDSetLight(&telemetry_lightsLP, NULL);
  return gstat;
}


/*******************************************************************************
 * action: ifilter_move_doit()
 ******************************************************************************/
GReturn ifilter_move_doit(int ifiltslot) {

  /* declare some variable(s) and initialize them */
  GReturn gstat = (GReturn)0;
  int countdown = BOK_NG_INSTRUMENT_FILTER_TIME;
  bool ifilter_move = false;

  /* move the filter */
  busy = true;
  telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
  IDSetLight(&telemetry_lightsLP, NULL);
  unscheduled_telemetry((void *)NULL);
  if ((gstat=xq_hx()) == G_NO_ERROR) {
    IDMessage(GALIL_DEVICE, "Called xq_hx() from '%s' OK", "ifilter_move_doit");
  } else {
    IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_hx() from '%s', gstat=%d", "ifilter_move_doit", (int)gstat);
  }
  if ((gstat=xq_reqfilt(tcp_val.filtvals[ifiltslot])) == G_NO_ERROR) {
    IDMessage(GALIL_DEVICE, "Called xq_reqfilt(%.1f) from '%s' OK", tcp_val.filtvals[ifiltslot], "ifilter_move_doit");
  } else {
    IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_reqfilt(%.1f) from '%s', gstat=%d", tcp_val.filtvals[ifiltslot], "ifilter_move_doit", (int)gstat);
  }
  if ((gstat=xq_filtmov()) == G_NO_ERROR) {
    IDMessage(GALIL_DEVICE, "Called xq_filtmov() from '%s' OK", "ifilter_move_doit");
  } else {
    IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_filtmov() from '%s', gstat=%d", "ifilter_move_doit", (int)gstat);
  }
  while (--countdown > 0) {
    (void) sleep(1);
    unscheduled_telemetry((void *)NULL);
    IDMessage(GALIL_DEVICE, "tcp_val.lv.filtval=%d tcp_val.lv.reqfilt=%d tcp_val.filtvals[%d]=%d timestamp='%s' countdown=%d", (int)tcp_val.lv.filtval, (int)tcp_val.lv.reqfilt, ifiltslot, (int)tcp_val.filtvals[ifiltslot], tcp_val.timestamp, countdown);
    if (tcp_val.lv.filtval==tcp_val.lv.reqfilt && tcp_val.lv.reqfilt==tcp_val.filtvals[ifiltslot]) { ifilter_move = true; break; }
  }
  if (ifilter_move) {
    IDMessage(GALIL_DEVICE, "ifilter moved OK");
    ifilter_changeSP.s = IPS_OK;
  } else {
    IDMessage(GALIL_DEVICE, "<ERROR> failed to move ifilter");
    ifilter_changeSP.s = IPS_ALERT;
  }
  if ((gstat=xq_hx()) == G_NO_ERROR) {
    IDMessage(GALIL_DEVICE, "Called xq_hx() from '%s' OK", "ifilter_move_doit");
  } else {
    IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_hx() from '%s', gstat=%d", "ifilter_move_doit", (int)gstat);
  }
  busy = false;
  telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
  IDSetLight(&telemetry_lightsLP, NULL);
  return gstat;
}


/*******************************************************************************
 * action: ifilter_read_doit()
 ******************************************************************************/
GReturn ifilter_read_doit(void) {

  /* declare some variable(s) and initialize them */
  GReturn gstat = (GReturn)0;
  int countdown = BOK_NG_INSTRUMENT_FILTER_TIME;
  bool ifilter_read = false;

  /* read the filter(s) */
  busy = true;
  telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
  IDSetLight(&telemetry_lightsLP, NULL);
  //unscheduled_telemetry((void *)NULL);
  if ((gstat=xq_hx()) == G_NO_ERROR) {
    IDMessage(GALIL_DEVICE, "Called xq_hx() from '%s' OK", "ifilter_read_doit");
  } else {
    IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_hx() from '%s', gstat=%d", "ifilter_read_doit", (int)gstat);
  }
  if ((gstat=xq_reset_errfilt()) == G_NO_ERROR) {
    IDMessage(GALIL_DEVICE, "Called xq_reset_errfilt() from '%s' OK", "ifilter_read_doit");
  } else {
    IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_reset_errfilt() from '%s', gstat=%d", "ifilter_read_doit", (int)gstat);
  }
  if ((gstat=xq_filtrd()) == G_NO_ERROR) {
    IDMessage(GALIL_DEVICE, "Called xq_filtrd() OK");
  } else {
    IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_filtrd(), gstat=%d", (int)gstat);
  }
  while (--countdown > 0) {
    (void) sleep(1);
    unscheduled_telemetry((void *)NULL);
    IDMessage(GALIL_DEVICE, "tcp_val.lv.filtnum=%d tcp_val.lv.filtval=%d tcp_val.lv.initfilt=%d timestamp='%s' countdown=%d", (int)tcp_val.lv.filtnum, (int)tcp_val.lv.filtval, (int)tcp_val.lv.initfilt, tcp_val.timestamp, countdown);
    if (tcp_val.lv.filtnum==6.0 && tcp_val.lv.initfilt==1.0) { ifilter_read = true; break; }
  }
  if (ifilter_read) {
    IDMessage(GALIL_DEVICE, "ifilter(s) read OK");
    ifilter_changeSP.s = IPS_OK;
  } else {
    IDMessage(GALIL_DEVICE, "<ERROR> failed to read ifilter(s)");
    ifilter_changeSP.s = IPS_ALERT;
  }
  if ((gstat=xq_hx()) == G_NO_ERROR) {
    IDMessage(GALIL_DEVICE, "Called xq_hx() from '%s' OK", "ifilter_read_doit");
  } else {
    IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_hx() from '%s', gstat=%d", "ifilter_read_doit", (int)gstat);
  }
  busy = false;
  telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
  IDSetLight(&telemetry_lightsLP, NULL);
  return gstat;
}


/*******************************************************************************
 * action: ifilter_unload_doit()
 ******************************************************************************/
GReturn ifilter_unload_doit(void) {

  /* declare some variable(s) and initialize them */
  GReturn gstat = (GReturn)0;
  int countdown = BOK_NG_INSTRUMENT_UNLOAD_TIME;
  bool ifilter_unload = false;

  /* unload the filter */
  busy = true;
  telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
  IDSetLight(&telemetry_lightsLP, NULL);
  unscheduled_telemetry((void *)NULL);
  if (tcp_val.lv.filtisin == 1.0) {
    if ((gstat=xq_hx()) == G_NO_ERROR) {
      IDMessage(GALIL_DEVICE, "Called xq_hx() from '%s' OK", "ifilter_unload_doit");
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_hx() from '%s', gstat=%d", "ifilter_unload_doit", (int)gstat);
    }
    if ((gstat=xq_filtout()) == G_NO_ERROR) {
      IDMessage(GALIL_DEVICE, "Called xq_filtout() from '%s' OK", "ifilter_unload_doit");
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_filtout() from '%s', gstat=%d", "ifilter_unload_doit", (int)gstat);
    }
    while (--countdown > 0) {
      (void) sleep(1);
      unscheduled_telemetry((void *)NULL);
      IDMessage(GALIL_DEVICE, "tcp_val.lv.filtisin=%d timestamp='%s' countdown=%d", (int)tcp_val.lv.filtisin, tcp_val.timestamp, countdown);
      if (tcp_val.lv.filtisin==0.0 && tcp_val.lv.filttsc==3.0) { ifilter_unload = true; break; }
    }
    if (ifilter_unload) {
      IDMessage(GALIL_DEVICE, "ifilter unloaded OK");
      ifilter_changeSP.s = IPS_OK;
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> failed to unload ifilter");
      ifilter_changeSP.s = IPS_ALERT;
    }
    if ((gstat=xq_hx()) == G_NO_ERROR) {
      IDMessage(GALIL_DEVICE, "Called xq_hx() from '%s' OK", "ifilter_unload_doit");
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_hx() from '%s', gstat=%d", "ifilter_unload_doit", (int)gstat);
    }
  }
  busy = false;
  telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
  IDSetLight(&telemetry_lightsLP, NULL);
  return gstat;
}


/*******************************************************************************
 * action: ifilter_change_doit()
 ******************************************************************************/
GReturn ifilter_change_doit(int ifiltslot) {
  GReturn gstat = (GReturn)0;
  gstat = ifilter_unload_doit();
  gstat = ifilter_move_doit(ifiltslot);
  gstat = ifilter_load_doit();
  return gstat;
}


/*******************************************************************************
 * action: execute_ifilter_change()
 ******************************************************************************/
void execute_ifilter_change(ISState states[], char *names[], int n) {

  /* declare some variables and initialize them */
  GReturn gstat = (GReturn)0;
  ISwitch *sp = (ISwitch *)NULL;
  ISState state = (ISState)NULL;
  bool state_change = false;

  /* find switches with the passed names in the ifilter_changeSP property */
  for (int i=0; i < n; i++) {
    sp = IUFindSwitch(&ifilter_changeSP, names[i]);
    state = states[i];
    state_change = state != sp->s;
    if (! state_change) { continue; }

    /* process 'iFilter 1' - NB: it's up to the higher-level software to check telemetry */
    if (sp == &ifilter_changeS[0]) {
      gstat = ifilter_change_doit(0);
      ifilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;

    /* process 'iFilter 2' - NB: it's up to the higher-level software to check telemetry */
    } else if (sp == &ifilter_changeS[1]) {
      gstat = ifilter_change_doit(1);
      ifilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;

    /* process 'iFilter 3' - NB: it's up to the higher-level software to check telemetry */
    } else if (sp == &ifilter_changeS[2]) {
      gstat = ifilter_change_doit(2);
      ifilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;

    /* process 'iFilter 4' - NB: it's up to the higher-level software to check telemetry */
    } else if (sp == &ifilter_changeS[3]) {
      gstat = ifilter_change_doit(3);
      ifilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;

    /* process 'iFilter 5' - NB: it's up to the higher-level software to check telemetry */
    } else if (sp == &ifilter_changeS[4]) {
      gstat = ifilter_change_doit(4);
      ifilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;

    /* process 'iFilter 6' - NB: it's up to the higher-level software to check telemetry */
    } else if (sp == &ifilter_changeS[5]) {
      gstat = ifilter_change_doit(5);
      ifilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;

/*
      if (tcp_val.lv.filtisin == 1.0) {
        IDMessage(GALIL_DEVICE, "Cannot change filter whilst another filter is in the beam");
        ifilter_changeSP.s = IPS_BUSY;
      } else if (tcp_val.lv.filtval==tcp_val.lv.reqfilt && tcp_val.lv.reqfilt == tcp_val.filtvals[5]) {
        IDMessage(GALIL_DEVICE, "iFilter 6 is already selected");
        ifilter_changeSP.s = IPS_OK;
      } else {
        busy = true;
        telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
        IDSetLight(&telemetry_lightsLP, NULL);
        ifilter_changeSP.s = IPS_BUSY;
        IDMessage(GALIL_DEVICE, "Calling xq_reqfilt(%.1f)", tcp_val.filtvals[5]);
        if ((gstat=xq_reqfilt(tcp_val.filtvals[5])) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_reqfilt(%.1f) OK", tcp_val.filtvals[5]);
          IDMessage(GALIL_DEVICE, "Calling xq_filtmov()");
          if ((gstat=xq_filtmov()) == G_NO_ERROR) {
            IDMessage(GALIL_DEVICE, "Called xq_filtmov() OK");
          } else {
            IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_filtmov(), gstat=%d", (int)gstat);
          }
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_reqfilt(%.1f), gstat=%d", tcp_val.filtvals[5], (int)gstat);
        }
        ifilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        busy = false;
        telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
        IDSetLight(&telemetry_lightsLP, NULL);
      }
      ifilter_changeS[5].s = ISS_OFF;
*/
    }
  }

  /* reset */
  IUResetSwitch(&ifilter_changeSP);
  IDSetSwitch(&ifilter_changeSP, NULL);
}

/*******************************************************************************
 * action: execute_ifilter_switches()
 ******************************************************************************/
void execute_ifilter_switches(ISState states[], char *names[], int n) {

  /* declare some variables and initialize them */
  GReturn gstat = (GReturn)0;
  ISwitch *sp = (ISwitch *)NULL;
  ISState state = (ISState)NULL;
  bool state_change = false;

  /* find switches with the passed names in the ifilterSP property */
  for (int i=0; i < n; i++) {
    sp = IUFindSwitch(&ifilterSP, names[i]);
    state = states[i];
    state_change = state != sp->s;
    if (! state_change) { continue; }

    /* process 'iFilter Load' - NB: it's up to the higher-level software to check telemetry */
    if (sp == &ifilterS[0]) {
      if (tcp_val.lv.filtisin == 1.0) {
        IDMessage(GALIL_DEVICE, "Filter is already in the beam");
        ifilterSP.s = IPS_OK;
      } else {

        /* talk to hardware */
        busy = true;
        telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
        IDSetLight(&telemetry_lightsLP, NULL);
        ifilterSP.s = IPS_BUSY;
        IDMessage(GALIL_DEVICE, "Calling xq_filtin()");
        if ((gstat=xq_filtin()) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_filtin() OK");
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_filtin(), gstat=%d", (int)gstat);
        }
        ifilterSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        busy = false;
        telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
        IDSetLight(&telemetry_lightsLP, NULL);
      }

      /* update widget(s) */
      ifilterS[0].s = ISS_OFF;

    /* process 'iFilter Unload' - NB: it's up to the higher-level software to check telemetry */
    } else if (sp == &ifilterS[1]) {
      if (tcp_val.lv.filtisin == 0.0) {
        IDMessage(GALIL_DEVICE, "Filter is already out of the beam");
        ifilterSP.s = IPS_OK;
      } else {

        /* talk to hardware */
        busy = true;
        telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
        IDSetLight(&telemetry_lightsLP, NULL);
        ifilterSP.s = IPS_BUSY;
        IDMessage(GALIL_DEVICE, "Calling xq_filtout()");
        if ((gstat=xq_filtout()) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_filtout() OK");
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_filtout(), gstat=%d", (int)gstat);
        }
        ifilterSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        busy = false;
        telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
        IDSetLight(&telemetry_lightsLP, NULL);
      }

      /* update widget(s) */
      ifilterS[1].s = ISS_OFF;

    /* process 'iFilter ReadWheel' - NB: it's up to the higher-level software to check telemetry */
    } else if (sp == &ifilterS[2]) {
    }
  }

  /* reset */
  IUResetSwitch(&ifilterSP);
  IDSetSwitch(&ifilterSP, NULL);
}


/*******************************************************************************
 * action: execute_ifocus_reference_switches()
 ******************************************************************************/
void execute_ifocus_reference_switches(ISState states[], char *names[], int n) {

  /* declare some variables and initialize them */
  GReturn gstat = (GReturn)0;
  ISwitch *sp = (ISwitch *)NULL;
  ISState state = (ISState)NULL;
  bool state_change = false;

  /* find switches with the passed names in the ifocus_referenceSP property */
  for (int i=0; i < n; i++) {
    sp = IUFindSwitch(&ifocus_referenceSP, names[i]);
    state = states[i];
    state_change = state != sp->s;
    if (! state_change) { continue; }

    /* process 'Save Focus Reference' */
    if (sp == &ifocus_referenceS[0]) {
      ifoci.refa = (float)udp_val.baxis_analog_in * BOK_LVDT_STEPS;
      ifoci.refb = (float)udp_val.daxis_analog_in * BOK_LVDT_STEPS;
      ifoci.refc = (float)udp_val.faxis_analog_in * BOK_LVDT_STEPS;
      IDMessage(GALIL_DEVICE, "Saving focus reference a=%.3f, b=%.3f, c=%.3f OK", ifoci.refa, ifoci.refb, ifoci.refc);

      /* update widget(s) */
      ifocus_referenceSP.s = IPS_OK;
      telemetry_referenceTP.s = IPS_OK;
      ifocus_referenceS[0].s = ISS_OFF;

    /* process 'Restore Focus Reference' */
    } else if (sp == &ifocus_referenceS[1]) {
      if (ifoci.refa == BOK_BAD_FLOAT || ifoci.refb == BOK_BAD_FLOAT || ifoci.refc == BOK_BAD_FLOAT) {
        IDMessage(GALIL_DEVICE, "Invalid focus reference");
      } else {

        float dista = 0.0;
        float distb = 0.0;
        float distc = 0.0;
        float focus_a = ifoci.refa * 1000.0;
        float focus_b = ifoci.refb * 1000.0;
        float focus_c = ifoci.refc * 1000.0;
        float tolerance = ifoci.tolerance;

        /* talk to hardware */
        busy = true;
        telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
        IDSetLight(&telemetry_lightsLP, NULL);
        ifocus_lvdtNP.s = IPS_BUSY;
        is_done = false;

        /* talk to hardware */
        int countdown = BOK_NG_INSTRUMENT_FOCUS_TIME;
        while (--countdown > 0) {

          /* get fresh telemetry */
          unscheduled_telemetry((void *)NULL);
          float cur_a = round(((float)udp_val.a_position * 1000.0));
          float cur_b = round(((float)udp_val.b_position * 1000.0));
          float cur_c = round(((float)udp_val.c_position * 1000.0));
          dista = round((focus_a/1000.0 - cur_a/1000.0) * BOK_LVDT_ATOD);
          distb = round((focus_b/1000.0 - cur_b/1000.0) * BOK_LVDT_ATOD);
          distc = round((focus_c/1000.0 - cur_c/1000.0) * BOK_LVDT_ATOD);
          float new_a = ifoci.refa * 1000.0;
          float new_b = ifoci.refb * 1000.0;
          float new_c = ifoci.refc * 1000.0;
          // if (tolerance<BOK_MIN_TOLERANCE || tolerance>BOK_MAX_TOLERANCE) { tolerance = BOK_NOM_TOLERANCE; }
          IDMessage(GALIL_DEVICE, "restore ifocus cur_a=%.4f dista=%.4f focus_a=%.4f new_a=%.4f tolerance=%.4f diff=%.4f timestamp='%s' countdown=%d", cur_a, dista, focus_a, new_a, tolerance, (cur_a - new_a), tcp_val.timestamp, countdown);
          IDMessage(GALIL_DEVICE, "restore ifocus cur_b=%.4f distb=%.4f focus_b=%.4f new_b=%.4f tolerance=%.4f diff=%.4f timestamp='%s' countdown=%d", cur_b, distb, focus_b, new_b, tolerance, (cur_b - new_b), tcp_val.timestamp, countdown);
          IDMessage(GALIL_DEVICE, "restore ifocus cur_c=%.4f distc=%.4f focus_c=%.4f new_c=%.4f tolerance=%.4f diff=%.4f timestamp='%s' countdown=%d", cur_c, distc, focus_c, new_c, tolerance, (cur_c - new_c), tcp_val.timestamp, countdown);
    
          /* end if within tolerance */
          if ( (abs(round(cur_a - new_a)) <= tolerance) && (abs(round(cur_b - new_b)) <= tolerance) && (abs(round(cur_c - new_c)) <= tolerance) ) { is_done = true; break; }
    
          /* stop current execution */
          if ((gstat=xq_hx()) == G_NO_ERROR) {
            IDMessage(GALIL_DEVICE, "Called xq_hx() from '%s' OK", "ifocus lvdt");
          } else {
            IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_hx() from '%s', gstat=%d", "ifocus lvdt", (int)gstat);
          }
    
          /* start new drive */
          if ((gstat=xq_focusind(dista, distb, distc)) == G_NO_ERROR) {
            IDMessage(GALIL_DEVICE, "Called xq_focusind(a=%.1f, b=%.1f, c=%.1f) from '%s' with tolerance %.2f OK", dista, distb, distc, "ifocus lvdt", tolerance);
          } else {
            IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_focusind(a=%.1f, b=%.1f, c=%.1f) from '%s' with tolerance %.2f, gstat=%d", dista, distb, distc, "ifocus lvdt", tolerance, (int)gstat);
          }
          (void) sleep(1);
        }
        if (is_done) {
          IDMessage(GALIL_DEVICE, "restore ifocus reached focus_a=%.4f focus_b=%.4f focus_c=%.4f within tolerance=%.4f OK", focus_a, focus_b, focus_c, tolerance);
          ifocus_lvdtNP.s = IPS_OK;
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> restore ifocus failed to reached focus_a=%.4f focus_b=%.4f focus_c=%.4f within tolerance=%.4f, countdown=%d OK", focus_a, focus_b, focus_c, tolerance, countdown);
          ifocus_lvdtNP.s = IPS_ALERT;
        }
        if ((gstat=xq_hx()) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_hx() from '%s' OK", "ifocus lvdt");
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_hx() from '%s', gstat=%d", "ifocus lvdt", (int)gstat);
        }
        busy = false;
        telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
        IDSetLight(&telemetry_lightsLP, NULL);
        IDSetNumber(&ifocus_lvdtNP, NULL);
        ifocus_referenceSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        telemetry_referenceTP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        ifocus_referenceS[1].s = ISS_OFF;
        IDSetNumber(&ifocus_lvdtNP, NULL);
      }

    /* process 'Save Nominal Plane' */
    } else if (sp == &ifocus_referenceS[2]) {

      /* nominal plane is offset from B position */
      ifoci.noma = (ifoci.vala - ifoci.valb) * 1000.0;
      ifoci.nomb = (ifoci.valb - ifoci.valb) * 1000.0;
      ifoci.nomc = (ifoci.valc - ifoci.valb) * 1000.0;
      IDMessage(GALIL_DEVICE, "Saving nominal plane a=%.3f b=%.3f c=%.3f", ifoci.noma, ifoci.nomb, ifoci.nomc);
      save_nominal_plane_to_file(BOK_NPLANE_FILE, ifoci.noma, ifoci.nomb, ifoci.nomc);

      /* update widget(s) */
      ifocus_referenceSP.s = IPS_OK;
      ifocus_referenceS[2].s = ISS_OFF;

    /* process 'Restore Nominal Plane' */
    } else if (sp == &ifocus_referenceS[3]) {
      read_nominal_plane_from_file(BOK_NPLANE_FILE, &ifoci.noma, &ifoci.nomb, &ifoci.nomc);
      if (ifoci.noma == NAN || ifoci.nomb == NAN || ifoci.nomc == NAN) {
        IDMessage(GALIL_DEVICE, "Invalid nominal position");
      } else {

        float dista = 0.0;
        float distb = 0.0;
        float distc = 0.0;
        float focus_a = 0.0;
        float focus_b = 0.0;
        float focus_c = 0.0;
        float tolerance = ifoci.tolerance;

        /* talk to hardware */
        busy = true;
        telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
        IDSetLight(&telemetry_lightsLP, NULL);
        ifocus_lvdtNP.s = IPS_BUSY;
        is_done  = false;
        int countdown = BOK_NG_INSTRUMENT_FOCUS_TIME;

	/* get value(s) */
        unscheduled_telemetry((void *)NULL);
        focus_a = round(((float)udp_val.a_position * 1000.0)) + ifoci.noma;
        focus_b = round(((float)udp_val.b_position * 1000.0)) + ifoci.nomb;
        focus_c = round(((float)udp_val.c_position * 1000.0)) + ifoci.nomc;

        while (--countdown > 0) {
          unscheduled_telemetry((void *)NULL);
          float cur_a = round(((float)udp_val.a_position * 1000.0));
          float cur_b = round(((float)udp_val.b_position * 1000.0));
          float cur_c = round(((float)udp_val.c_position * 1000.0));
          dista = round((focus_a/1000.0 - cur_a/1000.0) * BOK_LVDT_ATOD);
          distb = round((focus_b/1000.0 - cur_b/1000.0) * BOK_LVDT_ATOD);
          distc = round((focus_c/1000.0 - cur_c/1000.0) * BOK_LVDT_ATOD);
          float new_a = focus_a;
          float new_b = focus_b;
          float new_c = focus_c;
          // if (tolerance<BOK_MIN_TOLERANCE || tolerance>BOK_MAX_TOLERANCE) { tolerance = BOK_NOM_TOLERANCE; }
          IDMessage(GALIL_DEVICE, "nominal ifocus cur_a=%.4f dista=%.4f focus_a=%.4f new_a=%.4f tolerance=%.4f diff=%.4f, countdown=%d", cur_a, dista, focus_a, new_a, tolerance, (cur_a - new_a), countdown);
          IDMessage(GALIL_DEVICE, "nominal ifocus cur_b=%.4f distb=%.4f focus_b=%.4f new_b=%.4f tolerance=%.4f diff=%.4f, countdown=%d", cur_b, distb, focus_b, new_b, tolerance, (cur_b - new_b), countdown);
          IDMessage(GALIL_DEVICE, "nominal ifocus cur_c=%.4f distc=%.4f focus_c=%.4f new_c=%.4f tolerance=%.4f diff=%.4f, countdown=%d", cur_c, distc, focus_c, new_c, tolerance, (cur_c - new_c), countdown);
          if ((gstat=xq_hx()) == G_NO_ERROR) {
            IDMessage(GALIL_DEVICE, "Called xq_hx() from '%s' OK", "nominal ifocus");
          } else {
            IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_hx() from '%s', gstat=%d", "nominal ifocus", (int)gstat);
          }
          if ( (abs(round(cur_a - new_a)) <= tolerance) && (abs(round(cur_b - new_b)) <= tolerance) && (abs(round(cur_c - new_c)) <= tolerance) ) {
            IDMessage(GALIL_DEVICE, "nominal ifocus cur_a %.4f new_a %.4f within tolerance %.4f", cur_a, new_a, tolerance);
            IDMessage(GALIL_DEVICE, "nominal ifocus cur_b %.4f new_b %.4f within tolerance %.4f", cur_b, new_b, tolerance);
            IDMessage(GALIL_DEVICE, "nominal ifocus cur_c %.4f new_c %.4f within tolerance %.4f", cur_c, new_c, tolerance);
            is_done = true; break;
          }
          (void) sleep(1);
          IDMessage(GALIL_DEVICE, "Calling xq_focusind(a=%.1f, b=%.1f, c=%.1f) from '%s' with tolerance %.2f", dista, distb, distc, "nominal ifocus", tolerance);
          if ((gstat=xq_focusind(dista, distb, distc)) == G_NO_ERROR) {
            IDMessage(GALIL_DEVICE, "Called xq_focusind(a=%.1f, b=%.1f, c=%.1f) from '%s' with tolerance %.2f OK", dista, distb, distc, "nominal ifocus", tolerance);
          } else {
            IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_focusind(a=%.1f, b=%.1f, c=%.1f) from '%s' with tolerance %.2f, gstat=%d", dista, distb, distc, "nominal ifocus", tolerance, (int)gstat);
          }
          (void) sleep(1);
        }
        if (is_done) {
          IDMessage(GALIL_DEVICE, "nominal ifocus reached focus_a=%.4f focus_b=%.4f focus_c=%.4f within tolerance=%.4f, countdown=%d OK", focus_a, focus_b, focus_c, tolerance, countdown);
          ifocus_lvdtNP.s = IPS_OK;
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> nominal ifocus failed to reached focus_a=%.4f focus_b=%.4f focus_c=%.4f within tolerance=%.4f, countdown=%d OK", focus_a, focus_b, focus_c, tolerance, countdown);
          ifocus_lvdtNP.s = IPS_ALERT;
        }
        if ((gstat=xq_hx()) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_hx() from '%s' OK", "nominal ifocus");
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_hx() from '%s', gstat=%d", "nominal ifocus", (int)gstat);
        }
        busy = false;
        telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
        IDSetLight(&telemetry_lightsLP, NULL);
      }

      /* update widget(s) */
      ifocus_referenceSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
      ifocus_referenceS[3].s = ISS_OFF;
      IDSetNumber(&ifocus_lvdtNP, NULL);
    }
  }

  /* reset */
  IUResetSwitch(&ifocus_referenceSP);
  IDSetSwitch(&ifocus_referenceSP, NULL);
}


/*******************************************************************************
 * action: unscheduled_telemetry()
 ******************************************************************************/
static void unscheduled_telemetry(void *p) {

  /* declare some variable(s) and initialize them */
  int _gfiltn = -1;
  int _ifiltn = -1;
  int _tfd = -1;
  int _ufd = -1;
  tcp_val_p _tp = (tcp_val_p)NULL;
  udp_val_p _up = (udp_val_p)NULL;

  /* read TCP shared memory */
  if ((_tfd=shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666))>=0 &&
      (_tp=(tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, _tfd, 0))!=(tcp_val_p)NULL) {
    /* IDMessage(GALIL_DEVICE, "Reading TCP shared memory"); */
    (void) memset((void *)&tcp_val, 0, TCP_VAL_SIZE);
    (void) memmove((void *)&tcp_val, _tp, TCP_VAL_SIZE);
    /* (void) dump_tcp_structure(&tcp_val); */
    (void) munmap(_tp, TCP_VAL_SIZE);
    (void) close(_tfd);
    _tp = (tcp_val_p)NULL;
    _tfd = -1;
    /* IDMessage(GALIL_DEVICE, "Read TCP shared memory OK"); */
  } else {
    IDMessage(GALIL_DEVICE, "<ERROR> failed to call TCP shared memory");
  }

  /* read UDP shared memory */
  if ((_ufd=shm_open(BOK_SHM_UDP_NAME, O_RDONLY, 0666)) &&
       (_up=(udp_val_p)mmap(0, UDP_VAL_SIZE, PROT_READ, MAP_SHARED, _ufd, 0))!=(udp_val_p)NULL) {
    /* IDMessage(GALIL_DEVICE, "Reading UDP shared memory"); */
    (void) memset((void *)&udp_val, 0, UDP_VAL_SIZE);
    (void) memmove((void *)&udp_val, _up, UDP_VAL_SIZE);
    /* (void) dump_udp_structure(&udp_val); */
    (void) munmap(_up, UDP_VAL_SIZE);
    (void) close(_ufd);
    _up = (udp_val_p)NULL;
    _ufd = -1;
    /* IDMessage(GALIL_DEVICE, "Read UDP shared memory OK"); */
  } else {
    IDMessage(GALIL_DEVICE, "<ERROR> failed to call UDP shared memory");
  }

  /* update gfilter(s) */
  IUResetSwitch(&gfilter_changeSP);
  (void) memset((void *)&gfilter_names, 0, sizeof(gfilter_names));
  _gfiltn = (int)round(tcp_val.lv.gfiltn);
  (void) sprintf(gfilter_names.filter_1, "%s", bok_gfilters[1].name);
  (void) sprintf(gfilter_names.filter_2, "%s", bok_gfilters[2].name);
  (void) sprintf(gfilter_names.filter_3, "%s", bok_gfilters[3].name);
  (void) sprintf(gfilter_names.filter_4, "%s", bok_gfilters[4].name);
  (void) sprintf(gfilter_names.filter_5, "%s", bok_gfilters[5].name);
  (void) sprintf(gfilter_names.filter_6, "%s", bok_gfilters[6].name);
  if (_gfiltn>=1 && _gfiltn<=BOK_GFILTERS) {
    gfilter_changeS[_gfiltn - 1].s = ISS_ON;
  }
  IDSetSwitch(&gfilter_changeSP, NULL);
  IDSetText(&gfilterTP, NULL);

  /* update ifilter(s) */
  IUResetSwitch(&ifilter_changeSP);
  (void) memset((void *)&ifilter_names, 0, sizeof(ifilter_names));
  _ifiltn = (int)round(tcp_val.lv.filtval);
  if (_ifiltn == (int)round(tcp_val.filtvals[0])) {
    (void) sprintf(ifilter_names.filter_1, "%s", bok_ifilters[(int)round(tcp_val.filtvals[0])].name);
    ifilter_changeS[0].s = ISS_ON;
  } else {
    (void) sprintf(ifilter_names.filter_1, "%s", bok_ifilters[(int)round(tcp_val.filtvals[0])].name);
  }
  if (_ifiltn == (int)round(tcp_val.filtvals[1])) {
    (void) sprintf(ifilter_names.filter_2, "%s", bok_ifilters[(int)round(tcp_val.filtvals[1])].name);
    ifilter_changeS[1].s = ISS_ON;
  } else {
    (void) sprintf(ifilter_names.filter_2, "%s", bok_ifilters[(int)round(tcp_val.filtvals[1])].name);
  }
  if (_ifiltn == (int)round(tcp_val.filtvals[2])) {
    (void) sprintf(ifilter_names.filter_3, "%s", bok_ifilters[(int)round(tcp_val.filtvals[2])].name);
    ifilter_changeS[2].s = ISS_ON;
  } else {
    (void) sprintf(ifilter_names.filter_3, "%s", bok_ifilters[(int)round(tcp_val.filtvals[2])].name);
  }
  if (_ifiltn == (int)round(tcp_val.filtvals[3])) {
    (void) sprintf(ifilter_names.filter_4, "%s", bok_ifilters[(int)round(tcp_val.filtvals[3])].name);
    ifilter_changeS[3].s = ISS_ON;
  } else {
    (void) sprintf(ifilter_names.filter_4, "%s", bok_ifilters[(int)round(tcp_val.filtvals[3])].name);
  }
  if (_ifiltn == (int)round(tcp_val.filtvals[4])) {
    (void) sprintf(ifilter_names.filter_5, "%s", bok_ifilters[(int)round(tcp_val.filtvals[4])].name);
    ifilter_changeS[4].s = ISS_ON;
  } else {
    (void) sprintf(ifilter_names.filter_5, "%s", bok_ifilters[(int)round(tcp_val.filtvals[4])].name);
  }
  if (_ifiltn == (int)round(tcp_val.filtvals[5])) {
    (void) sprintf(ifilter_names.filter_6, "%s", bok_ifilters[(int)round(tcp_val.filtvals[5])].name);
    ifilter_changeS[5].s = ISS_ON;
  } else {
    (void) sprintf(ifilter_names.filter_6, "%s", bok_ifilters[(int)round(tcp_val.filtvals[5])].name);
  }
  IDSetSwitch(&ifilter_changeSP, NULL);
  IDSetText(&ifilterTP, NULL);

  /* update values */
  ifoci.enca = tcp_val.position[0];
  ifoci.encb = tcp_val.position[1];
  ifoci.encc = tcp_val.position[2];
  ifoci.vala = (float)udp_val.baxis_analog_in * BOK_LVDT_STEPS;
  ifoci.valb = (float)udp_val.daxis_analog_in * BOK_LVDT_STEPS;
  ifoci.valc = (float)udp_val.faxis_analog_in * BOK_LVDT_STEPS;

  /* set and update text(s) */
  (void) memset((void *)&telemetrys, 0, sizeof(telemetrys));
  for (int j=0; j<BOK_IFILTER_SLOTS; j++) {
    if (tcp_val.lv.filtval == (float)j) { (void) sprintf(telemetrys.filtval, "%s (%d)", bok_ifilters[j].name, j); }
    if (tcp_val.lv.reqfilt == (float)j) { (void) sprintf(telemetrys.reqfilt, "%s (%d)", bok_ifilters[j].name, j); }
  }
  for (int k=0; k<BOK_GFILTER_SLOTS; k++) {
    if (tcp_val.lv.gfiltn == (float)k) { (void) sprintf(telemetrys.gfiltn, "%s (%d)", bok_gfilters[k].name, k); }
    if (tcp_val.lv.fnum_in == (float)k) { (void) sprintf(telemetrys.fnum_in, "%s (%d)", bok_gfilters[k].name, k); }
  }
  for (int l=1; l<BOK_SFILTER_SLOTS; l++) {
    if (tcp_val.lv.snum == (float)l) { (void) sprintf(telemetrys.snum, "%s (%d)", bok_sfilters[l].name, l); }
    if (tcp_val.lv.snum_in == (float)l) { (void) sprintf(telemetrys.snum_in, "%s (%d)", bok_sfilters[l].name, l); }
  }
  (void) sprintf(telemetrys.hardware,     "%s",             tcp_val.hardware);
  (void) sprintf(telemetrys.software,     "%s",             tcp_val.software);
  (void) sprintf(telemetrys.timestamp,    "%s",             tcp_val.timestamp);
  (void) sprintf(telemetrys.jd,           "%f",             tcp_val.jd);
  (void) sprintf(telemetrys.tcp_shm_mode, "%s",             tcp_val.simulate > 0 ? "SIMULATE" : "DIRECT");
  (void) sprintf(telemetrys.udp_shm_mode, "%s",             udp_val.simulate > 0 ? "SIMULATE" : "DIRECT");
  (void) sprintf(telemetrys.a_encoder,    "%05d / %05d",    (int)ifoci.enca, udp_val.aaxis_motor_position);
  (void) sprintf(telemetrys.b_encoder,    "%05d / %05d",    (int)ifoci.encb, udp_val.baxis_motor_position);
  (void) sprintf(telemetrys.c_encoder,    "%05d / %05d",    (int)ifoci.encc, udp_val.caxis_motor_position);
  (void) sprintf(telemetrys.a_position,   "%05d / %08.3f",  udp_val.baxis_analog_in, ifoci.vala);
  (void) sprintf(telemetrys.b_position,   "%05d / %08.3f",  udp_val.daxis_analog_in, ifoci.valb);
  (void) sprintf(telemetrys.c_position,   "%05d / %08.3f",  udp_val.faxis_analog_in, ifoci.valc);
  (void) sprintf(telemetrys.a_reference,  "%.0f",           ifoci.refa == 123456792.0 ? 0.0 : ifoci.refa * 1000.0);
  (void) sprintf(telemetrys.b_reference,  "%.0f",           ifoci.refb == 123456792.0 ? 0.0 : ifoci.refb * 1000.0);
  (void) sprintf(telemetrys.c_reference,  "%.0f",           ifoci.refc == 123456792.0 ? 0.0 : ifoci.refc * 1000.0);
  (void) sprintf(telemetrys.lvdta,        "%.0f",           ifoci.vala * 1000.0);
  (void) sprintf(telemetrys.lvdtb,        "%.0f",           ifoci.valb * 1000.0);
  (void) sprintf(telemetrys.lvdtc,        "%.0f",           ifoci.valc * 1000.0);
  (void) sprintf(telemetrys.tolerance,    "%.0f",           ifoci.tolerance);
  (void) sprintf(telemetrys.distall,      "%08.1f",         tcp_val.lv.distall);
  (void) sprintf(telemetrys.dista,        "%08.1f",         tcp_val.lv.dista);
  (void) sprintf(telemetrys.distb,        "%08.1f",         tcp_val.lv.distb);
  (void) sprintf(telemetrys.distc,        "%08.1f",         tcp_val.lv.distc);
  (void) sprintf(telemetrys.distgcam,     "%.0f",           tcp_val.lv.distgcam);
  (void) sprintf(telemetrys.errfilt,      "%08.1f",         tcp_val.lv.errfilt);
  (void) sprintf(telemetrys.filttsc,      "%08.1f",         tcp_val.lv.filttsc);
  (void) sprintf(telemetrys.filtisin,     "%08.1f",         tcp_val.lv.filtisin);
  (void) sprintf(telemetrys.ifilter_1,    "%s (%d)",        ifilter_names.filter_1, (int)round(tcp_val.filtvals[0]));
  (void) sprintf(telemetrys.ifilter_2,    "%s (%d)",        ifilter_names.filter_2, (int)round(tcp_val.filtvals[1]));
  (void) sprintf(telemetrys.ifilter_3,    "%s (%d)",        ifilter_names.filter_3, (int)round(tcp_val.filtvals[2]));
  (void) sprintf(telemetrys.ifilter_4,    "%s (%d)",        ifilter_names.filter_4, (int)round(tcp_val.filtvals[3]));
  (void) sprintf(telemetrys.ifilter_5,    "%s (%d)",        ifilter_names.filter_5, (int)round(tcp_val.filtvals[4]));
  (void) sprintf(telemetrys.ifilter_6,    "%s (%d)",        ifilter_names.filter_6, (int)round(tcp_val.filtvals[5]));
  (void) sprintf(telemetrys.gfilter_1,    "%s (1)",         gfilter_names.filter_1);
  (void) sprintf(telemetrys.gfilter_2,    "%s (2)",         gfilter_names.filter_2);
  (void) sprintf(telemetrys.gfilter_3,    "%s (3)",         gfilter_names.filter_3);
  (void) sprintf(telemetrys.gfilter_4,    "%s (4)",         gfilter_names.filter_4);
  (void) sprintf(telemetrys.gfilter_5,    "%s (5)",         gfilter_names.filter_5);
  (void) sprintf(telemetrys.gfilter_6,    "%s (6)",         gfilter_names.filter_6);
  (void) sprintf(telemetrys.gfocus_position, "%d",          udp_val.eaxis_reference_position);
  (void) sprintf(telemetrys.gfocus_limit,  "%d",            (int)round(udp_val.eaxis_stop_code));
  
  /* update lights for guider limits */
  telemetry_glimitsL[0].s = ((int)round(udp_val.eaxis_stop_code)) == 2 ? IPS_ALERT : IPS_IDLE;
  telemetry_glimitsL[1].s = ((int)round(udp_val.eaxis_stop_code)) == 3 ? IPS_ALERT : IPS_IDLE;
  IDSetLight(&telemetry_glimitsLP, NULL);

  /* update LVDT */
  ifocus_lvdtN[0].value = ifoci.vala * 1000.0;
  ifocus_lvdtN[1].value = ifoci.valb * 1000.0;
  ifocus_lvdtN[2].value = ifoci.valc * 1000.0;
  IDSetNumber(&ifocus_lvdtNP, NULL);

  /* update guider focus */
  gfocus_distN[0].value = udp_val.eaxis_reference_position;
  IDSetNumber(&gfocus_distNP, NULL);

  IDSetText(&telemetryTP, NULL);
  IDSetText(&telemetry_referenceTP, NULL);
  IDSetText(&telemetry_lvdtTP, NULL);
  IDSetText(&telemetry_gfocusTP, NULL);

  /* set and update light(s)*/
  telemetry_engineeringL[0].s = (tcp_val.simulate == 0) ? IPS_OK : IPS_ALERT;
  telemetry_engineeringL[1].s = (udp_val.simulate == 0) ? IPS_OK : IPS_ALERT;
  telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
  telemetry_lightsL[1].s = (IS_BIT_SET(tcp_val.status, 7)) ? IPS_BUSY : IPS_IDLE;
  telemetry_lightsL[2].s = (popact == true) ? IPS_BUSY : IPS_IDLE;
  telemetry_gfilterwheelL[0].s = (udp_val.haxis_moving == 1) ? IPS_BUSY : IPS_IDLE;

  telemetry_ifilterwheelL[0].s = (tcp_val.lv.filtisin == 1.0) ? IPS_IDLE : IPS_OK;
  telemetry_ifilterwheelL[1].s = (tcp_val.lv.filtisin == 1.0) ? IPS_BUSY : IPS_IDLE;
  telemetry_ifilterwheelL[2].s = (udp_val.faxis_moving == 1) ? IPS_BUSY : IPS_IDLE;
  telemetry_ifilterwheelL[3].s = (udp_val.gaxis_moving == 1) ? IPS_BUSY : IPS_IDLE;
  telemetry_ifilterwheelL[4].s = (tcp_val.lv.errfilt == 1.0) ? IPS_ALERT : IPS_IDLE;
  telemetry_ifilterwheelL[5].s = tcp_val.lv.filttsc==1.0 ? IPS_ALERT : IPS_IDLE;

  telemetry_engineeringLP.s = IPS_IDLE;

  /* logic to update light for filter wheel lights */
  if (tcp_val.lv.errfilt == 1.0) {
    telemetry_ifilterwheelLP.s = IPS_ALERT;
  } else if (tcp_val.lv.filttsc==1.0) {
    telemetry_ifilterwheelLP.s = IPS_ALERT;
  } else if (udp_val.haxis_moving==1 || udp_val.faxis_moving==1 || udp_val.gaxis_moving==1 || tcp_val.lv.filtisin==1.0) {
    telemetry_ifilterwheelLP.s = IPS_BUSY;
  } else if (tcp_val.lv.filtisin != 1.0) {
    telemetry_ifilterwheelLP.s = IPS_OK;
  } else {
    telemetry_ifilterwheelLP.s = IPS_IDLE;
  }
  telemetry_gfilterwheelLP.s = IPS_IDLE;
  IDSetLight(&telemetry_engineeringLP, NULL);
  IDSetLight(&telemetry_lightsLP, NULL);
  IDSetLight(&telemetry_ifilterwheelLP, NULL);
  IDSetLight(&telemetry_gfilterwheelLP, NULL);
}


/*******************************************************************************
 * action: scheduled_telemetry()
 ******************************************************************************/
static void scheduled_telemetry(void *p) {

  /* declare some variable(s) and initialize them */
  int _gfiltn = -1;
  int _ifiltn = -1;
  int _tfd = -1;
  int _ufd = -1;
  tcp_val_p _tp = (tcp_val_p)NULL;
  udp_val_p _up = (udp_val_p)NULL;

  /* read TCP shared memory */
  if ((_tfd=shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666))>=0 &&
      (_tp=(tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, _tfd, 0))!=(tcp_val_p)NULL) {
    /* IDMessage(GALIL_DEVICE, "Reading TCP shared memory"); */
    (void) memset((void *)&tcp_val, 0, TCP_VAL_SIZE);
    (void) memmove((void *)&tcp_val, _tp, TCP_VAL_SIZE);
    /* (void) dump_tcp_structure(&tcp_val); */
    (void) munmap(_tp, TCP_VAL_SIZE);
    (void) close(_tfd);
    _tp = (tcp_val_p)NULL;
    _tfd = -1;
    /* IDMessage(GALIL_DEVICE, "Read TCP shared memory OK"); */
  } else {
    IDMessage(GALIL_DEVICE, "<ERROR> failed to call TCP shared memory");
  }

  /* read UDP shared memory */
  if ((_ufd=shm_open(BOK_SHM_UDP_NAME, O_RDONLY, 0666)) &&
       (_up=(udp_val_p)mmap(0, UDP_VAL_SIZE, PROT_READ, MAP_SHARED, _ufd, 0))!=(udp_val_p)NULL) {
    /* IDMessage(GALIL_DEVICE, "Reading UDP shared memory"); */
    (void) memset((void *)&udp_val, 0, UDP_VAL_SIZE);
    (void) memmove((void *)&udp_val, _up, UDP_VAL_SIZE);
    /* (void) dump_udp_structure(&udp_val); */
    (void) munmap(_up, UDP_VAL_SIZE);
    (void) close(_ufd);
    _up = (udp_val_p)NULL;
    _ufd = -1;
    /* IDMessage(GALIL_DEVICE, "Read UDP shared memory OK"); */
  } else {
    IDMessage(GALIL_DEVICE, "<ERROR> failed to call UDP shared memory");
  }

  /* update gfilter(s) */
  IUResetSwitch(&gfilter_changeSP);
  (void) memset((void *)&gfilter_names, 0, sizeof(gfilter_names));
  _gfiltn = (int)round(tcp_val.lv.gfiltn);
  (void) sprintf(gfilter_names.filter_1, "%s", bok_gfilters[1].name);
  (void) sprintf(gfilter_names.filter_2, "%s", bok_gfilters[2].name);
  (void) sprintf(gfilter_names.filter_3, "%s", bok_gfilters[3].name);
  (void) sprintf(gfilter_names.filter_4, "%s", bok_gfilters[4].name);
  (void) sprintf(gfilter_names.filter_5, "%s", bok_gfilters[5].name);
  (void) sprintf(gfilter_names.filter_6, "%s", bok_gfilters[6].name);
  if (_gfiltn>=1 && _gfiltn<=BOK_GFILTERS) {
    gfilter_changeS[_gfiltn - 1].s = ISS_ON;
  }
  IDSetSwitch(&gfilter_changeSP, NULL);
  IDSetText(&gfilterTP, NULL);

  /* update ifilter(s) */
  IUResetSwitch(&ifilter_changeSP);
  (void) memset((void *)&ifilter_names, 0, sizeof(ifilter_names));
  _ifiltn = (int)round(tcp_val.lv.filtval);
  if (_ifiltn == (int)round(tcp_val.filtvals[0])) {
    (void) sprintf(ifilter_names.filter_1, "%s", bok_ifilters[(int)round(tcp_val.filtvals[0])].name);
    ifilter_changeS[0].s = ISS_ON;
  } else {
    (void) sprintf(ifilter_names.filter_1, "%s", bok_ifilters[(int)round(tcp_val.filtvals[0])].name);
  }
  if (_ifiltn == (int)round(tcp_val.filtvals[1])) {
    (void) sprintf(ifilter_names.filter_2, "%s", bok_ifilters[(int)round(tcp_val.filtvals[1])].name);
    ifilter_changeS[1].s = ISS_ON;
  } else {
    (void) sprintf(ifilter_names.filter_2, "%s", bok_ifilters[(int)round(tcp_val.filtvals[1])].name);
  }
  if (_ifiltn == (int)round(tcp_val.filtvals[2])) {
    (void) sprintf(ifilter_names.filter_3, "%s", bok_ifilters[(int)round(tcp_val.filtvals[2])].name);
    ifilter_changeS[2].s = ISS_ON;
  } else {
    (void) sprintf(ifilter_names.filter_3, "%s", bok_ifilters[(int)round(tcp_val.filtvals[2])].name);
  }
  if (_ifiltn == (int)round(tcp_val.filtvals[3])) {
    (void) sprintf(ifilter_names.filter_4, "%s", bok_ifilters[(int)round(tcp_val.filtvals[3])].name);
    ifilter_changeS[3].s = ISS_ON;
  } else {
    (void) sprintf(ifilter_names.filter_4, "%s", bok_ifilters[(int)round(tcp_val.filtvals[3])].name);
  }
  if (_ifiltn == (int)round(tcp_val.filtvals[4])) {
    (void) sprintf(ifilter_names.filter_5, "%s", bok_ifilters[(int)round(tcp_val.filtvals[4])].name);
    ifilter_changeS[4].s = ISS_ON;
  } else {
    (void) sprintf(ifilter_names.filter_5, "%s", bok_ifilters[(int)round(tcp_val.filtvals[4])].name);
  }
  if (_ifiltn == (int)round(tcp_val.filtvals[5])) {
    (void) sprintf(ifilter_names.filter_6, "%s", bok_ifilters[(int)round(tcp_val.filtvals[5])].name);
    ifilter_changeS[5].s = ISS_ON;
  } else {
    (void) sprintf(ifilter_names.filter_6, "%s", bok_ifilters[(int)round(tcp_val.filtvals[5])].name);
  }
  IDSetSwitch(&ifilter_changeSP, NULL);
  IDSetText(&ifilterTP, NULL);

  /* update values */
  ifoci.enca = tcp_val.position[0];
  ifoci.encb = tcp_val.position[1];
  ifoci.encc = tcp_val.position[2];
  ifoci.vala = (float)udp_val.baxis_analog_in * BOK_LVDT_STEPS;
  ifoci.valb = (float)udp_val.daxis_analog_in * BOK_LVDT_STEPS;
  ifoci.valc = (float)udp_val.faxis_analog_in * BOK_LVDT_STEPS;

  /* set and update text(s) */
  (void) memset((void *)&telemetrys, 0, sizeof(telemetrys));
  for (int j=0; j<BOK_IFILTER_SLOTS; j++) {
    if (tcp_val.lv.filtval == (float)j) { (void) sprintf(telemetrys.filtval, "%s (%d)", bok_ifilters[j].name, j); }
    if (tcp_val.lv.reqfilt == (float)j) { (void) sprintf(telemetrys.reqfilt, "%s (%d)", bok_ifilters[j].name, j); }
  }
  for (int k=0; k<BOK_GFILTER_SLOTS; k++) {
    if (tcp_val.lv.gfiltn == (float)k) { (void) sprintf(telemetrys.gfiltn, "%s (%d)", bok_gfilters[k].name, k); }
    if (tcp_val.lv.fnum_in == (float)k) { (void) sprintf(telemetrys.fnum_in, "%s (%d)", bok_gfilters[k].name, k); }
  }
  for (int l=1; l<BOK_SFILTER_SLOTS; l++) {
    if (tcp_val.lv.snum == (float)l) { (void) sprintf(telemetrys.snum, "%s (%d)", bok_sfilters[l].name, l); }
    if (tcp_val.lv.snum_in == (float)l) { (void) sprintf(telemetrys.snum_in, "%s (%d)", bok_sfilters[l].name, l); }
  }
  (void) sprintf(telemetrys.hardware,     "%s",             tcp_val.hardware);
  (void) sprintf(telemetrys.software,     "%s",             tcp_val.software);
  (void) sprintf(telemetrys.timestamp,    "%s",             tcp_val.timestamp);
  (void) sprintf(telemetrys.jd,           "%f",             tcp_val.jd);
  (void) sprintf(telemetrys.tcp_shm_mode, "%s",             tcp_val.simulate > 0 ? "SIMULATE" : "DIRECT");
  (void) sprintf(telemetrys.udp_shm_mode, "%s",             udp_val.simulate > 0 ? "SIMULATE" : "DIRECT");
  (void) sprintf(telemetrys.a_encoder,    "%05d / %05d",    (int)ifoci.enca, udp_val.aaxis_motor_position);
  (void) sprintf(telemetrys.b_encoder,    "%05d / %05d",    (int)ifoci.encb, udp_val.baxis_motor_position);
  (void) sprintf(telemetrys.c_encoder,    "%05d / %05d",    (int)ifoci.encc, udp_val.caxis_motor_position);
  (void) sprintf(telemetrys.a_position,   "%05d / %08.3f",  udp_val.baxis_analog_in, ifoci.vala);
  (void) sprintf(telemetrys.b_position,   "%05d / %08.3f",  udp_val.daxis_analog_in, ifoci.valb);
  (void) sprintf(telemetrys.c_position,   "%05d / %08.3f",  udp_val.faxis_analog_in, ifoci.valc);
  (void) sprintf(telemetrys.a_reference,  "%.0f",           ifoci.refa == 123456792.0 ? 0.0 : ifoci.refa * 1000.0);
  (void) sprintf(telemetrys.b_reference,  "%.0f",           ifoci.refb == 123456792.0 ? 0.0 : ifoci.refb * 1000.0);
  (void) sprintf(telemetrys.c_reference,  "%.0f",           ifoci.refc == 123456792.0 ? 0.0 : ifoci.refc * 1000.0);
  (void) sprintf(telemetrys.lvdta,        "%.0f",           ifoci.vala * 1000.0);
  (void) sprintf(telemetrys.lvdtb,        "%.0f",           ifoci.valb * 1000.0);
  (void) sprintf(telemetrys.lvdtc,        "%.0f",           ifoci.valc * 1000.0);
  (void) sprintf(telemetrys.tolerance,    "%.0f",           ifoci.tolerance);
  (void) sprintf(telemetrys.distall,      "%08.1f",         tcp_val.lv.distall);
  (void) sprintf(telemetrys.dista,        "%08.1f",         tcp_val.lv.dista);
  (void) sprintf(telemetrys.distb,        "%08.1f",         tcp_val.lv.distb);
  (void) sprintf(telemetrys.distc,        "%08.1f",         tcp_val.lv.distc);
  (void) sprintf(telemetrys.distgcam,     "%.0f",           tcp_val.lv.distgcam);
  (void) sprintf(telemetrys.errfilt,      "%08.1f",         tcp_val.lv.errfilt);
  (void) sprintf(telemetrys.filttsc,      "%08.1f",         tcp_val.lv.filttsc);
  (void) sprintf(telemetrys.filtisin,     "%08.1f",         tcp_val.lv.filtisin);
  (void) sprintf(telemetrys.ifilter_1,    "%s (%d)",        ifilter_names.filter_1, (int)round(tcp_val.filtvals[0]));
  (void) sprintf(telemetrys.ifilter_2,    "%s (%d)",        ifilter_names.filter_2, (int)round(tcp_val.filtvals[1]));
  (void) sprintf(telemetrys.ifilter_3,    "%s (%d)",        ifilter_names.filter_3, (int)round(tcp_val.filtvals[2]));
  (void) sprintf(telemetrys.ifilter_4,    "%s (%d)",        ifilter_names.filter_4, (int)round(tcp_val.filtvals[3]));
  (void) sprintf(telemetrys.ifilter_5,    "%s (%d)",        ifilter_names.filter_5, (int)round(tcp_val.filtvals[4]));
  (void) sprintf(telemetrys.ifilter_6,    "%s (%d)",        ifilter_names.filter_6, (int)round(tcp_val.filtvals[5]));
  (void) sprintf(telemetrys.gfilter_1,    "%s (1)",         gfilter_names.filter_1);
  (void) sprintf(telemetrys.gfilter_2,    "%s (2)",         gfilter_names.filter_2);
  (void) sprintf(telemetrys.gfilter_3,    "%s (3)",         gfilter_names.filter_3);
  (void) sprintf(telemetrys.gfilter_4,    "%s (4)",         gfilter_names.filter_4);
  (void) sprintf(telemetrys.gfilter_5,    "%s (5)",         gfilter_names.filter_5);
  (void) sprintf(telemetrys.gfilter_6,    "%s (6)",         gfilter_names.filter_6);
  (void) sprintf(telemetrys.gfocus_position, "%d",          udp_val.eaxis_reference_position);
  (void) sprintf(telemetrys.gfocus_limit,  "%d",            (int)round(udp_val.eaxis_stop_code));
  
  /* update lights for guider limits */
  telemetry_glimitsL[0].s = ((int)round(udp_val.eaxis_stop_code)) == 2 ? IPS_ALERT : IPS_IDLE;
  telemetry_glimitsL[1].s = ((int)round(udp_val.eaxis_stop_code)) == 3 ? IPS_ALERT : IPS_IDLE;
  IDSetLight(&telemetry_glimitsLP, NULL);

  /* update LVDT */
  ifocus_lvdtN[0].value = ifoci.vala * 1000.0;
  ifocus_lvdtN[1].value = ifoci.valb * 1000.0;
  ifocus_lvdtN[2].value = ifoci.valc * 1000.0;
  IDSetNumber(&ifocus_lvdtNP, NULL);

  /* update guider focus */
  gfocus_distN[0].value = udp_val.eaxis_reference_position;
  IDSetNumber(&gfocus_distNP, NULL);

  IDSetText(&telemetryTP, NULL);
  IDSetText(&telemetry_referenceTP, NULL);
  IDSetText(&telemetry_lvdtTP, NULL);
  IDSetText(&telemetry_gfocusTP, NULL);

  /* set and update light(s)*/
  telemetry_engineeringL[0].s = (tcp_val.simulate == 0) ? IPS_OK : IPS_ALERT;
  telemetry_engineeringL[1].s = (udp_val.simulate == 0) ? IPS_OK : IPS_ALERT;
  telemetry_lightsL[0].s = (busy == true) ? IPS_BUSY : IPS_IDLE;
  telemetry_lightsL[1].s = (IS_BIT_SET(tcp_val.status, 7)) ? IPS_BUSY : IPS_IDLE;
  telemetry_lightsL[2].s = (popact == true) ? IPS_BUSY : IPS_IDLE;
  telemetry_gfilterwheelL[0].s = (udp_val.haxis_moving == 1) ? IPS_BUSY : IPS_IDLE;

  telemetry_ifilterwheelL[0].s = (tcp_val.lv.filtisin == 1.0) ? IPS_IDLE : IPS_OK;
  telemetry_ifilterwheelL[1].s = (tcp_val.lv.filtisin == 1.0) ? IPS_BUSY : IPS_IDLE;
  telemetry_ifilterwheelL[2].s = (udp_val.faxis_moving == 1) ? IPS_BUSY : IPS_IDLE;
  telemetry_ifilterwheelL[3].s = (udp_val.gaxis_moving == 1) ? IPS_BUSY : IPS_IDLE;
  telemetry_ifilterwheelL[4].s = (tcp_val.lv.errfilt == 1.0) ? IPS_ALERT : IPS_IDLE;
  telemetry_ifilterwheelL[5].s = tcp_val.lv.filttsc==1.0 ? IPS_ALERT : IPS_IDLE;

  telemetry_engineeringLP.s = IPS_IDLE;

  /* logic to update light for filter wheel lights */
  if (tcp_val.lv.errfilt == 1.0) {
    telemetry_ifilterwheelLP.s = IPS_ALERT;
  } else if (tcp_val.lv.filttsc==1.0) {
    telemetry_ifilterwheelLP.s = IPS_ALERT;
  } else if (udp_val.haxis_moving==1 || udp_val.faxis_moving==1 || udp_val.gaxis_moving==1 || tcp_val.lv.filtisin==1.0) {
    telemetry_ifilterwheelLP.s = IPS_BUSY;
  } else if (tcp_val.lv.filtisin != 1.0) {
    telemetry_ifilterwheelLP.s = IPS_OK;
  } else {
    telemetry_ifilterwheelLP.s = IPS_IDLE;
  }
  telemetry_gfilterwheelLP.s = IPS_IDLE;
  IDSetLight(&telemetry_engineeringLP, NULL);
  IDSetLight(&telemetry_lightsLP, NULL);
  IDSetLight(&telemetry_ifilterwheelLP, NULL);
  IDSetLight(&telemetry_gfilterwheelLP, NULL);

  /* re-schedule */
  IEAddTimer(BOK_TCP_DELAY_MS, scheduled_telemetry, NULL);
}


/*******************************************************************************
 * action: zero_telemetry()
 ******************************************************************************/
static void zero_telemetry(void) {

  /* initialize structure(s) */
  (void) memset((void *)&gfilter_names, 0, sizeof(gfilter_names));
  (void) memset((void *)&ifilter_names, 0, sizeof(ifilter_names));
  (void) memset((void *)&ifoci, 0, sizeof(ifoci));
  (void) memset((void *)&supports, 0, sizeof(supports));
  (void) memset((void *)&telemetrys, 0, sizeof(telemetrys));

  /* populate */
  ifoci.enca = BOK_BAD_FLOAT;
  ifoci.encb = BOK_BAD_FLOAT;
  ifoci.encc = BOK_BAD_FLOAT;
  ifoci.lima = BOK_BAD_FLOAT;
  ifoci.limb = BOK_BAD_FLOAT;
  ifoci.limc = BOK_BAD_FLOAT;
  ifoci.noma = BOK_BAD_FLOAT;
  ifoci.nomb = BOK_BAD_FLOAT;
  ifoci.nomc = BOK_BAD_FLOAT;
  ifoci.refa = BOK_BAD_FLOAT;
  ifoci.refb = BOK_BAD_FLOAT;
  ifoci.refc = BOK_BAD_FLOAT;
  ifoci.vala = BOK_BAD_FLOAT;
  ifoci.valb = BOK_BAD_FLOAT;
  ifoci.valc = BOK_BAD_FLOAT;
  ifoci.tolerance = BOK_NOM_TOLERANCE;

  (void) sprintf(supports.author,  "%s", _AUTHOR_);
  (void) sprintf(supports.date,    "%s", _DATE_);
  (void) sprintf(supports.email,   "%s", _EMAIL_);
  (void) sprintf(supports.version, "%s", _VERSION_);

  telemetry_engineeringL[0].s = ISS_OFF;
  telemetry_engineeringL[1].s = ISS_OFF;
  telemetry_lightsL[0].s = ISS_OFF;
  telemetry_lightsL[1].s = ISS_OFF;
  telemetry_lightsL[2].s = ISS_OFF;
  telemetry_gfilterwheelL[0].s = ISS_OFF;

  telemetry_ifilterwheelL[0].s = ISS_OFF;
  telemetry_ifilterwheelL[1].s = ISS_OFF;
  telemetry_ifilterwheelL[2].s = ISS_OFF;
  telemetry_ifilterwheelL[3].s = ISS_OFF;

  telemetry_engineeringLP.s = IPS_IDLE;
  telemetry_lightsLP.s   = IPS_IDLE;
  telemetry_gfilterwheelLP.s = IPS_IDLE;
  telemetry_ifilterwheelLP.s = IPS_IDLE;

  /* initialize indi structure(s) */
  IDSetText(&ifilterTP, NULL);
  IDSetText(&gfilterTP, NULL);
  IDSetText(&supportTP, NULL);
  IDSetText(&telemetryTP, NULL);
  IDSetLight(&telemetry_engineeringLP, NULL);
  IDSetLight(&telemetry_lightsLP, NULL);
  IDSetLight(&telemetry_gfilterwheelLP, NULL);
  IDSetLight(&telemetry_ifilterwheelLP, NULL);
}
