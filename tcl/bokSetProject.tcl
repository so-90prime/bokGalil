# +
# BOK_INSTRUMENT \ Instrument                 \ 90Prime
# BOK_INDI_ADDR  \ IndiServer Address         \ 10.130.133.206
# BOK_INDI_PORT  \ IndiServer Port            \ 7624
# BOK_NG_ADDR    \ NG Server Address          \ 10.130.133.206
# BOK_NG_PORT    \ NG Server Port             \ 5750
# BOK_TCP_ADDR   \ Galil TCP Command Address  \ 10.130.133.206
# BOK_TCP_PORT   \ Galil TCP Command Port     \ 23
# BOK_UDP_ADDR   \ Galil UDP Command Address  \ 10.130.133.206
# BOK_UDP_PORT   \ Galil UDP Command Port     \ 5078
# BOK_WEB_ADDR   \ pyINDI Website Address     \ 10.130.133.206
# BOK_WEB_PORT   \ pyINDI Website Port        \ 5905
# BOK_DATA_ADDR  \ DataServer Address         \ 10.130.133.206
# BOK_DATA_PORT  \ DataServer Port            \ 6543
# -

proc bokSetProjectQuit { W } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  return [destroy $W]
}

proc bokSetProjectInit { X } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  if { [file exists ${X}] == 0 } { return }
  set F [open ${X} r]
  while {[gets ${F} T] >= 0} {
    if { [string index $T 0] != "#" } {
      set L [split $T "\\"]
      set K [string trim [lindex $L 0]]
      set C [string trim [lindex $L 1]]
      set V [string trim [lindex $L 2]]
      set bokParams($K) $V
    }
  }
  close $F
}

proc bokSetProjectReInit { X } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  if { [file exists ${X}] == 0 } { exec rm -f ${X} }
  set F [open ${X} w]
  puts $F [format "BOK_INSTRUMENT \\ Instrument                 \\ %s" [string trim $bokParams(BOK_INSTRUMENT)]]
  puts $F [format "BOK_INDI_ADDR  \\ IndiServer Address         \\ %s" [string trim $bokParams(BOK_INDI_ADDR)]]
  puts $F [format "BOK_INDI_PORT  \\ IndiServer Port            \\ %d" [string trim $bokParams(BOK_INDI_PORT)]]
  puts $F [format "BOK_NG_ADDR    \\ NG Server Address          \\ %s" [string trim $bokParams(BOK_NG_ADDR)]]
  puts $F [format "BOK_NG_PORT    \\ NG Server Port             \\ %d" [string trim $bokParams(BOK_NG_PORT)]]
  puts $F [format "BOK_TCP_ADDR   \\ Galil TCP Command Address  \\ %s" [string trim $bokParams(BOK_TCP_ADDR)]]
  puts $F [format "BOK_TCP_PORT   \\ Galil TCP Command Port     \\ %d" [string trim $bokParams(BOK_TCP_PORT)]]
  puts $F [format "BOK_UDP_ADDR   \\ Galil UDP Command Address  \\ %s" [string trim $bokParams(BOK_UDP_ADDR)]]
  puts $F [format "BOK_UDP_PORT   \\ Galil UDP Command Port     \\ %d" [string trim $bokParams(BOK_UDP_PORT)]]
  puts $F [format "BOK_WEB_ADDR   \\ pyINDI Website Address     \\ %s" [string trim $bokParams(BOK_WEB_ADDR)]]
  puts $F [format "BOK_WEB_PORT   \\ pyINDI Website Port        \\ %d" [string trim $bokParams(BOK_WEB_PORT)]]
  puts $F [format "BOK_DATA_ADDR  \\ DataServer Address         \\ %s" [string trim $bokParams(BOK_DATA_ADDR)]]
  puts $F [format "BOK_DATA_PORT  \\ DataServer Port            \\ %d" [string trim $bokParams(BOK_DATA_PORT)]]
  flush $F
  close $F
}

proc bokSetProjectSave { W } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  set bokParams(BOK_INSTRUMENT) [string trim [$bokWidgets(GUI_INSTRUMENT) get]]
  set bokParams(BOK_INDI_ADDR)  [string trim [$bokWidgets(GUI_INDI_ADDR) get]]
  set bokParams(BOK_INDI_PORT)  [string trim [$bokWidgets(GUI_INDI_PORT) get]]
  set bokParams(BOK_NG_ADDR)    [string trim [$bokWidgets(GUI_NG_ADDR) get]]
  set bokParams(BOK_NG_PORT)    [string trim [$bokWidgets(GUI_NG_PORT) get]]
  set bokParams(BOK_TCP_ADDR)   [string trim [$bokWidgets(GUI_TCP_ADDR) get]]
  set bokParams(BOK_TCP_PORT)   [string trim [$bokWidgets(GUI_TCP_PORT) get]]
  set bokParams(BOK_UDP_ADDR)   [string trim [$bokWidgets(GUI_UDP_ADDR) get]]
  set bokParams(BOK_UDP_PORT)   [string trim [$bokWidgets(GUI_UDP_PORT) get]]
  set bokParams(BOK_WEB_ADDR)   [string trim [$bokWidgets(GUI_WEB_ADDR) get]]
  set bokParams(BOK_WEB_PORT)   [string trim [$bokWidgets(GUI_WEB_PORT) get]]
  set bokParams(BOK_DATA_ADDR)  [string trim [$bokWidgets(GUI_DATA_ADDR) get]]
  set bokParams(BOK_DATA_PORT)  [string trim [$bokWidgets(GUI_DATA_PORT) get]]
  bokSetProjectReInit $env(BOK_GALIL_TCL)/bokParams.txt
  bokSetProjectQuit $W
}

proc bokSetProject { W } {

  # global(s)
  global env bokParams bokVariables bokVersions bokWidgets bokXopt

  # top level and frame(s)
  if { [winfo exists $W] } { destroy $W }
  toplevel $W
  if {[info exists bokParams(BOK_INSTRUMENT)]} {
    wm title $W "$bokParams(BOK_INSTRUMENT) Project Parameter(s)"
  } else {
    wm title $W "Project Parameter(s)"
  }
  set U [frame ${W}.u]
  set L [frame ${W}.l]
  pack $U $L -in $W -side top -fill both -expand yes

  # create Save and Quit buttons
  set bokWidgets(GUI_SAVE) [button ${L}.ok -text Save]
  pack $bokWidgets(GUI_SAVE) -in $L -side left -fill both -expand y
  $bokWidgets(GUI_SAVE) configure -command { bokSetProjectSave [winfo parent [winfo parent $bokWidgets(GUI_SAVE)]] }
  pack [button ${L}.cancel -text Cancel -command "bokSetProjectQuit ${W}"] -side left -fill both -expand y

  # init params
  bokSetProjectInit $env(BOK_GALIL_TCL)/bokParams.txt

  # create bokect parameter(s)
  set U1 [frame ${U}.u1]
  set U2 [frame ${U}.u2]
  set U3 [frame ${U}.u3]
  set U4 [frame ${U}.u4]
  set U5 [frame ${U}.u5]
  set U6 [frame ${U}.u6]
  set U7 [frame ${U}.u7]
  set U8 [frame ${U}.u8]
  set U9 [frame ${U}.u9]
  set UA [frame ${U}.ua]
  set UB [frame ${U}.ub]
  set UC [frame ${U}.uc]
  set UD [frame ${U}.ud]
  pack $U1 $U2 $U3 $U4 $U5 $U6 $U7 $U8 $U9 $UA $UB $UC $UD -side top -fill both -expand yes
  set bokWidgets(GUI_INSTRUMENT)  [entry ${U1}.e -width 50 -textvariable bokParams(BOK_INSTRUMENT)]
  set bokWidgets(GUI_INDI_ADDR)   [entry ${U2}.e -width 50 -textvariable bokParams(BOK_INDI_ADDR)]
  set bokWidgets(GUI_INDI_PORT)   [entry ${U3}.e -width 50 -textvariable bokParams(BOK_INDI_PORT)]
  set bokWidgets(GUI_NG_ADDR)     [entry ${U4}.e -width 50 -textvariable bokParams(BOK_NG_ADDR)]
  set bokWidgets(GUI_NG_PORT)     [entry ${U5}.e -width 50 -textvariable bokParams(BOK_NG_PORT)]
  set bokWidgets(GUI_TCP_ADDR)    [entry ${U6}.e -width 50 -textvariable bokParams(BOK_TCP_ADDR)]
  set bokWidgets(GUI_TCP_PORT)    [entry ${U7}.e -width 50 -textvariable bokParams(BOK_TCP_PORT)]
  set bokWidgets(GUI_UDP_ADDR)    [entry ${U8}.e -width 50 -textvariable bokParams(BOK_UDP_ADDR)]
  set bokWidgets(GUI_UDP_PORT)    [entry ${U9}.e -width 50 -textvariable bokParams(BOK_UDP_PORT)]
  set bokWidgets(GUI_WEB_ADDR)    [entry ${UA}.e -width 50 -textvariable bokParams(BOK_WEB_ADDR)]
  set bokWidgets(GUI_WEB_PORT)    [entry ${UB}.e -width 50 -textvariable bokParams(BOK_WEB_PORT)]
  set bokWidgets(GUI_DATA_ADDR)   [entry ${UC}.e -width 50 -textvariable bokParams(BOK_DATA_ADDR)]
  set bokWidgets(GUI_DATA_PORT)   [entry ${UD}.e -width 50 -textvariable bokParams(BOK_DATA_PORT)]

  pack [label ${U1}.l -text "Instrument:"                -width 20 -anchor e] $bokWidgets(GUI_INSTRUMENT) -side left -fill both -expand y
  pack [label ${U2}.l -text "IndiServer Address:"        -width 20 -anchor e] $bokWidgets(GUI_INDI_ADDR)  -side left -fill both -expand y
  pack [label ${U3}.l -text "IndiServer Port:"           -width 20 -anchor e] $bokWidgets(GUI_INDI_PORT)  -side left -fill both -expand y
  pack [label ${U4}.l -text "NG Server Address:"         -width 20 -anchor e] $bokWidgets(GUI_NG_ADDR)    -side left -fill both -expand y
  pack [label ${U5}.l -text "NG Server Port:"            -width 20 -anchor e] $bokWidgets(GUI_NG_PORT)    -side left -fill both -expand y
  pack [label ${U6}.l -text "Galil TCP Command Address:" -width 20 -anchor e] $bokWidgets(GUI_TCP_ADDR)   -side left -fill both -expand y
  pack [label ${U7}.l -text "Galil TCP Command Port:"    -width 20 -anchor e] $bokWidgets(GUI_TCP_PORT)   -side left -fill both -expand y
  pack [label ${U8}.l -text "Galil UDP Command Address:" -width 20 -anchor e] $bokWidgets(GUI_UDP_ADDR)   -side left -fill both -expand y
  pack [label ${U9}.l -text "Galil UDP Command Port:"    -width 20 -anchor e] $bokWidgets(GUI_UDP_PORT)   -side left -fill both -expand y
  pack [label ${UA}.l -text "pyINDI Website Address:"    -width 20 -anchor e] $bokWidgets(GUI_WEB_ADDR)   -side left -fill both -expand y
  pack [label ${UB}.l -text "pyINDI Website Port:"       -width 20 -anchor e] $bokWidgets(GUI_WEB_PORT)   -side left -fill both -expand y
  pack [label ${UC}.l -text "DataServer Address:"        -width 20 -anchor e] $bokWidgets(GUI_DATA_ADDR)  -side left -fill both -expand y
  pack [label ${UD}.l -text "DataServer Port:"           -width 20 -anchor e] $bokWidgets(GUI_DATA_PORT)  -side left -fill both -expand y
}
