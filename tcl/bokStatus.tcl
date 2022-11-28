# +
# BOK_INSTRUMENT \ Instrument                 \ 90Prime
# BOK_INDI_ADDR  \ Indiserver Address         \ 10.130.133.206
# BOK_INDI_PORT  \ Indiserver Port            \ 7624
# BOK_NG_ADDR    \ NG Server Address          \ 10.130.133.206
# BOK_NG_PORT    \ NG Server Port             \ 5750
# BOK_TCP_ADDR   \ Galil TCP Command Address  \ 10.130.133.206
# BOK_TCP_PORT   \ Galil TCP Command Port     \ 23
# BOK_UDP_ADDR   \ Galil UDP Command Address  \ 10.130.133.206
# BOK_UDP_PORT   \ Galil UDP Command Port     \ 5078
# BOK_WEB_ADDR   \ pyINDI Website Address     \ 10.130.133.206
# BOK_WEB_PORT   \ pyINDI Website Port        \ 5905
# -
proc bokStatus { } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  bokClear

  if {[info exists bokVariables(bok_indidriver)] && $bokVariables(bok_indidriver) > 0} {
    bokInform [format {%4s %-35s %s} "INFO" "bokGalilIndiDriver" "RUNNING (pid(s): $bokVariables(bok_indidriver))"]
  } else {
    bokInform [format {%4s %-35s %s} "INFO" "bokGalilIndiDriver" "NOT RUNNING"]
  }

  if {[info exists bokVariables(bok_indiserver)] && $bokVariables(bok_indiserver) > 0} {
    bokInform [format {%4s %-35s %s} "INFO" "IndiServer" "RUNNING on $bokParams(BOK_INDI_ADDR):$bokParams(BOK_INDI_PORT) (pid(s): $bokVariables(bok_indiserver))"]
  } else {
    bokInform [format {%4s %-35s %s} "INFO" "IndiServer" "NOT RUNNING"]
  }

  if {[info exists bokVariables(bok_ngserver)] && $bokVariables(bok_ngserver) > 0} {
    bokInform [format {%4s %-35s %s} "INFO" "Galil_DMC_22x0_NgServer" "RUNNING on $bokParams(BOK_NG_ADDR):$bokParams(BOK_NG_PORT) (pid(s): $bokVariables(bok_ngserver))"]
  } else {
    bokInform [format {%4s %-35s %s} "INFO" "Galil_DMC_22x0_NgServer" "NOT RUNNING"]
  }

  if {[info exists bokVariables(bok_memory)] && $bokVariables(bok_memory) > 0} {
    bokInform [format {%4s %-35s %s} "INFO" "Galil_DMC_22x0_Write_memory" "RUNNING (pid(s): $bokVariables(bok_memory))"]
  } else {
    bokInform [format {%4s %-35s %s} "INFO" "Galil_DMC_22x0_Write_memory" "NOT RUNNING"]
  }

  if {[info exists bokVariables(bok_website)] && $bokVariables(bok_website) > 0} {
    bokInform [format {%4s %-35s %s} "INFO" "python3 bok.py" "RUNNING on $bokParams(BOK_WEB_ADDR):$bokParams(BOK_WEB_PORT) (pid(s): $bokVariables(bok_website))"]
  } else {
    bokInform [format {%4s %-35s %s} "INFO" "python3 bok.py" "NOT RUNNING"]
  }

  if {[info exists bokVariables(bok_ds9)] && $bokVariables(bok_ds9) > 0} {
    bokInform [format {%4s %-35s %s} "INFO" "ds9" "RUNNING (pid(s): $bokVariables(bok_ds9))"]
  } else {
    bokInform [format {%4s %-35s %s} "INFO" "ds9" "NOT RUNNING"]
  }

  if {[info exists bokVariables(bok_dataserver)] && $bokVariables(bok_dataserver) > 0} {
    bokInform [format {%4s %-35s %s} "INFO" "DataServer" "RUNNING (pid(s): $bokVariables(bok_dataserver))"]
  } else {
    bokInform [format {%4s %-35s %s} "INFO" "DataServer" "NOT RUNNING"]
  }

  if {[info exists bokVariables(bok_tcpmem)] && $bokVariables(bok_tcpmem) > 0} {
    bokInform [format {%4s %-35s %s} "INFO" "/dev/shm/tcp_mem" "PRESENT"]
  } else {
    bokInform [format {%4s %-35s %s} "INFO" "/dev/shm/tcp_mem" "NOT PRESENT"]
  }

  if {[info exists bokVariables(bok_udpmem)] && $bokVariables(bok_udpmem) > 0} {
    bokInform [format {%4s %-35s %s} "INFO" "/dev/shm/udp_mem" "PRESENT"]
  } else {
    bokInform [format {%4s %-35s %s} "INFO" "/dev/shm/udp_mem" "NOT PRESENT"]
  }
}
