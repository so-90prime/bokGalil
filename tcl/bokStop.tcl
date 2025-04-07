proc bokStop { } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  bokClear

  if {[info exists bokVariables(bok_indidriver)] && $bokVariables(bok_indidriver)>0} {
    bokInform "after 500 bokTerminate $bokVariables(bok_indidriver)"
    after 500 bokTerminate $bokVariables(bok_indidriver)
  } else {
    bokInform [format {%4s %-60s %s} "WARN" "$env(BOK_GALIL_BIN)/bokGalilIndiDriver" "ALREADY STOPPED"]
  }

  if {[info exists bokVariables(bok_indiserver)] && $bokVariables(bok_indiserver)>0} {
    bokInform "after 500 bokTerminate $bokVariables(bok_indiserver)"
    after 500 bokTerminate $bokVariables(bok_indiserver)
  } else {
    bokInform [format {%4s %-60s %s} "WARN" "/usr/bin/indiserver" "ALREADY STOPPED"]
  }

  if {[info exists bokVariables(bok_ngserver)] && $bokVariables(bok_ngserver)>0} {
    bokInform "after 500 bokTerminate $bokVariables(bok_ngserver)"
    after 500 bokTerminate $bokVariables(bok_ngserver)
  } else {
    bokInform [format {%4s %-60s %s} "WARN" "$env(BOK_GALIL_BIN)/Galil_DMC_22x0_NgServer" "ALREADY STOPPED"]
  }

  if {[info exists bokVariables(bok_memory)] && $bokVariables(bok_memory)>0} {
    bokInform "after 500 bokTerminate $bokVariables(bok_memory)"
    after 500 bokTerminate $bokVariables(bok_memory)
  } else {
    bokInform [format {%4s %-60s %s} "WARN" "$env(BOK_GALIL_BIN)/Galil_DMC_22x0_Write_Memory" "ALREADY STOPPED"]
  }

  if {[info exists bokVariables(bok_tcpmem)] && $bokVariables(bok_tcpmem)>0} {
    bokInform "file delete -force /dev/shm/tcp_mem"
    file delete -force /dev/shm/tcp_shm
  } else {
    bokInform [format {%4s %-60s %s} "WARN" "/dev/shm/tcp_mem" "ALREADY REMOVED"]
  }

  if {[info exists bokVariables(bok_udpmem)] && $bokVariables(bok_udpmem)>0} {
    bokInform "file delete -force /dev/shm/udp_mem"
    file delete -force /dev/shm/udp_shm
  } else {
    bokInform [format {%4s %-60s %s} "WARN" "/dev/shm/udp_mem" "ALREADY REMOVED"]
  }

  if {[info exists bokVariables(bok_website)] && $bokVariables(bok_website)>0} {
    bokInform "after 500 bokTerminate $bokVariables(bok_website)"
    after 500 bokTerminate $bokVariables(bok_website)
  } else {
    bokInform [format {%4s %-60s %s} "WARN" "$bokParams(BOK_WEB_REPO)/src/bok.py" "ALREADY STOPPED"]
  }

  if {[info exists bokVariables(bok_dataserver)] && $bokVariables(bok_dataserver)>0} {
    bokInform "after 500 bokTerminate $bokVariables(bok_dataserver)"
    after 500 bokTerminate $bokVariables(bok_dataserver)
  } else {
    bokInform [format {%4s %-60s %s} "WARN" "$bokParams(BOK_DATA_REPO)/dataserver" "ALREADY STOPPED"]
  }

  if {[info exists bokVariables(bok_flatfield)] && $bokVariables(bok_flatfield)>0} {
    bokInform "after 500 bokTerminate $bokVariables(bok_flatfield)"
    after 500 bokTerminate $bokVariables(bok_flatfield)
  } else {
    bokInform [format {%4s %-60s %s} "WARN" "$bokParams(BOK_DATA_REPO)/bff_src/bok-flat-field.py" "ALREADY STOPPED"]
  }

  if {[info exists bokVariables(bok_ds9)] && $bokVariables(bok_ds9)>0} {
    bokInform "after 500 bokTerminate $bokVariables(bok_ds9)"
    after 500 bokTerminate $bokVariables(bok_ds9)
  } else {
    bokInform [format {%4s %-60s %s} "WARN" "/usr/bin/ds9" "ALREADY STOPPED"]
  }
}
