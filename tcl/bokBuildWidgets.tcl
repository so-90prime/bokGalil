proc bokBuildWidgets { W } {

  # global(s)
  global env bokParams bokVariables bokVersions bokWidgets bokXopt

  # create top level (T)
  set T [frame ${W}.main]
  pack ${T} -fill both -expand yes -side top

  # create 3 sub-frames
  set bokWidgets(bok_upper) [frame ${T}.bok_upper -relief sunken]
  pack $bokWidgets(bok_upper) -fill both -expand yes -side top

  set bokWidgets(bok_middle) [frame ${T}.bok_middle -relief sunken]
  pack $bokWidgets(bok_middle) -fill both -expand yes -side top 

  set bokWidgets(bok_lower) [frame ${T}.bok_lower -relief sunken]
  pack $bokWidgets(bok_lower) -fill both -expand yes -side top 

  set bokWidgets(bok_basement) [frame ${T}.bok_basement -relief sunken]
  pack $bokWidgets(bok_basement) -fill both -expand yes -side top 

  # create scrolling region for output
  set bokWidgets(bok_scroll_bar) [scrollbar $bokWidgets(bok_upper).scrollbar -orient vertical -relief sunken -bd 2]
  set bokWidgets(bok_text_widget) [text $bokWidgets(bok_upper).text -state disabled -wrap word -relief sunken -bd 2 -width 0]
  $bokWidgets(bok_scroll_bar) configure -command "$bokWidgets(bok_text_widget) yview"
  $bokWidgets(bok_text_widget) configure -yscroll "$bokWidgets(bok_scroll_bar) set"
  pack $bokWidgets(bok_scroll_bar) -side right -fill y
  pack $bokWidgets(bok_text_widget) -side right -fill both -expand yes

  # create label(s) in middle
  set bokWidgets(bok_memory)       [label $bokWidgets(bok_middle).memory      -textvariable bokVariables(txt_memory)]
  pack $bokWidgets(bok_memory)     -side top -expand yes -fill both
  set bokWidgets(bok_tcpmem)       [label $bokWidgets(bok_middle).tcpmem      -textvariable bokVariables(txt_tcpmem)]
  pack $bokWidgets(bok_tcpmem)     -side top -expand yes -fill both
  set bokWidgets(bok_udpmem)       [label $bokWidgets(bok_middle).udpmem      -textvariable bokVariables(txt_udpmem)]
  pack $bokWidgets(bok_udpmem)     -side top -expand yes -fill both
  set bokWidgets(bok_indidriver)   [label $bokWidgets(bok_middle).indidriver  -textvariable bokVariables(txt_indidriver)]
  pack $bokWidgets(bok_indidriver) -side top -expand yes -fill both
  set bokWidgets(bok_indiserver)   [label $bokWidgets(bok_middle).indiserver  -textvariable bokVariables(txt_indiserver)]
  pack $bokWidgets(bok_indiserver) -side top -expand yes -fill both
  set bokWidgets(bok_ngserver)     [label $bokWidgets(bok_middle).ngserver    -textvariable bokVariables(txt_ngserver)]
  pack $bokWidgets(bok_ngserver)   -side top -expand yes -fill both
  set bokWidgets(bok_website)      [label $bokWidgets(bok_middle).website     -textvariable bokVariables(txt_website)]
  pack $bokWidgets(bok_website)    -side top -expand yes -fill both
  set bokWidgets(bok_dataserver)   [label $bokWidgets(bok_middle).dataserver  -textvariable bokVariables(txt_dataserver)]
  pack $bokWidgets(bok_dataserver) -side top -expand yes -fill both
  set bokWidgets(bok_flatfield)    [label $bokWidgets(bok_middle).flatfield   -textvariable bokVariables(txt_flatfield)]
  pack $bokWidgets(bok_flatfield)  -side top -expand yes -fill both
  set bokWidgets(bok_ds9)          [label $bokWidgets(bok_middle).ds9         -textvariable bokVariables(txt_ds9)]
  pack $bokWidgets(bok_ds9)        -side top -expand yes -fill both

  # create button(s)
  set bokWidgets(bok_start) [button $bokWidgets(bok_lower).bok_start -text "Start" -command bokStart]
  pack $bokWidgets(bok_start) -fill both -expand yes -side left
  set bokWidgets(bok_status) [button $bokWidgets(bok_lower).bok_status -text "Status" -command bokStatus]
  pack $bokWidgets(bok_status) -fill both -expand yes -side left
  set bokWidgets(bok_stop) [button $bokWidgets(bok_lower).bok_stop -text "Stop" -command bokStop]
  pack $bokWidgets(bok_stop) -fill both -expand yes -side left

  # create label
  pack [label $bokWidgets(bok_basement).banner -fg blue -text "$bokParams(BOK_INSTRUMENT) v$bokVersions(VERSION) (C) $bokVersions(OWNER), Contact: $bokVersions(AUTHOR) $bokVersions(EMAIL)" -font -adobe-helvetica-italic-r-normal--*-110-* -relief sunken] -expand yes -fill both -side top 

  # return
  bokBindWidgets ${T}
  return ${T}
}
