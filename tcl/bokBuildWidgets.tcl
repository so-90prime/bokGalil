proc bokBuildWidgets { W } {

  # global(s)
  global env bokLocals bokVariables bokVersions bokWidgets bokXopts

  # create top level (T)
  set T [frame ${W}.main]
  pack ${T} -fill both -expand yes -side top

  # create 2 sub-frames
  set bokWidgets(upper) [frame ${T}.upper]
  pack $bokWidgets(upper) -fill both -expand yes -side top
  set bokWidgets(lower) [frame ${T}.lower]
  pack $bokWidgets(lower) -fill both -expand yes -side top

  # create label(s) in upper
  set bokWidgets(bok_memory)       [label $bokWidgets(upper).memory -text "Shared Memory Handler"]
  pack $bokWidgets(bok_memory)     -side top -expand yes -fill both
  set bokWidgets(bok_tcpmem)       [label $bokWidgets(upper).tcpmem -text "TCP Memory Segment"]
  pack $bokWidgets(bok_tcpmem)     -side top -expand yes -fill both
  set bokWidgets(bok_udpmem)       [label $bokWidgets(upper).udpmem -text "UDP Memory Segment"]
  pack $bokWidgets(bok_udpmem)     -side top -expand yes -fill both
  set bokWidgets(bok_indidriver)   [label $bokWidgets(upper).indidriver -text bokGalilIndiDriver]
  pack $bokWidgets(bok_indidriver) -side top -expand yes -fill both
  set bokWidgets(bok_indiserver)   [label $bokWidgets(upper).indiserver -text "IndiServer ($bokLocals(BOK_INDI_ADDR):$bokLocals(BOK_INDI_PORT))"]
  pack $bokWidgets(bok_indiserver) -side top -expand yes -fill both
  set bokWidgets(bok_ngserver)     [label $bokWidgets(upper).ngserver -text "Ng Server ($bokLocals(BOK_NG_ADDR):$bokLocals(BOK_NG_PORT))"]
  pack $bokWidgets(bok_ngserver)   -side top -expand yes -fill both
  set bokWidgets(bok_website)      [label $bokWidgets(upper).website -text "http://$bokLocals(BOK_WEB_ADDR):$bokLocals(BOK_WEB_PORT)"]
  pack $bokWidgets(bok_website)    -side top -expand yes -fill both
  set bokWidgets(bok_dataserver)   [label $bokWidgets(upper).dataserver -text DataServer]
  pack $bokWidgets(bok_dataserver) -side top -expand yes -fill both
  set bokWidgets(bok_ds9)          [label $bokWidgets(upper).ds9 -text DS9]
  pack $bokWidgets(bok_ds9)        -side top -expand yes -fill both

  # create button(s) in lower
  set bokWidgets(start) [button $bokWidgets(lower).start -text "START" -command "puts start"]
  pack $bokWidgets(start) -fill both -expand yes -side left
  set bokWidgets(exit) [button $bokWidgets(lower).exit -text "EXIT" -command "bokExit 0"]
  pack $bokWidgets(exit) -fill both -expand yes -side left
  set bokWidgets(stop) [button $bokWidgets(lower).stop -text "STOP" -command bokStop]
  pack $bokWidgets(stop) -fill both -expand yes -side left

  # return
  return ${T}
}
