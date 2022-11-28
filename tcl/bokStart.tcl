proc bokStart { } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  bokClear

  set _c $env(BOK_GALIL_BIN)/Galil_DMC_22x0_Write_Memory
  set _l $env(BOK_GALIL_LOG)/Galil_DMC_22x0_Write_Memory.log
  set _s [catch {exec ${_c} >> ${_l} 2>> ${_l} &} _r]
  if {${_s} == 0} {
    bokInform "${_c} started OK, _r=${_r}"
  } else {
    bokInform "ERROR ${_c} failed to start, error='${_r}'!"
  }

  set _c "cd $env(BOK_GALIL_BIN); indiserver -vvv bokGalilIndiDriver"
  set _l $env(BOK_GALIL_LOG)/bokGalilIndiDriver.log
  set _s [catch {exec ${_c} >> ${_l} 2>> ${_l} &} _r]
  if {${_s} == 0} {
    bokInform "${_c} started OK, _r=${_r}"
  } else {
    bokInform "ERROR ${_c} failed to start, error='${_r}'!"
  }

  set _c $env(BOK_GALIL_BIN)/Galil_DMC_22x0_NgServer
  set _l $env(BOK_GALIL_LOG)/Galil_DMC_22x0_NgServer.log
  set _s [catch {exec ${_c} >> ${_l} 2>> ${_l} &} _r]
  if {${_s} == 0} {
    bokInform "${_c} started OK, _r=${_r}"
  } else {
    bokInform "ERROR ${_c} failed to start, error='${_r}'!"
  }

  set _c "/home/pndaly/bok-90prime-gui/src/bok.py"
  set _l $env(BOK_GALIL_LOG)/bok.py.log
  set _s [catch {exec ${_c} >> ${_l} 2>> ${_l} &} _r]
  if {${_s} == 0} {
    bokInform "${_c} started OK, _r=${_r}"
  } else {
    bokInform "ERROR ${_c} failed to start, error='${_r}'!"
  }

  set _c "/usr/local/bin/dataserver"
  set _l $env(BOK_GALIL_LOG)/dataserver.log
  set _s [catch {exec ${_c} >> ${_l} 2>> ${_l} &} _r]
  if {${_s} == 0} {
    bokInform "${_c} started OK, _r=${_r}"
  } else {
    bokInform "ERROR ${_c} failed to start, error='${_r}'!"
  }
}
