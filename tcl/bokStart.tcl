proc bokStart { } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  bokClear

  if {[info exists bokVariables(bok_memory)] && $bokVariables(bok_memory)>0} {
    bokInform [format {%4s %-35s %s} "WARN" "Galil_DMC_22x0_Write_Memory" "ALREADY RUNNING"]
  } else {
    set _c $env(BOK_GALIL_BIN)/Galil_DMC_22x0_Write_Memory
    set _l $env(BOK_GALIL_LOG)/Galil_DMC_22x0_Write_Memory.log
    if {[file exists ${_c}]} {
      bokInform "INFO ${_c} exists"
      bokInform "INFO executing ${_c} >> ${_l} 2>> ${_l}"
      set _s [catch {exec ${_c} >> ${_l} 2>> ${_l} &} _r]
      if {${_s} == 0} {
        bokInform "${_c} started OK, _r=${_r}"
      } else {
        bokInform "ERROR ${_c} failed to start, status=${_s}, error='${_r}'!"
      }
    }
  }

  if {[info exists bokVariables(bok_indiserver)] && $bokVariables(bok_indiserver)>0 && 
      [info exists bokVariables(bok_indidriver)] && $bokVariables(bok_indidriver)>0} {
    bokInform [format {%4s %-35s %s} "WARN" "IndiServer" "ALREADY RUNNING"]
    bokInform [format {%4s %-35s %s} "WARN" "bokGalilIndiDriver" "ALREADY RUNNING"]
  } else {
    set _c "/usr/bin/indiserver -vvv $env(BOK_GALIL_BIN)/bokGalilIndiDriver"
    set _l $env(BOK_GALIL_LOG)/bokGalilIndiDriver.log
    if {[file exists /usr/bin/indiserver]} {
      bokInform "INFO /usr/bin/indiserver exists"
      if {[file exists $env(BOK_GALIL_BIN)/bokGalilIndiDriver]} {
        bokInform "INFO $env(BOK_GALIL_BIN)/bokGalilIndiDriver exists"
        bokInform "INFO executing ${_c} >> ${_l} 2>> ${_l}"
        set _s [catch {exec {*}${_c} >> ${_l} 2>> ${_l} &} _r]
        if {${_s} == 0} {
          bokInform "${_c} started OK, _r=${_r}"
        } else {
          bokInform "ERROR ${_c} failed to start, status=${_s}, error='${_r}'!"
        }
      }
    }
  }

  if {[info exists bokVariables(bok_ngserver)] && $bokVariables(bok_ngserver)>0} {
    bokInform [format {%4s %-35s %s} "WARN" "Galil_DMC_22x0_NgServer" "ALREADY RUNNING"]
  } else {
    set _c $env(BOK_GALIL_BIN)/Galil_DMC_22x0_NgServer
    set _l $env(BOK_GALIL_LOG)/Galil_DMC_22x0_NgServer.log
    if {[file exists ${_c}]} {
      bokInform "INFO ${_c} exists"
      bokInform "INFO executing ${_c} >> ${_l} 2>> ${_l}"
      set _s [catch {exec ${_c} >> ${_l} 2>> ${_l} &} _r]
      if {${_s} == 0} {
        bokInform "${_c} started OK, _r=${_r}"
      } else {
        bokInform "ERROR ${_c} failed to start, status=${_s}, error='${_r}'!"
      }
    }
  }

  if {[info exists bokVariables(bok_website)] && $bokVariables(bok_website)>0} {
    bokInform [format {%4s %-35s %s} "WARN" "python3 bok.py" "ALREADY RUNNING"]
  } else {
    set _c "/home/primefocus/bok-90prime-gui/src/bok.py"
    set _l $env(BOK_GALIL_LOG)/bok.py.log
    if {[file exists ${_c}]} {
      bokInform "INFO ${_c} exists"
      bokInform "INFO executing ${_c} >> ${_l} 2>> ${_l}"
      set _s [catch {exec ${_c} >> ${_l} 2>> ${_l} &} _r]
      if {${_s} == 0} {
        bokInform "${_c} started OK, _r=${_r}"
      } else {
        bokInform "ERROR ${_c} failed to start, status=${_s}, error='${_r}'!"
      }
    }
  }

  if {[info exists bokVariables(bok_dataserver)] && $bokVariables(bok_dataserver)>0} {
    bokInform [format {%4s %-35s %s} "WARN" "/usr/local/bin/dataserver" "ALREADY RUNNING"]
  } else {
    set _c "/usr/local/bin/dataserver"
    set _l $env(BOK_GALIL_LOG)/dataserver.log
    if {[file exists ${_c}]} {
      bokInform "INFO ${_c} exists"
      bokInform "INFO executing ${_c} >> ${_l} 2>> ${_l}"
      set _s [catch {exec ${_c} >> ${_l} 2>> ${_l} &} _r]
      if {${_s} == 0} {
        bokInform "${_c} started OK, _r=${_r}"
      } else {
        bokInform "ERROR ${_c} failed to start, status=${_s}, error='${_r}'!"
      }
    }
  }

  if {[info exists bokVariables(bok_ds9)] && $bokVariables(bok_ds9)>0} {
    bokInform [format {%4s %-35s %s} "WARN" "ds9" "ALREADY RUNNING"]
  }

  if {[info exists bokVariables(bok_tcpmem)] && $bokVariables(bok_tcpmem)>0} {
    bokInform [format {%4s %-35s %s} "WARN" "/dev/shm/tcp_mem" "ALREADY PRESENT"]
  }

  if {[info exists bokVariables(bok_udpmem)] && $bokVariables(bok_udpmem)>0} {
    bokInform [format {%4s %-35s %s} "WARN" "/dev/shm/udp_mem" "ALREADY PRESENT"]
  }
}
