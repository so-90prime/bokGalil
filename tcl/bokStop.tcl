proc bokStop { } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  bokClear

  if {[info exists bokVariables(bok_indidriver)] && $bokVariables(bok_indidriver) > 0} {
    bokInform "after 500 bokTerminate $bokVariables(bok_indidriver)"
    after 500 bokTerminate $bokVariables(bok_indidriver)
  } else {
    bokInform [format {%4s %-35s %s} "WARN" "bokGalilIndiDriver" "STOPPED"]
  }

  if {[info exists bokVariables(bok_indiserver)] && $bokVariables(bok_indiserver) > 0} {
    bokInform "after 500 bokTerminate $bokVariables(bok_indiserver)"
    after 500 bokTerminate $bokVariables(bok_indiserver)
  } else {
    bokInform [format {%4s %-35s %s} "WARN" "IndiServer" "STOPPED"]
  }

  if {[info exists bokVariables(bok_ngserver)] && $bokVariables(bok_ngserver) > 0} {
    bokInform "after 500 bokTerminate $bokVariables(bok_ngserver)"
    after 500 bokTerminate $bokVariables(bok_ngserver)
  } else {
    bokInform [format {%4s %-35s %s} "WARN" "Galil_DMC_22x0_NgServer" "STOPPED"]
  }

  if {[info exists bokVariables(bok_memory)] && $bokVariables(bok_memory) > 0} {
    bokInform "after 500 bokTerminate $bokVariables(bok_memory)"
    after 500 bokTerminate $bokVariables(bok_memory)
  } else {
    bokInform [format {%4s %-35s %s} "WARN" "Galil_DMC_22x0_Write_Memory" "STOPPED"]
  }

  if {[info exists bokVariables(bok_website)] && $bokVariables(bok_website) > 0} {
    bokInform "after 500 bokTerminate $bokVariables(bok_website)"
    after 500 bokTerminate $bokVariables(bok_website)
  } else {
    bokInform [format {%4s %-35s %s} "WARN" "python3 bok.py" "STOPPED"]
  }

  if {[info exists bokVariables(bok_ds9)] && $bokVariables(bok_ds9) > 0} {
    bokInform "after 500 bokTerminate $bokVariables(bok_ds9)"
    after 500 bokTerminate $bokVariables(bok_ds9)
  } else {
    bokInform [format {%4s %-35s %s} "WARN" "ds9" "STOPPED"]
  }

  if {[info exists bokVariables(bok_dataserver)] && $bokVariables(bok_dataserver) > 0} {
    bokInform "after 500 bokTerminate $bokVariables(bok_dataserver)"
    after 500 bokTerminate $bokVariables(bok_dataserver)
  } else {
    bokInform [format {%4s %-35s %s} "WARN" "DataServer" "STOPPED"]
  }

  if {[info exists bokVariables(bok_tcpmem)] && $bokVariables(bok_tcpmem) > 0} {
    bokInform "file delete -force /dev/shm/tcp_mem"
    file delete -force /dev/shm/tcp_shm
  } else {
    bokInform [format {%4s %-35s %s} "WARN" "/dev/shm/tcp_mem" "NOT PRESENT"]
  }

  if {[info exists bokVariables(bok_udpmem)] && $bokVariables(bok_udpmem) > 0} {
    bokInform "file delete -force /dev/shm/udp_mem"
    file delete -force /dev/shm/udp_shm
  } else {
    bokInform [format {%4s %-35s %s} "WARN" "/dev/shm/udp_mem" "NOT PRESENT"]
  }
}
