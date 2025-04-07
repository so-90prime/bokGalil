proc bokStatus { } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  bokClear

  if {[info exists bokVariables(bok_memory)] && $bokVariables(bok_memory)>0} {
    bokInform [format {%4s %-60s %s} "INFO" "$env(BOK_GALIL_BIN)/Galil_DMC_22x0_Write_Memory" "RUNNING \[pid(s): $bokVariables(bok_memory)\]"]
  } else {
    bokInform [format {%4s %-60s %s} "INFO" "$env(BOK_GALIL_BIN)/Galil_DMC_22x0_Write_Memory" "NOT RUNNING"]
  }

  if {[info exists bokVariables(bok_tcpmem)] && $bokVariables(bok_tcpmem)>0} {
    bokInform [format {%4s %-60s %s} "INFO" "/dev/shm/tcp_mem" "PRESENT"]
  } else {
    bokInform [format {%4s %-60s %s} "INFO" "/dev/shm/tcp_mem" "NOT PRESENT"]
  }

  if {[info exists bokVariables(bok_udpmem)] && $bokVariables(bok_udpmem)>0} {
    bokInform [format {%4s %-60s %s} "INFO" "/dev/shm/udp_mem" "PRESENT"]
  } else {
    bokInform [format {%4s %-60s %s} "INFO" "/dev/shm/udp_mem" "NOT PRESENT"]
  }

  if {[info exists bokVariables(bok_indidriver)] && $bokVariables(bok_indidriver)>0} {
    bokInform [format {%4s %-60s %s} "INFO" "$env(BOK_GALIL_BIN)/bokGalilIndiDriver" "RUNNING \[pid(s): $bokVariables(bok_indidriver)\]"]
  } else {
    bokInform [format {%4s %-60s %s} "INFO" "$env(BOK_GALIL_BIN)/bokGalilIndiDriver" "NOT RUNNING"]
  }

  if {[info exists bokVariables(bok_indiserver)] && $bokVariables(bok_indiserver)>0} {
    bokInform [format {%4s %-60s %s} "INFO" "/usr/bin/indiserver" "RUNNING \[pid(s): $bokVariables(bok_indiserver)\]"]
  } else {
    bokInform [format {%4s %-60s %s} "INFO" "/usr/bin/indiserver" "NOT RUNNING"]
  }

  if {[info exists bokVariables(bok_ngserver)] && $bokVariables(bok_ngserver)>0} {
    bokInform [format {%4s %-60s %s} "INFO" "$env(BOK_GALIL_BIN)/Galil_DMC_22x0_NgServer" "RUNNING \[pid(s): $bokVariables(bok_ngserver)\]"]
  } else {
    bokInform [format {%4s %-60s %s} "INFO" "$env(BOK_GALIL_BIN)/Galil_DMC_22x0_NgServer" "NOT RUNNING"]
  }

  if {[info exists bokVariables(bok_website)] && $bokVariables(bok_website)>0} {
    bokInform [format {%4s %-60s %s} "INFO" "$bokParams(BOK_WEB_REPO)/src/bok.py" "RUNNING \[pid(s): $bokVariables(bok_website)\]"]
  } else {
    bokInform [format {%4s %-60s %s} "INFO" "$bokParams(BOK_WEB_REPO)/src/bok.py" "NOT RUNNING"]
  }

  if {[info exists bokVariables(bok_dataserver)] && $bokVariables(bok_dataserver)>0} {
    bokInform [format {%4s %-60s %s} "INFO" "$bokParams(BOK_DATA_REPO)/dataserver" "RUNNING \[pid(s): $bokVariables(bok_dataserver)\]"]
  } else {
    bokInform [format {%4s %-60s %s} "INFO" "$bokParams(BOK_DATA_REPO)/dataserver" "NOT RUNNING"]
  }

  if {[info exists bokVariables(bok_flatfield)] && $bokVariables(bok_flatfield)>0} {
    bokInform [format {%4s %-60s %s} "INFO" "$bokParams(BOK_FF_REPO)/bff_src/bok-flat-field.py" "RUNNING \[pid(s): $bokVariables(bok_flatfield)\]"]
  } else {
    bokInform [format {%4s %-60s %s} "INFO" "$bokParams(BOK_FF_REPO)/bff_src/bok-flat-field.py" "NOT RUNNING"]
  }

  if {[info exists bokVariables(bok_ds9)] && $bokVariables(bok_ds9)>0} {
    bokInform [format {%4s %-60s %s} "INFO" "/usr/bin/ds9" "RUNNING \[pid(s): $bokVariables(bok_ds9))"]
  } else {
    bokInform [format {%4s %-60s %s} "INFO" "/usr/bin/ds9" "NOT RUNNING"]
  }
}
