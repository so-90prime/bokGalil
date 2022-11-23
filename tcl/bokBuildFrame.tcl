proc bokBuildFrame { W } {

  # global(s)
  global env bokLocals bokVariables bokVersions bokWidgets bokXopts

  # top level
  toplevel $W
  wm withdraw $W
  frame ${W}.mb -relief raised -bd 2
  pack ${W}.mb -fill x

  # file menu
  menubutton ${W}.mb.file -text File -menu ${W}.mb.file.menu -underline 0
  pack ${W}.mb.file -side left
  menubutton ${W}.mb.help -text Help -menu ${W}.mb.help.menu -underline 0
  pack ${W}.mb.help -side right

  menu ${W}.mb.file.menu
  ${W}.mb.file.menu add command -underline 5 -label "Show Locals"    -command "bokShowLocals"
  ${W}.mb.file.menu add command -underline 5 -label "Show Variables" -command "bokShowVariables"
  ${W}.mb.file.menu add command -underline 5 -label "Show Widgets"   -command "bokShowWidgets"
  ${W}.mb.file.menu add command -underline 1 -label Exit -command "bokExit 0"

  menu ${W}.mb.help.menu
  ${W}.mb.help.menu add command -underline 0 -label About -command "bokAbout $W"

  # build the widget tree
  pack [bokBuildWidgets $W]

  # set W titles
  wm title $W "$bokLocals(instrument) Status"
  wm iconname $W $bokLocals(instrument)
  wm minsize  $W 500 375
  wm protocol $W WM_DELETE_WINDOW "bokExit 0"

  # initialise the task
  after 500
  wm withdraw .
  wm deiconify $W
  return $W
}
