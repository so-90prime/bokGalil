/*******************************************************************************
 *
 * Galil_DMC_22x0_TCP_Write.c
 *
 ******************************************************************************/


/*******************************************************************************
 * include(s)
 ******************************************************************************/
#include "__hosts__.h"
#include "bokGalil.h"


/*******************************************************************************
 * signal(s)
 ******************************************************************************/
static int running = 1;
void galil_handler(int mysig) { running = 0; }


/*******************************************************************************
 * define(s)
 ******************************************************************************/
#define _HELP_ "TCP command(s) for the Galil_DMC_22x0 via gclib"
#define _NAME_ "Galil_DMC_22x0_TCP_Write"


/*******************************************************************************
 * function: main()
 ******************************************************************************/
int main( int argc, char *argv[] ) {

  /* declare some variables and initialize them */
  bool simulate = false;
  char buffer[BOK_STR_2048] = {'\0'};
  char cmnd[BOK_STR_64] = {'\0'};
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
  int delay = BOK_TCP_DELAY_MS;
  int tcp_shm_fd = -1;
  int tcp_shm_stat = -1;
  tcp_val_p tcp_shm_p = (tcp_val_p)NULL;
  tcp_val_t tcp_val;
  time_t now = (time_t)NULL;

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
  (void) shm_unlink(BOK_SHM_TCP_NAME);
  if ((tcp_shm_fd=shm_open(BOK_SHM_TCP_NAME, O_CREAT | O_RDWR, 0777)) < 0) {
    (void) fprintf(stderr, "%s <ERROR> failed to execute '%s' exiting, tcp_shm_fd=%d\n", _NAME_, "shm_open()", tcp_shm_fd);
    (void) fflush(stderr);
    exit(tcp_shm_fd);
  }

  /* (re)size shared memory segment */
  if ((tcp_shm_stat=ftruncate(tcp_shm_fd, TCP_VAL_SIZE)) < 0) {
    (void) fprintf(stderr, "%s <ERROR> failed to execute '%s' exiting, tcp_shm_stat=%d\n", _NAME_, "ftruncate()", tcp_shm_stat);
    (void) fflush(stderr);
    exit(tcp_shm_stat);
  }
  
  /* memory map the shared memory object */
  if ((tcp_shm_p=(tcp_val_p)mmap(0, TCP_VAL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, tcp_shm_fd, 0)) == (tcp_val_p)NULL) {
    (void) fprintf(stderr, "%s <ERROR> failed to execute '%s' exiting, tcp_shm_p=%p\n", _NAME_, "mmap()", tcp_shm_p);
    (void) fflush(stderr);
    exit(-1);
  }

  /* open the ip_addr */
  gstat = G_NO_ERROR;
  (void) memset((void *)cmnd, 0, sizeof(cmnd));
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

  /* loop for command(s) */
  while (running > 0) {

    /* reset record */
    (void) memset((void *)&tcp_val, 0, sizeof(tcp_val));

    /* initialize element(s) */
    now = time(NULL);
    (void) strftime(tcp_val.timestamp, sizeof(tcp_val.timestamp), "%Y-%m-%dT%X.00", localtime(&now));
    tcp_val.jd = get_jd(localtime(&now));
    tcp_val.shutdown = 0;
    tcp_val.counter = ++counter;
    tcp_val.simulate = simulate == true ? 1 : 0;
    if (simulate) {
      (void) strftime(tcp_val.software, sizeof(tcp_val.software), "%a%b%W-%Zsw", localtime(&now));
      (void) strftime(tcp_val.hardware, sizeof(tcp_val.hardware), "%a%b%j-%Zhw", localtime(&now));
    } else {
      (void) GVersion(tcp_val.software, sizeof(tcp_val.software));
      (void) GInfo(gfd, tcp_val.hardware, sizeof(tcp_val.hardware));
    }

    /* create a simulated record using snapshot data */
    if (simulate == true) {
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

    /* create record from tcp command(s) */
    } else {

      for (int x=0; x<TCP_CMD_NELMS; x++) {

        /* get command */
        pr = (char *)TcpCmds[x].name;

        /* send command */
        (void) memset((void *)buffer, '\0', sizeof(buffer));
        if ((gstat=GCommand(gfd, pr, buffer, sizeof(buffer), 0)) != G_NO_ERROR) {

          /* report error */
          tcp_val.gstatus += gstat;
          (void) memset(buffer, '\0', sizeof(buffer));
          if ((gstat=GCommand(gfd, "TC 1;", buffer, sizeof(buffer), 0)) == G_NO_ERROR) {
            chomp(buffer, "\n");
            chomp(buffer, "\r");
            (void) fprintf(stderr, "%s <INFO> executed 'TC 1;', buffer='%s', gstat=%d\n", _NAME_, buffer, gstat);
            (void) fflush(stderr);
          } else {
            tcp_val.gstatus += gstat;
            (void) fprintf(stderr, "%s <ERROR> failed executing 'TC 1;', estring='', gstat=%d\n", _NAME_, gstat);
            (void) fflush(stderr);
          }

        /* command executed ok */
        } else {

          /* clean up the response */
          tcp_val.gstatus += gstat;
          replace_word(buffer, sizeof(buffer), "\n", " ");
          replace_word(buffer, sizeof(buffer), "\r", " ");
          replace_word(buffer, sizeof(buffer), ":", "");

          /* check status */
          if (strncasecmp(buffer, "?", 1)==0 || strlen(buffer)<=1) {
            (void) fprintf(stderr, "%s <INFO> command '%s' rejected, gstat=%d\n", _NAME_, pr, gstat);
            (void) fflush(stderr);
          } else {
            if (strncasecmp(pr, "FILTVALS[0]=?;", strlen("FILTVALS[0]=?;")) == 0) {
              decode_float(buffer, &tcp_val.filtvals[0]);
            } else if (strncasecmp(pr, "FILTVALS[1]=?;", strlen("FILTVALS[1]=?;")) == 0) {
              decode_float(buffer, &tcp_val.filtvals[1]);
            } else if (strncasecmp(pr, "FILTVALS[2]=?;", strlen("FILTVALS[2]=?;")) == 0) {
              decode_float(buffer, &tcp_val.filtvals[2]);
            } else if (strncasecmp(pr, "FILTVALS[3]=?;", strlen("FILTVALS[3]=?;")) == 0) {
              decode_float(buffer, &tcp_val.filtvals[3]);
            } else if (strncasecmp(pr, "FILTVALS[4]=?;", strlen("FILTVALS[4]=?;")) == 0) {
              decode_float(buffer, &tcp_val.filtvals[4]);
            } else if (strncasecmp(pr, "FILTVALS[5]=?;", strlen("FILTVALS[5]=?;")) == 0) {
              decode_float(buffer, &tcp_val.filtvals[5]);
            } else if (strncasecmp(pr, "LV;", strlen("LV;")) == 0) {
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
                  } else if (strncasecmp(lv_name, "DISTALL=", strlen("DISTALL=")) == 0) {
                    tcp_val.lv.distall = fval;
                  } else if (strncasecmp(lv_name, "DISTB=", strlen("DISTB=")) == 0) {
                    tcp_val.lv.distb = fval;
                  } else if (strncasecmp(lv_name, "DISTC=", strlen("DISTC=")) == 0) {
                    tcp_val.lv.distc = fval;
                  } else if (strncasecmp(lv_name, "DISTGCAM=", strlen("DISTGCAM=")) == 0) {
                    tcp_val.lv.distgcam = fval;
                  } else if (strncasecmp(lv_name, "ERRFILT=", strlen("ERRFILT=")) == 0) {
                    tcp_val.lv.errfilt = fval;
                  } else if (strncasecmp(lv_name, "FILTBIT1=", strlen("FILTBIT1=")) == 0) {
                    tcp_val.lv.filtbit1 = fval;
                  } else if (strncasecmp(lv_name, "FILTBIT2=", strlen("FILTBIT2=")) == 0) {
                    tcp_val.lv.filtbit2 = fval;
                  } else if (strncasecmp(lv_name, "FILTBIT3=", strlen("FILTBIT3=")) == 0) {
                    tcp_val.lv.filtbit3 = fval;
                  } else if (strncasecmp(lv_name, "FILTBIT4=", strlen("FILTBIT4=")) == 0) {
                    tcp_val.lv.filtbit4 = fval;
                  } else if (strncasecmp(lv_name, "FILTBIT5=", strlen("FILTBIT5=")) == 0) {
                    tcp_val.lv.filtbit5 = fval;
                  } else if (strncasecmp(lv_name, "FILTBIT6=", strlen("FILTBIT6=")) == 0) {
                    tcp_val.lv.filtbit6 = fval;
                  } else if (strncasecmp(lv_name, "FILTBIT7=", strlen("FILTBIT7=")) == 0) {
                    tcp_val.lv.filtbit7 = fval;
                  } else if (strncasecmp(lv_name, "FILTBIT8=", strlen("FILTBIT8=")) == 0) {
                    tcp_val.lv.filtbit8 = fval;
                  } else if (strncasecmp(lv_name, "FILTHIGH=", strlen("FILTHIGH=")) == 0) {
                    tcp_val.lv.filthigh = fval;
                  } else if (strncasecmp(lv_name, "FILTISIN=", strlen("FILTISIN=")) == 0) {
                    tcp_val.lv.filtisin = fval;
                  } else if (strncasecmp(lv_name, "FILTLOW=", strlen("FILTLOW=")) == 0) {
                    tcp_val.lv.filtlow = fval;
                  } else if (strncasecmp(lv_name, "FILTNUM=", strlen("FILTNUM=")) == 0) {
                    tcp_val.lv.filtnum = fval;
                  } else if (strncasecmp(lv_name, "FILTRAC=", strlen("FILTRAC=")) == 0) {
                    tcp_val.lv.filtrac = fval;
                  } else if (strncasecmp(lv_name, "FILTRBL=", strlen("FILTRBL=")) == 0) {
                    tcp_val.lv.filtrbl = fval;
                  } else if (strncasecmp(lv_name, "FILTRDC=", strlen("FILTRDC=")) == 0) {
                    tcp_val.lv.filtrdc = fval;
                  } else if (strncasecmp(lv_name, "FILTRFL=", strlen("FILTRFL=")) == 0) {
                    tcp_val.lv.filtrfl = fval;
                  } else if (strncasecmp(lv_name, "FILTRSP=", strlen("FILTRSP=")) == 0) {
                    tcp_val.lv.filtrsp = fval;
                  } else if (strncasecmp(lv_name, "FILTTAC=", strlen("FILTTAC=")) == 0) {
                    tcp_val.lv.filttac = fval;
                  } else if (strncasecmp(lv_name, "FILTTDC=", strlen("FILTTDC=")) == 0) {
                    tcp_val.lv.filttdc = fval;
                  } else if (strncasecmp(lv_name, "FILTTDIS=", strlen("FILTTDIS=")) == 0) {
                    tcp_val.lv.filttdis = fval;
                  } else if (strncasecmp(lv_name, "FILTTNUD=", strlen("FILTTNUD=")) == 0) {
                    tcp_val.lv.filttnud = fval;
                  } else if (strncasecmp(lv_name, "FILTTSC=", strlen("FILTTSC=")) == 0) {
                    tcp_val.lv.filttsc = fval;
                  } else if (strncasecmp(lv_name, "FILTTSP=", strlen("FILTTSP=")) == 0) {
                    tcp_val.lv.filttsp = fval;
                  } else if (strncasecmp(lv_name, "FILTVAL=", strlen("FILTVAL=")) == 0) {
                    tcp_val.lv.filtval = fval;
                  } else if (strncasecmp(lv_name, "FNUM=", strlen("FNUM=")) == 0) {
                    tcp_val.lv.fnum = fval;
                  } else if (strncasecmp(lv_name, "FNUM_IN=", strlen("FNUM_IN=")) == 0) {
                    tcp_val.lv.fnum_in = fval;
                  } else if (strncasecmp(lv_name, "FOCAC=", strlen("FOCAC=")) == 0) {
                    tcp_val.lv.focac = fval;
                  } else if (strncasecmp(lv_name, "FOCBL=", strlen("FOCBL=")) == 0) {
                    tcp_val.lv.focbl = fval;
                  } else if (strncasecmp(lv_name, "FOCDC=", strlen("FOCDC=")) == 0) {
                    tcp_val.lv.focdc = fval;
                  } else if (strncasecmp(lv_name, "FOCFL=", strlen("FOCFL=")) == 0) {
                    tcp_val.lv.focfl = fval;
                  } else if (strncasecmp(lv_name, "FOCREFA=", strlen("FOCREFA=")) == 0) {
                    tcp_val.lv.focrefa = fval;
                  } else if (strncasecmp(lv_name, "FOCREFB=", strlen("FOCREFB=")) == 0) {
                    tcp_val.lv.focrefb = fval;
                  } else if (strncasecmp(lv_name, "FOCREFC=", strlen("FOCREFC=")) == 0) {
                    tcp_val.lv.focrefc = fval;
                  } else if (strncasecmp(lv_name, "FOCRFSET=", strlen("FOCRFSET=")) == 0) {
                    tcp_val.lv.focrfset = fval;
                  } else if (strncasecmp(lv_name, "FOCSP=", strlen("FOCSP=")) == 0) {
                    tcp_val.lv.focsp = fval;
                  } else if (strncasecmp(lv_name, "GFAC=", strlen("GFAC=")) == 0) {
                    tcp_val.lv.gfac = fval;
                  } else if (strncasecmp(lv_name, "GFCENT=", strlen("GFCENT=")) == 0) {
                    tcp_val.lv.gfcent = fval;
                  } else if (strncasecmp(lv_name, "GFDC=", strlen("GFDC=")) == 0) {
                    tcp_val.lv.gfdc = fval;
                  } else if (strncasecmp(lv_name, "GFILTAC=", strlen("GFILTAC=")) == 0) {
                    tcp_val.lv.gfiltac = fval;
                  } else if (strncasecmp(lv_name, "GFILTDC=", strlen("GFILTDC=")) == 0) {
                    tcp_val.lv.gfiltdc = fval;
                  } else if (strncasecmp(lv_name, "GFILTN=", strlen("GFILTN=")) == 0) {
                    tcp_val.lv.gfiltn = fval;
                  } else if (strncasecmp(lv_name, "GFILTQ=", strlen("GFILTQ=")) == 0) {
                    tcp_val.lv.gfiltq = fval;
                  } else if (strncasecmp(lv_name, "GFILTREQ=", strlen("GFILTREQ=")) == 0) {
                    tcp_val.lv.gfiltreq = fval;
                  } else if (strncasecmp(lv_name, "GFILTSP=", strlen("GFILTSP=")) == 0) {
                    tcp_val.lv.gfiltsp = fval;
                  } else if (strncasecmp(lv_name, "GFSP=", strlen("GFSP=")) == 0) {
                    tcp_val.lv.gfsp = fval;
                  } else if (strncasecmp(lv_name, "GIFLTN=", strlen("GIFLTN=")) == 0) {
                    tcp_val.lv.gifltn = fval;
                  } else if (strncasecmp(lv_name, "INITFILT=", strlen("INITFILT=")) == 0) {
                    tcp_val.lv.initfilt = fval;
                  } else if (strncasecmp(lv_name, "NMOVES=", strlen("NMOVES=")) == 0) {
                    tcp_val.lv.nmoves = fval;
                  } else if (strncasecmp(lv_name, "NROT=", strlen("NROT=")) == 0) {
                    tcp_val.lv.nrot = fval;
                  } else if (strncasecmp(lv_name, "REQFILT=", strlen("REQFILT=")) == 0) {
                    tcp_val.lv.reqfilt = fval;
                  } else if (strncasecmp(lv_name, "SNUM=", strlen("SNUM=")) == 0) {
                    tcp_val.lv.snum = fval;
                  } else if (strncasecmp(lv_name, "SNUM_IN=", strlen("SNUM_IN=")) == 0) {
                    tcp_val.lv.snum_in = fval;
                  } else if (strncasecmp(lv_name, "TOTFOCA=", strlen("TOTFOCA=")) == 0) {
                    tcp_val.lv.totfoca = fval;
                  } else if (strncasecmp(lv_name, "TOTFOCB=", strlen("TOTFOCB=")) == 0) {
                    tcp_val.lv.totfocb = fval;
                  } else if (strncasecmp(lv_name, "TOTFOCC=", strlen("TOTFOCC=")) == 0) {
                    tcp_val.lv.totfocc = fval;
                  } else if (strncasecmp(lv_name, "VECAC=", strlen("VECAC=")) == 0) {
                    tcp_val.lv.vecac = fval;
                  } else if (strncasecmp(lv_name, "VECDC=", strlen("VECDC=")) == 0) {
                    tcp_val.lv.vecdc = fval;
                  } else if (strncasecmp(lv_name, "VECSP=", strlen("VECSP=")) == 0) {
                    tcp_val.lv.vecsp = fval;
                  }
                }
                tok = strtok(NULL, "\\");
              }
            } else if (strncasecmp(pr, "TB;", strlen("TB;")) == 0) {
              decode_integer(buffer, &tcp_val.status);
            } else if (strncasecmp(pr, "TP;", strlen("TP;")) == 0) {
              decode_fvals(buffer, tcp_val.position, BOK_AXES, ',');
            } else {
              (void) fprintf(stdout, "%s '%s' returns '%s'\n", _NAME_, pr, buffer);
            }
            (void) fflush(stdout);
          }
        }
      }
    }

    /* move record to shared memory */
    (void) memmove(tcp_shm_p, &tcp_val, TCP_VAL_SIZE);

    /* dump shared memory */
    dump_tcp_structure(tcp_shm_p);

    /* delay */
    (void) usleep((useconds_t)delay*1000);
  }

  /* close device */
  if (gfd) { GClose(gfd); }

  /* close shared memory */
  tcp_val.shutdown = 1;
  (void) memmove(tcp_shm_p, &tcp_val, TCP_VAL_SIZE);
  if (tcp_shm_p != (tcp_val_p)NULL) { (void) munmap(tcp_shm_p, TCP_VAL_SIZE); }
  if (tcp_shm_fd >= 0) { (void) close(tcp_shm_fd); }

  /* return */
  return 0;
}
