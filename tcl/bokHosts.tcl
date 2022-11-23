proc bokHosts { } {
  global env bokLocals bokVersions bokVariables bokWidgets bokXopts
  set bokLocals(_GALIL_) "192.168.0.100"
  set bokLocals(BOK_GALIL_CMD_BOK) "192.168.0.100"
  set bokLocals(BOK_GALIL_CMD_LAB) "192.168.0.100"
  set bokLocals(BOK_INDI_ADDR) "192.168.0.161"
  set bokLocals(BOK_INDI_PORT)     7624
  set bokLocals(BOK_NG_ADDR) "192.168.0.161"
  set bokLocals(BOK_NG_PORT)       5750
  set bokLocals(BOK_TCP_ADDR) "192.168.0.161"
  set bokLocals(BOK_TCP_PORT)      23
  set bokLocals(BOK_UDP_ADDR) "192.168.0.161"
  set bokLocals(BOK_UDP_PORT)      5078
  set bokLocals(BOK_WEB_ADDR) "192.168.0.161"
  set bokLocals(BOK_WEB_PORT)      5905
}
