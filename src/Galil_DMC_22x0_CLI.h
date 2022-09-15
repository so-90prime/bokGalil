/*******************************************************************************
 *
 * Galil_DMC_22x0_CLI.h
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
#define SHORT_CMD  6
#define _NAME_     "Galil_DMC_22x0_CLI"
#define _HELP_     "Command line interface for Galil_DMC_22x0 using gclib"


/*******************************************************************************
 * structure(s)
 ******************************************************************************/
typedef struct supportedcommands {
  char name[BOK_STR_64];
  char help[BOK_STR_128];
} cmd_t, *cmd_p, **cmd_s;

const cmd_t GalilCmds[] = {
  {"exit",             "exit application"},
  {"quit",             "quit application"},
  {"help",             "list available commands"},
  {"?",                "list available commands"},
  {"DISTA=?;",         "report DISTA value"},
  {"DISTB=?;",         "report DISTB value"},
  {"DISTC=?;",         "report DISTC value"},
  {"DISTALL=?;",       "report DISTA/B/C values"},
  {"DISTGCAM=?;",      "report guider focus values"},
  {"DR0;",             "disable UDP packets"},
  {"ERRFILT=?;",       "report filter error status"},
  {"FILTISIN=?;",      "report filter 'in beam' status"},
  {"FILTNUM=?;",       "filter number counter"},
  {"FILTVAL=?",        "report current filter value"},
  {"FILTVALS[0]=?",    "report filter number in slot 0"},
  {"FILTVALS[1]=?",    "report filter number in slot 1"},
  {"FILTVALS[2]=?",    "report filter number in slot 2"},
  {"FILTVALS[3]=?",    "report filter number in slot 3"},
  {"FILTVALS[4]=?",    "report filter number in slot 4"},
  {"FILTVALS[5]=?",    "report filter number in slot 5"},
  {"FOCREFA=?;",       "report focus reference A"},
  {"FOCREFB=?;",       "report focus reference B"},
  {"FOCREFC=?;",       "report focus reference C"},
  {"GFILTN=?;",        "report guider filter"},
  {"HX;",              "halt execution"},
  {"LA;",              "list array(s)"},
  {"LL;",              "list label(s)"},
  {"LV;",              "list variable(s)"},
  {"QR;",              "report QR data (binary)"},
  {"QZ;",              "report QZ data"},
  {"REQFILT=?;",       "report requested filter"},
  {"RP;",              "report reference position(s)"},
  {"TC 1;",            "tell error"},
  {"TOTFOCA=?;",       "report focus actuator A"},
  {"TOTFOCB=?;",       "report focus actuator B"},
  {"TOTFOCC=?;",       "report focus actuator C"},
  {"TA;",              "tell amplifier error status"},
  {"TB;",              "tell status"},
  {"TD;",              "tell dual-encoder / stepper"},
  {"TE;",              "tell error"},
  {"TH;",              "tell (internet) handle"},
  {"TI;",              "tell inputs"},
  {"TP;",              "tell positions"},
  {"TS;",              "tell switches"},
  {"TT;",              "tell torques"},
  {"TV;",              "tell velocities"},
  {"TZ;",              "tell i/o status"},
  {"WH;",              "report handle(s)"},
  {"XQ #FILTIN;",      "set filter to 'in' position"},
  {"XQ #FILTOUT;",     "set filter to 'out' position"},
  {"XQ #FILTMOV;",     "move filter to requested position"},
  {"XQ #FILTRD;",      "read filter in each position"},
  {"XQ #FILTROT,0;",   "rotate filter 0  position"},
  {"XQ #FILTROT,1;",   "rotate filter 1  position"},
  {"XQ #FILTROT,2;",   "rotate filter 2  position"},
  {"XQ #FILTROT,3;",   "rotate filter 3  position"},
  {"XQ #FILTROT,4;",   "rotate filter 4  position"},
  {"XQ #FILTROT,5;",   "rotate filter 5  position"},
  {"XQ #GFWINIT;",     "initialize guider filter"},
  {"XQ #GFWMOV;",      "move guider filter"}
};
#define CMD_NELMS (sizeof(GalilCmds)/sizeof(cmd_t))


/*******************************************************************************
 * function: author()
 ******************************************************************************/
void author(void) {
  (void) fprintf(stdout,"author=%s\n", _AUTHOR_);
  (void) fflush(stdout);
}


/*******************************************************************************
 * function: commands()
 ******************************************************************************/
void commands(void) {
  for (int x=0; x<CMD_NELMS; x++) {
    if (strlen(GalilCmds[x].name) < SHORT_CMD) {
      (void) fprintf(stdout,"%s\t\t%s\n", GalilCmds[x].name, GalilCmds[x].help);
    } else {
      (void) fprintf(stdout,"%s\t%s\n", GalilCmds[x].name, GalilCmds[x].help);
    }
    (void) fflush(stdout);
  }
}


/*******************************************************************************
 * function: date()
 ******************************************************************************/
void date(void) {
  (void) fprintf(stdout, "date=%s\n", _DATE_);
  (void) fflush(stdout);
}


/*******************************************************************************
 * function: email()
 ******************************************************************************/
void email(void) {
  (void) fprintf(stdout, "email=%s\n", _EMAIL_);
  (void) fflush(stdout);
}


/*******************************************************************************
 * function: help()
 ******************************************************************************/
void help(void) {
  (void) fprintf(stdout, "%s: v%s %s %s\n", _NAME_, _VERSION_, _AUTHOR_, _DATE_);
  (void) fprintf(stdout, "%s\n", _HELP_);
  // (void) fprintf(stdout, "\nUse: %s [-a -c -d -e -h -i<addr> -o -v]\n", _NAME_);
  (void) fprintf(stdout, "\nUse: %s [-a -b<int> -c -d -e -h -o -v]\n", _NAME_);
  (void) fprintf(stdout, "\t-a       : show author\n");
  (void) fprintf(stdout, "\t-b<int>  : 1=bok, 0=lab [default=1]\n");
  (void) fprintf(stdout, "\t-c       : show (supported) commands\n");
  (void) fprintf(stdout, "\t-d       : show date\n");
  (void) fprintf(stdout, "\t-e       : show email\n");
  (void) fprintf(stdout, "\t-o       : override unsupported command(s)\n");
  // (void) fprintf(stdout, "\t-i<addr> : set ip address\n");
  (void) fprintf(stdout, "\t-v       : show version\n");
  (void) fflush(stdout);
}


/*******************************************************************************
 * function: version()
 ******************************************************************************/
void version(void) {
  (void) fprintf(stdout,"version=%s\n", _VERSION_);
  (void) fflush(stdout);
}
