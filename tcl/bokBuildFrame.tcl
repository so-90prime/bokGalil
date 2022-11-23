proc bokBuildFrame { W } {

  # global(s)
  global env bokParams bokVariables bokVersions bokWidgets bokXopt

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
  ${W}.mb.file.menu add command -underline 5 -label "Show Parameter(s)" -command bokShowParameters
  ${W}.mb.file.menu add command -underline 5 -label "Show Variable(s)"  -command bokShowVariables
  ${W}.mb.file.menu add command -underline 5 -label "Show Widget(s)"    -command bokShowWidgets
  ${W}.mb.file.menu add command -underline 5 -label "Show Xopt(s)"      -command bokShowXopt
  ${W}.mb.file.menu add separator
  ${W}.mb.file.menu add command -underline 1 -label "Clear Text Widget"    -command bokClear
  ${W}.mb.file.menu add command -underline 1 -label "Set Project"          -command "bokSetProject .proj"
  ${W}.mb.file.menu add command -underline 1 -label "Exit Application GUI" -command "bokExit 0"

  menu ${W}.mb.help.menu
  ${W}.mb.help.menu add command -underline 0 -label About -command "bokAbout $W"

  # build the widget tree
  pack [bokBuildWidgets $W]

  # set W titles
  wm title $W "$bokParams(BOK_INSTRUMENT) Status"
  wm iconname $W $bokParams(BOK_INSTRUMENT)
  # wm minsize  $W 500 375
  wm minsize  $W 750 500
  wm protocol $W WM_DELETE_WINDOW "bokExit 0"

  # initialise the task
  after 500
  update idletasks
  wm withdraw .
  wm deiconify $W
  return $W
}
