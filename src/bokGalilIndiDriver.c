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
  char distall[BOK_STR_32];
  char dista[BOK_STR_32];
  char distb[BOK_STR_32];
  char distc[BOK_STR_32];
  char distgcam[BOK_STR_32];
  char errfilt[BOK_STR_32];
  char filtisin[BOK_STR_32];
  char filtval[BOK_STR_32];
  char ifilter_1[BOK_STR_64];
  char ifilter_2[BOK_STR_64];
  char ifilter_3[BOK_STR_64];
  char ifilter_4[BOK_STR_64];
  char ifilter_5[BOK_STR_64];
  char ifilter_6[BOK_STR_64];
  char gfiltn[BOK_STR_32];
  char reqfilt[BOK_STR_32];
  char gfilter_1[BOK_STR_64];
  char gfilter_2[BOK_STR_64];
  char gfilter_3[BOK_STR_64];
  char gfilter_4[BOK_STR_64];
  char gfilter_5[BOK_STR_64];
  char gfilter_6[BOK_STR_64];
} telemetry_t, *telemetry_p, **telemetry_s;


/*******************************************************************************
 * prototype(s)
 ******************************************************************************/
static void driver_init(void);
static void execute_gfilter_switches(ISState [], char *[], int);
static void execute_gfilter_change(ISState [], char *[], int);
static void execute_ifilter_startup(ISState [], char *[], int);
static void execute_ifilter_switches(ISState [], char *[], int);
static void execute_ifilter_change(ISState [], char *[], int);
static void execute_ifocus_reference_switches(ISState [], char *[], int);
static void execute_timer(void *);
static void zero_telemetry(void);

/*******************************************************************************
 * static variable(s)
 ******************************************************************************/
static bool busy = false;
static filter_file_t bok_ifilters[BOK_IFILTER_SLOTS];
static filter_file_t bok_gfilters[BOK_GFILTER_SLOTS];
static bool initialized = false;
static int loop_counter = -1;
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
/* gfilter group */
static ISwitch gfilterS[] = {
  {"g_initfw", "gFilter Initialize", ISS_OFF, 0, 0},
};
ISwitchVectorProperty gfilterSP = {
  GALIL_DEVICE, "GFILTER_ACTIONS", "Actions", GFILTER_GROUP, IP_RW, ISR_1OFMANY, 0.0, IPS_IDLE, gfilterS, NARRAY(gfilterS), "", 0
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
  GALIL_DEVICE, "GFILTER_CHANGE", "Change Filter", GFILTER_GROUP, IP_RW, ISR_1OFMANY, 0.0, IPS_IDLE, gfilter_changeS, NARRAY(gfilter_changeS), "", 0
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
  GALIL_DEVICE, "GFILTER_NAMES", "Filter Names", GFILTER_GROUP, IP_RO, 0.0, IPS_IDLE, gfilterT, NARRAY(gfilterT), "", 0
};

/* gfocus group */
static INumber gfocus_distN[] = {
  {"distgcam", "Focus", "%5.0f", -100.0, 100.0, 1.0, 0.0, 0, 0, 0}
};
static INumberVectorProperty gfocus_distNP = {
  GALIL_DEVICE, "GFOCUS_DIST", "Change Focus", GFOCUS_GROUP, IP_WO, 0.0, IPS_IDLE, gfocus_distN, NARRAY(gfocus_distN), "", 0
};

/* ifilter group */
static ISwitch ifilter_startupS[] = {
  {"i_populate", "Populate", ISS_OFF, 0, 0},
  {"i_popdone",  "PopDone", ISS_OFF, 0, 0},
  {"i_readfw",   "Read Filters", ISS_OFF, 0, 0},
  {"i_initfw",   "FW Initialize", ISS_OFF, 0, 0}
};
ISwitchVectorProperty ifilter_startupSP = {
  GALIL_DEVICE, "IFILTER_STARTUP", "FW Startup", IFILTER_GROUP, IP_RW, ISR_1OFMANY, 0.0, IPS_IDLE, ifilter_startupS, NARRAY(ifilter_startupS), "", 0
};

static ISwitch ifilterS[] = {
  {"i_load",     "Load Filter", ISS_OFF, 0, 0},
  {"i_unload",   "Unload Filter", ISS_OFF, 0, 0}
};
ISwitchVectorProperty ifilterSP = {
  GALIL_DEVICE, "IFILTER_ACTIONS", "Actions", IFILTER_GROUP, IP_RW, ISR_1OFMANY, 0.0, IPS_IDLE, ifilterS, NARRAY(ifilterS), "", 0
};

static ISwitch ifilter_changeS[] = {
  {"i_slot_1",   "iFilter 1", ISS_OFF, 0, 0},
  {"i_slot_2",   "iFilter 2", ISS_OFF, 0, 0},
  {"i_slot_3",   "iFilter 3", ISS_OFF, 0, 0},
  {"i_slot_4",   "iFilter 4", ISS_OFF, 0, 0},
  {"i_slot_5",   "iFilter 5", ISS_OFF, 0, 0},
  {"i_slot_6",   "iFilter 6", ISS_OFF, 0, 0}
};
ISwitchVectorProperty ifilter_changeSP = {
  GALIL_DEVICE, "IFILTER_CHANGE", "Change Filter", IFILTER_GROUP, IP_RW, ISR_1OFMANY, 0.0, IPS_IDLE, ifilter_changeS, NARRAY(ifilter_changeS), "", 0
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
  GALIL_DEVICE, "IFILTER_NAMES", "Filter Names", IFILTER_GROUP, IP_RO, 0.0, IPS_IDLE, ifilterT, NARRAY(ifilterT), "", 0
};

/* ifocus group */
static ISwitch ifocus_referenceS[] = {
  {"savfoc", "Save Reference", ISS_OFF, 0, 0},
  {"resfoc", "Restore Reference", ISS_OFF, 0, 0},
  {"savnom", "Save Nominal Plane", ISS_OFF, 0, 0},
  {"resnom", "Restore Nominal Plane", ISS_OFF, 0, 0}
};
ISwitchVectorProperty ifocus_referenceSP = {
  GALIL_DEVICE, "IFOCUS_REFERENCE", "References", IFOCUS_GROUP, IP_RW, ISR_ATMOST1, 0.0, IPS_IDLE, ifocus_referenceS, NARRAY(ifocus_referenceS), "", 0
};

static INumber ifocus_distN[] = {
  {"dista", "Focus A", "%5.0f", -290.0, 2500.0, 1.0, 0.0, 0, 0, 0},
  {"distb", "Focus B", "%5.0f", -290.0, 2500.0, 1.0, 0.0, 0, 0, 0},
  {"distc", "Focus C", "%5.0f", -290.0, 2500.0, 1.0, 0.0, 0, 0, 0}
};
static INumberVectorProperty ifocus_distNP = {
  GALIL_DEVICE, "IFOCUS_DIST", "Relative Goto Individual", IFOCUS_GROUP, IP_WO, 0.0, IPS_IDLE, ifocus_distN, NARRAY(ifocus_distN), "", 0
};

static INumber ifocus_distallN[] = {
  {"distall", "Focus All", "%5.0f", -290.0, 2500.0, 1.0, 0.0, 0, 0, 0}
};
static INumberVectorProperty ifocus_distallNP = {
  GALIL_DEVICE, "IFOCUS_DISTALL", "Relative Goto All", IFOCUS_GROUP, IP_WO, 0.0, IPS_IDLE, ifocus_distallN, NARRAY(ifocus_distallN), "", 0
};

static INumber ifocus_lvdtN[] = {
  {"lvdta", "LVDT A", "%5.0f", -290.0, 2500.0, 1.0, 0.0, 0, 0, 0},
  {"lvdtb", "LVDT B", "%5.0f", -290.0, 2500.0, 1.0, 0.0, 0, 0, 0},
  {"lvdtc", "LVDT C", "%5.0f", -290.0, 2500.0, 1.0, 0.0, 0, 0, 0}
};
static INumberVectorProperty ifocus_lvdtNP = {
  GALIL_DEVICE, "IFOCUS_LVDT", "Goto LVDT Values", IFOCUS_GROUP, IP_WO, 0.0, IPS_IDLE, ifocus_lvdtN, NARRAY(ifocus_lvdtN), "", 0
};

/* support group */
static IText supportT[] = {
  {"author",  "Author",  supports.author,  0, 0, 0},
  {"email",   "Email",   supports.email,   0, 0, 0},
  {"date",    "Date",    supports.date,    0, 0, 0},
  {"version", "Version", supports.version, 0, 0, 0}
};
static ITextVectorProperty supportTP = {
  GALIL_DEVICE, "SUPPORT", "Support Available From", SUPPORT_GROUP, IP_RO, 0, IPS_IDLE, supportT, NARRAY(supportT), "", 0
};

static IText telemetry_referenceT[] = {
  {"a_reference",  "A Focus Reference", telemetrys.a_reference,  0, 0, 0},
  {"b_reference",  "B Focus Reference", telemetrys.b_reference,  0, 0, 0},
  {"c_reference",  "C Focus Reference", telemetrys.c_reference,  0, 0, 0}
};
static ITextVectorProperty telemetry_referenceTP = {
  GALIL_DEVICE, "TELEMETRY_REFERENCE", "References", TELEMETRY_GROUP, IP_RO, 0, IPS_IDLE, telemetry_referenceT, NARRAY(telemetry_referenceT), "", 0
};

/* telemetry group */
// Wanted to break up into different groups for display purposes
static IText telemetryT[] = {
  {"hardware",     "Hardware Version                  ", telemetrys.hardware,     0, 0, 0},
  {"software",     "Software Version                  ", telemetrys.software,     0, 0, 0},
  {"timestamp",    "Timestamp                         ", telemetrys.timestamp,    0, 0, 0},
  {"julian",       "Julian Date                       ", telemetrys.jd,           0, 0, 0},
  {"tcp_shm_mode", "TCP (Shared Memory) Mode          ", telemetrys.tcp_shm_mode, 0, 0, 0},
  {"udp_shm_mode", "UDP (Shared Memory) Mode          ", telemetrys.udp_shm_mode, 0, 0, 0},
  {"a_encoder",    "A Encoder (A axis motor position) ", telemetrys.a_encoder,    0, 0, 0},
  {"b_encoder",    "B Encoder (B axis motor position) ", telemetrys.b_encoder,    0, 0, 0},
  {"c_encoder",    "C Encoder (C axis motor position) ", telemetrys.c_encoder,    0, 0, 0},
  {"a_position",   "A Position (B axis analog in)     ", telemetrys.a_position,   0, 0, 0},
  {"b_position",   "B Position (D axis analog in)     ", telemetrys.b_position,   0, 0, 0},
  {"c_position",   "C Position (F axis analog in)     ", telemetrys.c_position,   0, 0, 0},
  {"a_reference",  "A Focus Reference                 ", telemetrys.a_reference,  0, 0, 0},
  {"b_reference",  "B Focus Reference                 ", telemetrys.b_reference,  0, 0, 0},
  {"c_reference",  "C Focus Reference                 ", telemetrys.c_reference,  0, 0, 0},
  {"distall",      "distall                           ", telemetrys.distall,      0, 0, 0},
  {"dista",        "dista                             ", telemetrys.dista,        0, 0, 0},
  {"distb",        "distb                             ", telemetrys.distb,        0, 0, 0},
  {"distc",        "distc                             ", telemetrys.distc,        0, 0, 0},
  {"distgcam",     "distgcam (gfocus)                 ", telemetrys.distgcam,     0, 0, 0},
  {"errfilt",      "errfilt (1.0=Error, 0.0=OK)       ", telemetrys.errfilt,      0, 0, 0},
  {"filtisin",     "filtisin  (1.0=True, 0.0=False)   ", telemetrys.filtisin,     0, 0, 0},
  {"ifilter_0",    "iFilter 1 (filtvals[0])           ", telemetrys.ifilter_1,    0, 0, 0},
  {"ifilter_1",    "iFilter 2 (filtvals[1])           ", telemetrys.ifilter_2,    0, 0, 0},
  {"ifilter_2",    "iFilter 3 (filtvals[2])           ", telemetrys.ifilter_3,    0, 0, 0},
  {"ifilter_3",    "iFilter 4 (filtvals[3])           ", telemetrys.ifilter_4,    0, 0, 0},
  {"ifilter_4",    "iFilter 5 (filtvals[4])           ", telemetrys.ifilter_5,    0, 0, 0},
  {"ifilter_5",    "iFilter 6 (filtvals[5])           ", telemetrys.ifilter_6,    0, 0, 0},
  {"reqfilt",      "reqfilt (requested filter)        ", telemetrys.reqfilt,      0, 0, 0},
  {"filtval",      "filtval (selected filter)         ", telemetrys.filtval,      0, 0, 0},
  {"gfiltn",       "gfiltn                            ", telemetrys.gfiltn,       0, 0, 0},
  {"gfilter_1",    "gFilter 1                         ", telemetrys.gfilter_1,    0, 0, 0},
  {"gfilter_2",    "gFilter 2                         ", telemetrys.gfilter_2,    0, 0, 0},
  {"gfilter_3",    "gFilter 3                         ", telemetrys.gfilter_3,    0, 0, 0},
  {"gfilter_4",    "gFilter 4                         ", telemetrys.gfilter_4,    0, 0, 0},
  {"gfilter_5",    "gFilter 5                         ", telemetrys.gfilter_5,    0, 0, 0},
  {"gfilter_6",    "gFilter 6                         ", telemetrys.gfilter_6,    0, 0, 0}
};
static ITextVectorProperty telemetryTP = {
  GALIL_DEVICE, "Telemetry", "Telemetry",  TELEMETRY_GROUP, IP_RO, 0, IPS_IDLE, telemetryT, NARRAY(telemetryT), "", 0
};

static ILight telemetry_ifilterwheelL[] = {
  {"fout", "Filter Out", ISS_OFF, 0, 0},
  {"frot", "FW Rotating", ISS_OFF, 0, 0},
  {"flin", "Filter Translating", ISS_OFF, 0, 0},
  {"fin", "Filter In", ISS_OFF, 0, 0},
  {"ferr", "FW Error", ISS_OFF, 0, 0}
};
ILightVectorProperty telemetry_ifilterwheelLP = {
  GALIL_DEVICE, "FW_LIGHTS", "Filterwheel Status", TELEMETRY_GROUP, IPS_IDLE, telemetry_ifilterwheelL, NARRAY(telemetry_ifilterwheelL), "", 0
};

static ILight telemetry_gfilterwheelL[] = {
  {"grot",   "GW Rotating", ISS_OFF, 0, 0}
};
ILightVectorProperty telemetry_gfilterwheelLP = {
  GALIL_DEVICE, "GW_LIGHTS", "Guiderwheel Status", TELEMETRY_GROUP, IPS_IDLE, telemetry_gfilterwheelL, NARRAY(telemetry_gfilterwheelL), "", 0
};

static ILight telemetry_connectionL[] = {
  {"tcp",    "TCP Shared Memory", ISS_OFF, 0, 0},
  {"udp",    "UDP Shared Memory", ISS_OFF, 0, 0},
};
ILightVectorProperty telemetry_connectionLP = {
  GALIL_DEVICE, "SHARED_MEMORY_LIGHTS", "Shared Memory", TELEMETRY_GROUP, IPS_IDLE, telemetry_connectionL, NARRAY(telemetry_connectionL), "", 0
};

static ILight telemetry_lightsL[] = {
  {"swbusy", "Software Executing Procedure ", ISS_OFF, 0, 0},
  {"hwbusy", "Hardware Executing Procedure ", ISS_OFF, 0, 0}
  
};
ILightVectorProperty telemetry_lightsLP = {
  GALIL_DEVICE, "TELEMETRY", "Galil Status", TELEMETRY_GROUP, IPS_IDLE, telemetry_lightsL, NARRAY(telemetry_lightsL), "", 0
};


/*******************************************************************************
 * INDI hook: ISGetProperties()
 ******************************************************************************/
void ISGetProperties(const char *dev) {

  /* check device */
  if (dev && strcmp(GALIL_DEVICE, dev)) return;

  /* define widget(s) */
  IDDefLight(&telemetry_connectionLP, NULL);
  IDDefLight(&telemetry_gfilterwheelLP, NULL);
  IDDefLight(&telemetry_lightsLP, NULL);
  IDDefLight(&telemetry_ifilterwheelLP, NULL);
  IDDefText(&telemetry_referenceTP, NULL);
  IDDefText(&telemetryTP, NULL);
  IDDefSwitch(&ifilter_startupSP, NULL);
  IDDefSwitch(&ifilterSP, NULL);
  IDDefSwitch(&ifilter_changeSP, NULL);
  IDDefText(&ifilterTP, NULL);
  IDDefSwitch(&ifocus_referenceSP, NULL);
  IDDefNumber(&ifocus_distNP, NULL);
  IDDefNumber(&ifocus_distallNP, NULL);
  IDDefNumber(&ifocus_lvdtNP, NULL);
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

  /* focus dist value(s) */
  if (!strcmp(name, ifocus_distNP.name)) {
    float dista = values[0];
    float distb = values[1];
    float distc = values[2];
    busy = true;
    IDMessage(GALIL_DEVICE, "Calling xq_focusind(a=%.1f, b=%.1f, c=%.1f)", dista, distb, distc);
    if ((gstat=xq_focusind(dista, distb, distc)) == G_NO_ERROR) {
      IDMessage(GALIL_DEVICE, "Called xq_focusind(a=%.1f, b=%.1f, c=%.1f) OK", dista, distb, distc);
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_focusind(a=%.1f, b=%.1f, c=%.1f)", dista, distb, distc);
    }
    busy = false;
    ifocus_distNP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
    ifocus_distNP.np[0].value = dista;
    ifocus_distNP.np[1].value = distb;
    ifocus_distNP.np[2].value = distc;
    IDSetNumber(&ifocus_distNP, NULL);

  /* focus distall value(s) */
  } else if (!strcmp(name, ifocus_distallNP.name)) {
    float distall = values[0];
    busy = true;
    IDMessage(GALIL_DEVICE, "Calling xq_focusall(a=%.1f)", distall);
    if ((gstat=xq_focusall(distall)) == G_NO_ERROR) {
      IDMessage(GALIL_DEVICE, "Called xq_focusall(a=%.1f) OK", distall);
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_focusall(a=%.1f)", distall);
    }
    busy = false;
    ifocus_distallNP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
    ifocus_distallNP.np[0].value = distall;
    IDSetNumber(&ifocus_distallNP, NULL);

  /* focus lvdt value(s) */
  } else if (!strcmp(name, ifocus_lvdtNP.name)) {
    float dista = round((values[0]*1000.0 + ifoci.vala) * BOK_LVDT_ATOD);
    float distb = round((values[1]*1000.0 + ifoci.valb) * BOK_LVDT_ATOD);
    float distc = round((values[2]*1000.0 + ifoci.valc) * BOK_LVDT_ATOD);
    busy = true;
    IDMessage(GALIL_DEVICE, "Calling xq_focusind(a=%.1f, b=%.1f, c=%.1f)", dista, distb, distc);
    if ((gstat=xq_focusind(dista, distb, distc)) == G_NO_ERROR) {
      IDMessage(GALIL_DEVICE, "Called xq_focusind(a=%.1f, b=%.1f, c=%.1f) OK", dista, distb, distc);
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_focusind(a=%.1f, b=%.1f, c=%.1f)", dista, distb, distc);
    }
    busy = false;
    ifocus_lvdtNP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
    ifocus_lvdtNP.np[0].value = values[0];
    ifocus_lvdtNP.np[1].value = values[1];
    ifocus_lvdtNP.np[2].value = values[2];
    IDSetNumber(&ifocus_lvdtNP, NULL);

  /* gfocus dist value */
  } else if (!strcmp(name, gfocus_distNP.name)) {
    float distgcam = values[0];
    busy = true;
    IDMessage(GALIL_DEVICE, "Calling xq_gfocus(a=%.1f)", distgcam);
    if ((gstat=xq_gfocus(distgcam)) == G_NO_ERROR) {
      IDMessage(GALIL_DEVICE, "Called xq_gfocus(a=%.1f) OK", distgcam);
    } else {
      IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_gfocus(a=%.1f)", distgcam);
    }
    busy = false;
    gfocus_distNP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
    gfocus_distNP.np[0].value = distgcam;
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
  if (! strcmp(name, ifilter_startupSP.name)) {
    execute_ifilter_startup(states, names, n);
    IUResetSwitch(&ifilter_startupSP);
    IDSetSwitch(&ifilter_startupSP, NULL);
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

  /* check status */
  if (initialized == true) { return; }

  /* output some message(s) */
  IDMessage(GALIL_DEVICE, "Initializing driver v%s, %s (%s), %s", _VERSION_, _AUTHOR_, _EMAIL_, _DATE_);

  /* read instrument filter list from file */
  for (int i=0; i<BOK_IFILTER_SLOTS; i++) { (void) memset((void *)&bok_ifilters[i], 0, sizeof(filter_file_t)); }
  read_filters_from_file(BOK_IFILTER_FILE, (filter_file_t *)bok_ifilters, BOK_IFILTER_SLOTS, BOK_IFILTER_COLUMNS);
  for (int j=0; j<BOK_IFILTER_SLOTS; j++) {
    if (strlen(bok_ifilters[j].code)>0 && strlen(bok_ifilters[j].name)>0) {
      (void) fprintf(stderr, "instrument filters> index=%d, code='%s', name='%s'\n", j, bok_ifilters[j].code, bok_ifilters[j].name);
      (void) fflush(stderr);
    }
  }

  /* read guider filter list from file */
  for (int j=0; j<BOK_GFILTER_SLOTS; j++) { (void) memset((void *)&bok_gfilters[j], 0, sizeof(filter_file_t)); }
  read_filters_from_file(BOK_GFILTER_FILE, (filter_file_t *)bok_gfilters, BOK_GFILTER_SLOTS, BOK_GFILTER_COLUMNS);
  for (int j=0; j<BOK_GFILTER_SLOTS; j++) {
    if (strlen(bok_gfilters[j].code)>0 && strlen(bok_gfilters[j].name)>0) {
      (void) fprintf(stderr, "guider filters> index=%d, code='%s', name='%s'\n", j, bok_gfilters[j].code, bok_gfilters[j].name);
      (void) fflush(stderr);
      
    }
  }

  /* zero out telemetry values */
  zero_telemetry();

  /* add a background timer to wakeup the named function */
  if (timer_id < 0) {
    IDMessage(GALIL_DEVICE, "adding execute_timer(), timer_id=%d, initialized=%d", timer_id, initialized);
    if ((timer_id=IEAddTimer(BOK_TCP_DELAY_MS, execute_timer, (void *)NULL)) >= 0) {
      initialized = true;
      IDMessage(GALIL_DEVICE, "added execute_timer(), timer_id=%d, initialized=%d", timer_id, initialized);
    }
  }

  /* should we do a populate / popdone / read filter wheel / initialize guider filter wheel here? */
}

/*******************************************************************************
 * action: execute_gfilter_change()
 ******************************************************************************/
void execute_gfilter_change(ISState states[], char *names[], int n) {
  /* declare some variables and initialize them */
  GReturn gstat = (GReturn)0;
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
      if (tcp_val.lv.gfiltn == 1.0) {
        IDMessage(GALIL_DEVICE, "gFilter is already selected!");
        gfilter_changeSP.s = IPS_OK;
      } else {
        IDMessage(GALIL_DEVICE, "Executing 'gFilter 1'");
        IDMessage(GALIL_DEVICE, "Calling xq_gfiltn(1.0)");
        busy = true;
        if ((gstat=xq_gfiltn(1.0)) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_gfiltn(1.0) OK");
          IDMessage(GALIL_DEVICE, "Calling xq_gfwmov()");
          if ((gstat=xq_gfwmov()) == G_NO_ERROR) {
            IDMessage(GALIL_DEVICE, "Called xq_gfwmov() OK");
            IDMessage(GALIL_DEVICE, "Executed 'gFilter 1' OK");
          } else {
            IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_gfwmov(), gstat=%d", gstat);
          }
          gfilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_gfiltn(1.0), gstat=%d", gstat);
          gfilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        }
        busy = false;
      }
      gfilter_changeS[0].s = ISS_OFF;

    /* process 'gFilter 2' */
    } else if (sp == &gfilter_changeS[1]) {
      if (tcp_val.lv.gfiltn == 2.0) {
        IDMessage(GALIL_DEVICE, "gFilter is already selected!");
        gfilter_changeSP.s = IPS_OK;
      } else {
        IDMessage(GALIL_DEVICE, "Executing 'gFilter 2'");
        IDMessage(GALIL_DEVICE, "Calling xq_gfiltn(2.0)");
        busy = true;
        if ((gstat=xq_gfiltn(2.0)) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_gfiltn(2.0) OK");
          IDMessage(GALIL_DEVICE, "Calling xq_gfwmov()");
          if ((gstat=xq_gfwmov()) == G_NO_ERROR) {
            IDMessage(GALIL_DEVICE, "Called xq_gfwmov() OK");
            IDMessage(GALIL_DEVICE, "Executed 'gFilter 2' OK");
          } else {
            IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_gfwmov(), gstat=%d", gstat);
          }
          gfilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_gfiltn(2.0), gstat=%d", gstat);
          gfilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        }
        busy = false;
      }
      gfilter_changeS[1].s = ISS_OFF;

    /* process 'gFilter 3' */
    } else if (sp == &gfilter_changeS[2]) {
      if (tcp_val.lv.gfiltn == 3.0) {
        IDMessage(GALIL_DEVICE, "gFilter is already selected!");
        gfilter_changeSP.s = IPS_OK;
      } else {
        IDMessage(GALIL_DEVICE, "Executing 'gFilter 3'");
        IDMessage(GALIL_DEVICE, "Calling xq_gfiltn(3.0)");
        busy = true;
        if ((gstat=xq_gfiltn(3.0)) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_gfiltn(3.0) OK");
          IDMessage(GALIL_DEVICE, "Calling xq_gfwmov()");
          if ((gstat=xq_gfwmov()) == G_NO_ERROR) {
            IDMessage(GALIL_DEVICE, "Called xq_gfwmov() OK");
            IDMessage(GALIL_DEVICE, "Executed 'gFilter 3' OK");
          } else {
            IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_gfwmov(), gstat=%d", gstat);
          }
          gfilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_gfiltn(3.0), gstat=%d", gstat);
          gfilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        }
        busy = false;
      }
      gfilter_changeS[2].s = ISS_OFF;

    /* process 'gFilter 4' */
    } else if (sp == &gfilter_changeS[3]) {
      if (tcp_val.lv.gfiltn == 4.0) {
        IDMessage(GALIL_DEVICE, "gFilter is already selected!");
        gfilter_changeSP.s = IPS_OK;
      } else {
        IDMessage(GALIL_DEVICE, "Executing 'gFilter 4'");
        IDMessage(GALIL_DEVICE, "Calling xq_gfiltn(4.0)");
        busy = true;
        if ((gstat=xq_gfiltn(4.0)) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_gfiltn(4.0) OK");
          IDMessage(GALIL_DEVICE, "Calling xq_gfwmov()");
          if ((gstat=xq_gfwmov()) == G_NO_ERROR) {
            IDMessage(GALIL_DEVICE, "Called xq_gfwmov() OK");
            IDMessage(GALIL_DEVICE, "Executed 'gFilter 4' OK");
          } else {
            IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_gfwmov(), gstat=%d", gstat);
          }
          gfilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_gfiltn(4.0), gstat=%d", gstat);
          gfilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        }
        busy = false;
      }
      gfilter_changeS[3].s = ISS_OFF;

    /* process 'gFilter 5' */
    } else if (sp == &gfilter_changeS[4]) {
      if (tcp_val.lv.gfiltn == 5.0) {
        IDMessage(GALIL_DEVICE, "gFilter is already selected!");
        gfilter_changeSP.s = IPS_OK;
      } else {
        IDMessage(GALIL_DEVICE, "Executing 'gFilter 5'");
        IDMessage(GALIL_DEVICE, "Calling xq_gfiltn(5.0)");
        busy = true;
        if ((gstat=xq_gfiltn(5.0)) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_gfiltn(5.0) OK");
          IDMessage(GALIL_DEVICE, "Calling xq_gfwmov()");
          if ((gstat=xq_gfwmov()) == G_NO_ERROR) {
            IDMessage(GALIL_DEVICE, "Called xq_gfwmov() OK");
            IDMessage(GALIL_DEVICE, "Executed 'gFilter 5' OK");
          } else {
            IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_gfwmov(), gstat=%d", gstat);
          }
          gfilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_gfiltn(5.0), gstat=%d", gstat);
          gfilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        }
        busy = false;
      }
      gfilter_changeS[4].s = ISS_OFF;

    /* process 'gFilter 6' */
    } else if (sp == &gfilter_changeS[5]) {
      if (tcp_val.lv.gfiltn == 6.0) {
        IDMessage(GALIL_DEVICE, "gFilter is already selected!");
        gfilter_changeSP.s = IPS_OK;
      } else {
        IDMessage(GALIL_DEVICE, "Executing 'gFilter 6'");
        IDMessage(GALIL_DEVICE, "Calling xq_gfiltn(6.0)");
        busy = true;
        if ((gstat=xq_gfiltn(6.0)) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_gfiltn(6.0) OK");
          IDMessage(GALIL_DEVICE, "Calling xq_gfwmov()");
          if ((gstat=xq_gfwmov()) == G_NO_ERROR) {
            IDMessage(GALIL_DEVICE, "Called xq_gfwmov() OK");
            IDMessage(GALIL_DEVICE, "Executed 'gFilter 6' OK");
          } else {
            IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_gfwmov(), gstat=%d", gstat);
          }
          gfilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_gfiltn(6.0), gstat=%d", gstat);
          gfilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        }
        busy = false;
      }
      gfilter_changeS[5].s = ISS_OFF;
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
      IDMessage(GALIL_DEVICE, "Executing 'gFilter Initialize'");
      IDMessage(GALIL_DEVICE, "Calling xq_gfwinit()");
      busy = true;
      if ((gstat=xq_gfwinit()) == G_NO_ERROR) {
        IDMessage(GALIL_DEVICE, "Called xq_gfwinit() OK");
        IDMessage(GALIL_DEVICE, "Executed 'gFilter Initialize' OK");
      } else {
        IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_gfwinit(), gstat=%d", gstat);
      }
      gfilterSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
      busy = false;
      gfilterS[0].s = ISS_OFF;
    }
  }

  /* reset */
  IUResetSwitch(&gfilterSP);
  IDSetSwitch(&gfilterSP, NULL);
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
      if (tcp_val.lv.filtisin == 1.0) {
        IDMessage(GALIL_DEVICE, "Cannot change filter whilst another filter is in the beam!");
        ifilter_changeSP.s = IPS_BUSY;
      } else if (tcp_val.lv.filtval==tcp_val.lv.reqfilt && tcp_val.lv.reqfilt==tcp_val.filtvals[0]) {
        IDMessage(GALIL_DEVICE, "iFilter is already selected!");
        ifilter_changeSP.s = IPS_OK;
      } else {
        IDMessage(GALIL_DEVICE, "Executing 'iFilter 1'");
        IDMessage(GALIL_DEVICE, "Calling xq_reqfilt(%.1f)", tcp_val.filtvals[0]);
        busy = true;
        if ((gstat=xq_reqfilt(tcp_val.filtvals[0])) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_reqfilt(%.1f) OK", tcp_val.filtvals[0]);
          IDMessage(GALIL_DEVICE, "Calling xq_filtmov()");
          if ((gstat=xq_filtmov()) == G_NO_ERROR) {
            IDMessage(GALIL_DEVICE, "Called xq_filtmov() OK");
            IDMessage(GALIL_DEVICE, "Executed 'iFilter 1' OK");
          } else {
            IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_filtmov(), gstat=%d", gstat);
          }
          ifilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_reqfilt(%.1f), gstat=%d", tcp_val.filtvals[0], gstat);
          ifilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        }
        busy = false;
      }
      ifilter_changeS[0].s = ISS_OFF;

    /* process 'iFilter 2' - NB: it's up to the higher-level software to check telemetry */
    } else if (sp == &ifilter_changeS[1]) {
      if (tcp_val.lv.filtisin == 1.0) {
        IDMessage(GALIL_DEVICE, "Cannot change filter whilst another filter is in the beam!");
        ifilter_changeSP.s = IPS_BUSY;
      } else if (tcp_val.lv.filtval==tcp_val.lv.reqfilt && tcp_val.lv.reqfilt == tcp_val.filtvals[1]) {
        IDMessage(GALIL_DEVICE, "iFilter is already selected!");
        ifilter_changeSP.s = IPS_OK;
      } else {
        IDMessage(GALIL_DEVICE, "Executing 'iFilter 2'");
        IDMessage(GALIL_DEVICE, "Calling xq_reqfilt(%.1f)", tcp_val.filtvals[1]);
        busy = true;
        if ((gstat=xq_reqfilt(tcp_val.filtvals[1])) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_reqfilt(%.1f) OK", tcp_val.filtvals[1]);
          IDMessage(GALIL_DEVICE, "Calling xq_filtmov()");
          if ((gstat=xq_filtmov()) == G_NO_ERROR) {
            IDMessage(GALIL_DEVICE, "Called xq_filtmov() OK");
            IDMessage(GALIL_DEVICE, "Executed 'iFilter 2' OK");
          } else {
            IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_filtmov(), gstat=%d", gstat);
          }
          ifilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_reqfilt(%.1f), gstat=%d", tcp_val.filtvals[1], gstat);
          ifilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        }
        busy = false;
      }
      ifilter_changeS[1].s = ISS_OFF;

    /* process 'iFilter 3' - NB: it's up to the higher-level software to check telemetry */
    } else if (sp == &ifilter_changeS[2]) {
      if (tcp_val.lv.filtisin == 1.0) {
        IDMessage(GALIL_DEVICE, "Cannot change filter whilst another filter is in the beam!");
        ifilter_changeSP.s = IPS_BUSY;
      } else if (tcp_val.lv.filtval==tcp_val.lv.reqfilt && tcp_val.lv.reqfilt == tcp_val.filtvals[2]) {
        IDMessage(GALIL_DEVICE, "iFilter is already selected!");
        ifilter_changeSP.s = IPS_OK;
      } else {
        IDMessage(GALIL_DEVICE, "Executing 'iFilter 3'");
        IDMessage(GALIL_DEVICE, "Calling xq_reqfilt(%.1f)", tcp_val.filtvals[2]);
        busy = true;
        if ((gstat=xq_reqfilt(tcp_val.filtvals[2])) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_reqfilt(%.1f) OK", tcp_val.filtvals[2]);
          IDMessage(GALIL_DEVICE, "Calling xq_filtmov()");
          if ((gstat=xq_filtmov()) == G_NO_ERROR) {
            IDMessage(GALIL_DEVICE, "Called xq_filtmov() OK");
            IDMessage(GALIL_DEVICE, "Executed 'iFilter 3' OK");
          } else {
            IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_filtmov(), gstat=%d", gstat);
          }
          ifilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_reqfilt(%.1f), gstat=%d", tcp_val.filtvals[2], gstat);
          ifilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        }
        busy = false;
      }
      ifilter_changeS[2].s = ISS_OFF;

    /* process 'iFilter 4' - NB: it's up to the higher-level software to check telemetry */
    } else if (sp == &ifilter_changeS[3]) {
      if (tcp_val.lv.filtisin == 1.0) {
        IDMessage(GALIL_DEVICE, "Cannot change filter whilst another filter is in the beam!");
        ifilter_changeSP.s = IPS_BUSY;
      } else if (tcp_val.lv.filtval==tcp_val.lv.reqfilt && tcp_val.lv.reqfilt == tcp_val.filtvals[3]) {
        IDMessage(GALIL_DEVICE, "iFilter is already selected!");
        ifilter_changeSP.s = IPS_OK;
      } else {
        IDMessage(GALIL_DEVICE, "Executing 'iFilter 4'");
        IDMessage(GALIL_DEVICE, "Calling xq_reqfilt(%.1f)", tcp_val.filtvals[3]);
        busy = true;
        if ((gstat=xq_reqfilt(tcp_val.filtvals[3])) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_reqfilt(%.1f) OK", tcp_val.filtvals[3]);
          IDMessage(GALIL_DEVICE, "Calling xq_filtmov()");
          if ((gstat=xq_filtmov()) == G_NO_ERROR) {
            IDMessage(GALIL_DEVICE, "Called xq_filtmov() OK");
            IDMessage(GALIL_DEVICE, "Executed 'iFilter 4' OK");
          } else {
            IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_filtmov(), gstat=%d", gstat);
          }
          ifilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_reqfilt(%.1f), gstat=%d", tcp_val.filtvals[3], gstat);
          ifilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        }
        busy = false;
      }
      ifilter_changeS[3].s = ISS_OFF;

    /* process 'iFilter 5' - NB: it's up to the higher-level software to check telemetry */
    } else if (sp == &ifilter_changeS[4]) {
      if (tcp_val.lv.filtisin == 1.0) {
        IDMessage(GALIL_DEVICE, "Cannot change filter whilst another filter is in the beam!");
        ifilter_changeSP.s = IPS_BUSY;
      } else if (tcp_val.lv.filtval==tcp_val.lv.reqfilt && tcp_val.lv.reqfilt == tcp_val.filtvals[4]) {
        IDMessage(GALIL_DEVICE, "iFilter is already selected!");
        ifilter_changeSP.s = IPS_OK;
      } else {
        IDMessage(GALIL_DEVICE, "Executing 'iFilter 5'");
        IDMessage(GALIL_DEVICE, "Calling xq_reqfilt(%.1f)", tcp_val.filtvals[4]);
        busy = true;
        if ((gstat=xq_reqfilt(tcp_val.filtvals[4])) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_reqfilt(%.1f) OK", tcp_val.filtvals[4]);
          IDMessage(GALIL_DEVICE, "Calling xq_filtmov()");
          if ((gstat=xq_filtmov()) == G_NO_ERROR) {
            IDMessage(GALIL_DEVICE, "Called xq_filtmov() OK");
            IDMessage(GALIL_DEVICE, "Executed 'iFilter 5' OK");
          } else {
            IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_filtmov(), gstat=%d", gstat);
          }
          ifilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_reqfilt(%.1f), gstat=%d", tcp_val.filtvals[4], gstat);
          ifilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        }
        busy = false;
      }
      ifilter_changeS[4].s = ISS_OFF;

    /* process 'iFilter 6' - NB: it's up to the higher-level software to check telemetry */
    } else if (sp == &ifilter_changeS[5]) {
      if (tcp_val.lv.filtisin == 1.0) {
        IDMessage(GALIL_DEVICE, "Cannot change filter whilst another filter is in the beam!");
        ifilter_changeSP.s = IPS_BUSY;
      } else if (tcp_val.lv.filtval==tcp_val.lv.reqfilt && tcp_val.lv.reqfilt == tcp_val.filtvals[5]) {
        IDMessage(GALIL_DEVICE, "iFilter is already selected!");
        ifilter_changeSP.s = IPS_OK;
      } else {
        IDMessage(GALIL_DEVICE, "Executing 'iFilter 6'");
        IDMessage(GALIL_DEVICE, "Calling xq_reqfilt(%.1f)", tcp_val.filtvals[5]);
        busy = true;
        if ((gstat=xq_reqfilt(tcp_val.filtvals[5])) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_reqfilt(%.1f) OK", tcp_val.filtvals[5]);
          IDMessage(GALIL_DEVICE, "Calling xq_filtmov()");
          if ((gstat=xq_filtmov()) == G_NO_ERROR) {
            IDMessage(GALIL_DEVICE, "Called xq_filtmov() OK");
            IDMessage(GALIL_DEVICE, "Executed 'iFilter 6' OK");
          } else {
            IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_filtmov(), gstat=%d", gstat);
          }
          ifilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_reqfilt(%.1f), gstat=%d", tcp_val.filtvals[5], gstat);
          ifilter_changeSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        }
        busy = false;
      }
      ifilter_changeS[5].s = ISS_OFF;
    }
  }

  /* reset */
  IUResetSwitch(&ifilter_changeSP);
  IDSetSwitch(&ifilter_changeSP, NULL);
}
/*******************************************************************************
 * action: execute_ifilter_startup()
 ******************************************************************************/
void execute_ifilter_startup(ISState states[], char *names[], int n) {

  /* declare some variables and initialize them */
  GReturn gstat = (GReturn)0;
  ISwitch *sp = (ISwitch *)NULL;
  ISState state = (ISState)NULL;
  bool state_change = false;

  /* find switches with the passed names in the ifilter_startupSP property */
  for (int i=0; i < n; i++) {
    sp = IUFindSwitch(&ifilter_startupSP, names[i]);
    state = states[i];
    state_change = state != sp->s;
    if (! state_change) { continue; }

    /* process 'iFilter Populate' - NB: it's up to the higher-level software to check telemetry */
    if (sp == &ifilter_startupS[0]) {
      if (tcp_val.lv.filtisin == 1.0) {
        IDMessage(GALIL_DEVICE, "Cannot populate whilst the filter is in the beam!");
        ifilter_startupSP.s = IPS_OK;
      } else {
        IDMessage(GALIL_DEVICE, "Executing 'iFilter Populate'");
        IDMessage(GALIL_DEVICE, "Calling xq_filtldm()");
        busy = true;
        if ((gstat=xq_filtldm()) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_filtldm() OK");
          IDMessage(GALIL_DEVICE, "Executed 'iFilter Populate' OK");
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_filtldm(), gstat=%d", gstat);
        }
        ifilter_startupSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        busy = false;
      }
      ifilter_startupS[0].s = ISS_OFF;

    /* process 'iFilter PopDone' - NB: it's up to the higher-level software to check telemetry */
    } else if (sp == &ifilter_startupS[1]) {
      IDMessage(GALIL_DEVICE, "Executing 'iFilter PopDone'");
      IDMessage(GALIL_DEVICE, "Calling xq_hx()");
      busy = true;
      if ((gstat=xq_hx()) == G_NO_ERROR) {
        IDMessage(GALIL_DEVICE, "Called xq_hx() OK");
        IDMessage(GALIL_DEVICE, "Executing 'iFilter PopDone' OK");
      } else {
        IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_hx(), gstat=%d", gstat);
      }
      busy = false;
      ifilter_startupSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
      ifilter_startupS[1].s = ISS_OFF;

    /* process 'iFilter ReadWheel' - NB: it's up to the higher-level software to check telemetry */
    } else if (sp == &ifilter_startupS[2]) {
      if (tcp_val.lv.filtisin == 1.0) {
        IDMessage(GALIL_DEVICE, "Cannot read wheel whilst the filter is in the beam!");
        ifilter_startupSP.s = IPS_OK;
      } else {
        IDMessage(GALIL_DEVICE, "Executing 'iFilter ReadWheel'");
        IDMessage(GALIL_DEVICE, "Calling xq_filtrd()");
        busy = true;
        if ((gstat=xq_filtrd()) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_filtrd() OK");
          IDMessage(GALIL_DEVICE, "Executed 'iFilter ReadWheel' OK");
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_filtrd(), gstat=%d", gstat);
        }
        ifilter_startupSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        busy = false;
      }
      ifilter_startupS[2].s = ISS_OFF;

    /* process 'iFilter Initialize' - NB: it's up to the higher-level software to check telemetry */
    } else if (sp == &ifilter_startupS[3]) {
      if (tcp_val.lv.filtisin == 1.0) {
        IDMessage(GALIL_DEVICE, "Cannot initialize whilst the filter is in the beam!");
        ifilter_startupSP.s = IPS_OK;
      } else {
        IDMessage(GALIL_DEVICE, "Executing 'iFilter Initialize'");
        IDMessage(GALIL_DEVICE, "Calling xq_filtrd()");
        busy = true;
        if ((gstat=xq_filtrd()) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_filtrd() OK");
          IDMessage(GALIL_DEVICE, "Executed 'iFilter Initialize' OK");
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_filtrd(), gstat=%d", gstat);
        }
        ifilter_startupSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        busy = false;
      }
      ifilter_startupS[3].s = ISS_OFF;
    }
  }

  /* reset */
  IUResetSwitch(&ifilter_startupSP);
  IDSetSwitch(&ifilter_startupSP, NULL);
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
        IDMessage(GALIL_DEVICE, "Filter is already in the beam!");
        ifilterSP.s = IPS_OK;
      } else {
        IDMessage(GALIL_DEVICE, "Executing 'iFilter Load'");
        IDMessage(GALIL_DEVICE, "Calling xq_filtin()");
        busy = true;
        if ((gstat=xq_filtin()) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_filtin() OK");
          IDMessage(GALIL_DEVICE, "Executed 'iFilter Load' OK");
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_filtin(), gstat=%d", gstat);
        }
        ifilterSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        busy = false;
      }
      ifilterS[0].s = ISS_OFF;

    /* process 'iFilter Unload' - NB: it's up to the higher-level software to check telemetry */
    } else if (sp == &ifilterS[1]) {
      if (tcp_val.lv.filtisin == 0.0) {
        IDMessage(GALIL_DEVICE, "Filter is already out of the beam!");
        ifilterSP.s = IPS_OK;
      } else {
        IDMessage(GALIL_DEVICE, "Executing 'iFilter Unload'");
        IDMessage(GALIL_DEVICE, "Calling xq_filtout()");
        busy = true;
        if ((gstat=xq_filtout()) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_filtout() OK");
          IDMessage(GALIL_DEVICE, "Executed 'iFilter Unload' OK");
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_filtout(), gstat=%d", gstat);
        }
        ifilterSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
        busy = false;
      }
      ifilterS[1].s = ISS_OFF;
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
  float delta_a = 0.0;
  float delta_b = 0.0;
  float delta_c = 0.0;
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
      IDMessage(GALIL_DEVICE, "Executing 'Save Focus Reference'");
      ifoci.refa = (float)udp_val.baxis_analog_in * BOK_LVDT_STEPS;
      ifoci.refb = (float)udp_val.daxis_analog_in * BOK_LVDT_STEPS;
      ifoci.refc = (float)udp_val.faxis_analog_in * BOK_LVDT_STEPS;
      IDMessage(GALIL_DEVICE, "Executed 'Save Focus Reference' a=%.3f, b=%.3f, c=%.3f OK", ifoci.refa, ifoci.refb, ifoci.refc);
      ifocus_referenceSP.s = IPS_OK;
      ifocus_referenceS[0].s = ISS_OFF;

    /* process 'Restore Focus Reference' */
    } else if (sp == &ifocus_referenceS[1]) {
      if (ifoci.refa == BOK_BAD_FLOAT) {
        IDMessage(GALIL_DEVICE, "Invalid focus A reference");
      } else if (ifoci.refb == BOK_BAD_FLOAT) {
        IDMessage(GALIL_DEVICE, "Invalid focus B reference");
      } else if (ifoci.refc == BOK_BAD_FLOAT) {
        IDMessage(GALIL_DEVICE, "Invalid focus C reference");
      } else {
        IDMessage(GALIL_DEVICE, "Executing 'Restore Focus Reference'");
        delta_a = (ifoci.vala - ifoci.refa) * BOK_LVDT_ATOD;
        delta_a = round((delta_a <= 0.0) ? delta_a : -1.0 * delta_a);
        delta_b = (ifoci.valb - ifoci.refb) * BOK_LVDT_ATOD;
        delta_b = round((delta_b <= 0.0) ? delta_b : -1.0 * delta_b);
        delta_c = (ifoci.valc - ifoci.refc) * BOK_LVDT_ATOD;
        delta_c = round((delta_c <= 0.0) ? delta_c : -1.0 * delta_c);
        busy = true;
        IDMessage(GALIL_DEVICE, "Calling xq_focusind(a=%.1f, b=%.1f, c=%.1f)", delta_a, delta_b, delta_c);
        if ((gstat=xq_focusind(delta_a, delta_b, delta_c)) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_focusind(a=%.1f, b=%.1f, c=%.1f) OK", delta_a, delta_b, delta_c);
          IDMessage(GALIL_DEVICE, "Executed 'Restore Focus Reference' OK");
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_focusind(a=%.1f, b=%.1f, c=%.1f)", delta_a, delta_b, delta_c);
        }
        busy = false;
      }
      ifocus_referenceSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
      ifocus_referenceS[1].s = ISS_OFF;

    /* process 'Save Nominal Plane' */
    } else if (sp == &ifocus_referenceS[2]) {
      // nominal plane is offset from B position
      ifoci.noma = ifoci.vala - ifoci.valb;
      ifoci.nomb = ifoci.valb - ifoci.valb;
      ifoci.nomc = ifoci.valc - ifoci.valb;
      save_nominal_plane_to_file(BOK_NPLANE_FILE, ifoci.noma, ifoci.nomb, ifoci.nomc);
      ifocus_referenceSP.s = IPS_OK;
      ifocus_referenceS[2].s = ISS_OFF;

    /* process 'Restore Nominal Plane' */
    } else if (sp == &ifocus_referenceS[3]) {
      read_nominal_plane_from_file(BOK_NPLANE_FILE, &ifoci.noma, &ifoci.nomb, &ifoci.nomc);
      if (ifoci.noma == NAN) {
        IDMessage(GALIL_DEVICE, "Invalid nominal position A");
      } else if (ifoci.nomb == NAN) {
        IDMessage(GALIL_DEVICE, "Invalid nominal position B");
      } else if (ifoci.nomc == NAN) {
        IDMessage(GALIL_DEVICE, "Invalid nominal position C");
      } else {
        // nominal plane is offset from B position
        IDMessage(GALIL_DEVICE, "Executing 'Restore Nominal Plane' a=%.3f, b=%.3f, c=%.3f", ifoci.noma, ifoci.nomb, ifoci.nomc);
        ifoci.nomb += ifoci.valb;
        ifoci.noma += ifoci.nomb;
        ifoci.nomc += ifoci.nomb;
        IDMessage(GALIL_DEVICE, "Corrected nominal plane a=%.3f, b=%.3f, c=%.3f", ifoci.noma, ifoci.nomb, ifoci.nomc);
        delta_a = (ifoci.vala - ifoci.noma) * BOK_LVDT_ATOD;
        delta_a = round((delta_a <= 0.0) ? delta_a : -1.0 * delta_a);
        delta_b = (ifoci.valb - ifoci.nomb) * BOK_LVDT_ATOD;
        delta_b = round((delta_b <= 0.0) ? delta_b : -1.0 * delta_b);
        delta_c = (ifoci.valc - ifoci.nomc) * BOK_LVDT_ATOD;
        delta_c = round((delta_c <= 0.0) ? delta_c : -1.0 * delta_c);
        busy = true;
        IDMessage(GALIL_DEVICE, "Calling xq_focusind(a=%.1f, b=%.1f, c=%.1f)", delta_a, delta_b, delta_c);
        if ((gstat=xq_focusind(delta_a, delta_b, delta_c)) == G_NO_ERROR) {
          IDMessage(GALIL_DEVICE, "Called xq_focusind(a=%.1f, b=%.1f, c=%.1f) OK", delta_a, delta_b, delta_c);
          IDMessage(GALIL_DEVICE, "Executing 'Restore Nominal Plane' OK");
        } else {
          IDMessage(GALIL_DEVICE, "<ERROR> Failed calling xq_focusind(a=%.1f, b=%.1f, c=%.1f)", delta_a, delta_b, delta_c);
        }
        busy = false;
      }
      ifocus_referenceSP.s = gstat == G_NO_ERROR ? IPS_OK : IPS_ALERT;
      ifocus_referenceS[3].s = ISS_OFF;
    }
  }

  /* reset */
  IUResetSwitch(&ifocus_referenceSP);
  IDSetSwitch(&ifocus_referenceSP, NULL);
}


/*******************************************************************************
 * action: execute_timer()
 ******************************************************************************/
static void execute_timer(void *p) {

  /* declare some variable(s) and initialize them */
  int _gfiltn = -1;
  int _ifiltn = -1;
  int _tfd = -1;
  int _ufd = -1;
  tcp_val_p _tp = (tcp_val_p)NULL;
  udp_val_p _up = (udp_val_p)NULL;

  /* increment counter */
  loop_counter++;

  /* read TCP shared memory */
  if ((_tfd=shm_open(BOK_SHM_TCP_NAME, O_RDONLY, 0666))>=0 &&
      (_tp=(tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ, MAP_SHARED, _tfd, 0))!=(tcp_val_p)NULL) {
    // reset local structure
    (void) memset((void *)&tcp_val, 0, TCP_VAL_SIZE);
    // copy shared memory to local structure
    (void) memmove((void *)&tcp_val, _tp, TCP_VAL_SIZE);
    // dump it
    (void) dump_tcp_structure(&tcp_val);
    // release shared memory
    (void) munmap(_tp, TCP_VAL_SIZE);
    (void) close(_tfd);
    _tp = (tcp_val_p)NULL;
    _tfd = -1;
  }

  /* read UDP shared memory */
  if ((_ufd=shm_open(BOK_SHM_UDP_NAME, O_RDONLY, 0666)) &&
       (_up=(udp_val_p)mmap(0, UDP_VAL_SIZE, PROT_READ, MAP_SHARED, _ufd, 0))!=(udp_val_p)NULL) {
    // reset local structure
    (void) memset((void *)&udp_val, 0, UDP_VAL_SIZE);
    // copy shared memory to local structure
    (void) memmove((void *)&udp_val, _up, UDP_VAL_SIZE);
    // dump it
    (void) dump_udp_structure(&udp_val);
    // release shared memory
    (void) munmap(_up, UDP_VAL_SIZE);
    (void) close(_ufd);
    _up = (udp_val_p)NULL;
    _ufd = -1;
  }

  /* update gfilter(s) */
  (void) memset((void *)&gfilter_names, 0, sizeof(gfilter_names));
  _gfiltn = (int)round(tcp_val.lv.gfiltn);
  // Use the above value to set the switch to on to highlight
  (void) sprintf(gfilter_names.filter_1, "%s", bok_gfilters[1].name);
  (void) sprintf(gfilter_names.filter_2, "%s", bok_gfilters[2].name);
  (void) sprintf(gfilter_names.filter_3, "%s", bok_gfilters[3].name);
  (void) sprintf(gfilter_names.filter_4, "%s", bok_gfilters[4].name);
  (void) sprintf(gfilter_names.filter_5, "%s", bok_gfilters[5].name);
  (void) sprintf(gfilter_names.filter_6, "%s", bok_gfilters[6].name);
  IUResetSwitch(&gfilter_changeSP); // Reset the switches to OFF
  gfilter_changeS[_gfiltn - 1].s = ISS_ON; // Subtract one since zero based
  IDSetSwitch(&gfilter_changeSP, NULL); // Update the switches
  IDSetText(&gfilterTP, NULL);

  /* update ifilter(s) */
  // ifilter is different, it used the filter codes from bar reader not 0-5
  IUResetSwitch(&ifilter_changeSP); // Reset the switches to OFF
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
    if (tcp_val.lv.filtval  == (float)j) { (void) sprintf(telemetrys.filtval, "%s (%d)", bok_ifilters[j].name, j); }
    if (tcp_val.lv.reqfilt  == (float)j) { (void) sprintf(telemetrys.reqfilt, "%s (%d)", bok_ifilters[j].name, j); }
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
  (void) sprintf(telemetrys.a_reference,  "%.3f",           ifoci.refa);
  (void) sprintf(telemetrys.b_reference,  "%.3f",           ifoci.refb);
  (void) sprintf(telemetrys.c_reference,  "%.3f",           ifoci.refc);
  (void) sprintf(telemetrys.distall,      "%08.1f",         tcp_val.lv.distall);
  (void) sprintf(telemetrys.dista,        "%08.1f",         tcp_val.lv.dista);
  (void) sprintf(telemetrys.distb,        "%08.1f",         tcp_val.lv.distb);
  (void) sprintf(telemetrys.distc,        "%08.1f",         tcp_val.lv.distc);
  (void) sprintf(telemetrys.distgcam,     "%08.1f",         tcp_val.lv.distgcam);
  (void) sprintf(telemetrys.errfilt,      "%08.1f",         tcp_val.lv.errfilt);
  (void) sprintf(telemetrys.filtisin,     "%08.1f",         tcp_val.lv.filtisin);
  (void) sprintf(telemetrys.ifilter_1,    "%s (%d)",        ifilter_names.filter_1, (int)round(tcp_val.filtvals[0]));
  (void) sprintf(telemetrys.ifilter_2,    "%s (%d)",        ifilter_names.filter_2, (int)round(tcp_val.filtvals[1]));
  (void) sprintf(telemetrys.ifilter_3,    "%s (%d)",        ifilter_names.filter_3, (int)round(tcp_val.filtvals[2]));
  (void) sprintf(telemetrys.ifilter_4,    "%s (%d)",        ifilter_names.filter_4, (int)round(tcp_val.filtvals[3]));
  (void) sprintf(telemetrys.ifilter_5,    "%s (%d)",        ifilter_names.filter_5, (int)round(tcp_val.filtvals[4]));
  (void) sprintf(telemetrys.ifilter_6,    "%s (%d)",        ifilter_names.filter_6, (int)round(tcp_val.filtvals[5]));
  (void) sprintf(telemetrys.gfiltn,       "%08.1f",         tcp_val.lv.gfiltn);
  (void) sprintf(telemetrys.gfilter_1,    "%s (1)",         gfilter_names.filter_1);
  (void) sprintf(telemetrys.gfilter_2,    "%s (2)",         gfilter_names.filter_2);
  (void) sprintf(telemetrys.gfilter_3,    "%s (3)",         gfilter_names.filter_3);
  (void) sprintf(telemetrys.gfilter_4,    "%s (4)",         gfilter_names.filter_4);
  (void) sprintf(telemetrys.gfilter_5,    "%s (5)",         gfilter_names.filter_5);
  (void) sprintf(telemetrys.gfilter_6,    "%s (6)",         gfilter_names.filter_6);
  IDSetText(&telemetryTP, NULL);
  IDSetText(&telemetry_referenceTP, NULL);

  /* set and update light(s)*/
  telemetry_connectionL[0].s = (tcp_val.simulate == 0) ? IPS_OK : IPS_ALERT;
  telemetry_connectionL[1].s = (udp_val.simulate == 0) ? IPS_OK : IPS_ALERT;
  telemetry_lightsL[0].s = (busy == true ) ? IPS_BUSY : IPS_IDLE;
  telemetry_lightsL[1].s = (IS_BIT_SET(tcp_val.status, 7)) ? IPS_BUSY : IPS_IDLE;
  telemetry_gfilterwheelL[0].s = (udp_val.haxis_moving == 1) ? IPS_BUSY : IPS_IDLE;
  telemetry_ifilterwheelL[0].s = (tcp_val.lv.filtisin == 1.0) ? IPS_IDLE : IPS_BUSY;
  telemetry_ifilterwheelL[1].s = (udp_val.faxis_moving == 1) ? IPS_BUSY : IPS_IDLE;
  telemetry_ifilterwheelL[2].s = (udp_val.gaxis_moving == 1) ? IPS_BUSY : IPS_IDLE;
  telemetry_ifilterwheelL[3].s = (tcp_val.lv.filtisin == 1.0) ? IPS_OK : IPS_IDLE;
  telemetry_ifilterwheelL[4].s = (tcp_val.lv.errfilt == 1.0) ? IPS_ALERT : IPS_IDLE;

  IDMessage(GALIL_DEVICE, "%d", udp_val.faxis_moving);

  telemetry_connectionLP.s = IPS_IDLE;
  telemetry_lightsLP.s   = IPS_IDLE;
  telemetry_ifilterwheelLP.s = IPS_IDLE;
  telemetry_gfilterwheelLP.s = IPS_IDLE;
  IDSetLight(&telemetry_connectionLP, NULL);
  IDSetLight(&telemetry_lightsLP, NULL);
  IDSetLight(&telemetry_ifilterwheelLP, NULL);
  IDSetLight(&telemetry_gfilterwheelLP, NULL);

  /* re-schedule */
  IEAddTimer(BOK_TCP_DELAY_MS, execute_timer, NULL);
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

  (void) sprintf(supports.author,  "%s", _AUTHOR_);
  (void) sprintf(supports.date,    "%s", _DATE_);
  (void) sprintf(supports.email,   "%s", _EMAIL_);
  (void) sprintf(supports.version, "%s", _VERSION_);

  telemetry_connectionL[0].s = ISS_OFF;
  telemetry_connectionL[1].s = ISS_OFF;
  telemetry_lightsL[0].s = ISS_OFF;
  telemetry_lightsL[1].s = ISS_OFF;
  telemetry_gfilterwheelL[0].s = ISS_OFF;
  telemetry_ifilterwheelL[0].s = ISS_OFF;
  telemetry_ifilterwheelL[1].s = ISS_OFF;
  telemetry_ifilterwheelL[2].s = ISS_OFF;
  telemetry_ifilterwheelL[3].s = ISS_OFF;

  telemetry_connectionLP.s = IPS_IDLE;
  telemetry_lightsLP.s   = IPS_IDLE;
  telemetry_gfilterwheelLP.s = IPS_IDLE;
  telemetry_ifilterwheelLP.s = IPS_IDLE;

  /* initialize indi structure(s) */
  IDSetText(&ifilterTP, NULL);
  IDSetText(&gfilterTP, NULL);
  IDSetText(&supportTP, NULL);
  IDSetText(&telemetryTP, NULL);
  IDSetLight(&telemetry_connectionLP, NULL);
  IDSetLight(&telemetry_lightsLP, NULL);
  IDSetLight(&telemetry_gfilterwheelLP, NULL);
  IDSetLight(&telemetry_ifilterwheelLP, NULL);
}
