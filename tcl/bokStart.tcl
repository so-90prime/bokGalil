proc bokStart { } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  # Galil_DMC_22x0_Write_Memory
  set _c $env(BOK_GALIL_BIN)/Galil_DMC_22x0_Write_Memory
  set _l $env(BOK_GALIL_LOG)/Galil_DMC_22x0_Write_Memory.log
  set _s [catch {exec nohup ${_c} >> ${_l} 2>> ${_l} &} _r]
  if {${_s} == 0} {
    bokInform "${_c} started OK"
  } else {
    bokInform "ERROR ${_c} failed to start, error='${_r}'!"
  }
  # indiserver / bokGalilIndiDriver
  set _c "indiserver -vv $env(BOK_GALIL_BIN)/bokGalilIndiDriver"
  set _l $env(BOK_GALIL_LOG)/bokGalilIndiDriver.log
  set _s [catch {exec nohup ${_c} >> ${_l} 2>> ${_l} &} _r]
  if {${_s} == 0} {
    bokInform "${_c} started OK"
  } else {
    bokInform "ERROR ${_c} failed to start, error='${_r}'!"
  }
  # Galil_DMC_22x0_NgServer
  set _c $env(BOK_GALIL_BIN)/Galil_DMC_22x0_NgServer
  set _l $env(BOK_GALIL_LOG)/Galil_DMC_22x0_NgServer.log
  set _s [catch {exec nohup ${_c} >> ${_l} 2>> ${_l} &} _r]
  if {${_s} == 0} {
    bokInform "${_c} started OK"
  } else {
    bokInform "ERROR ${_c} failed to start, error='${_r}'!"
  }
  # bok.py
  set _c "python3 $env(HOME)/bok-90prime-gui/src/bok.py"
  set _l $env(BOK_GALIL_LOG)/bok.py.log
  set _s [catch {exec nohup ${_c} >> ${_l} 2>> ${_l} &} _r]
  if {${_s} == 0} {
    bokInform "${_c} started OK"
  } else {
    bokInform "ERROR ${_c} failed to start, error='${_r}'!"
  }
  # dataserver / ds9
  set _c "xterm -e $env(HOME)/dataserver/dataserver"
  set _l $env(BOK_GALIL_LOG)/dataserver.log
  set _s [catch {exec nohup ${_c} >> ${_l} 2>> ${_l} &} _r]
  if {${_s} == 0} {
    bokInform "${_c} started OK"
  } else {
    bokInform "ERROR ${_c} failed to start, error='${_r}'!"
  }
}
