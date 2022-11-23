proc bokUpdateWidgets { W } {

  # global(s)
  global env bokLocals bokVersions bokVariables bokWidgets bokXopts

  # continue?
  # puts stdout "update gui"
  update
  # puts stdout "updating widget(s)"
  bokUpdateVariables
  after 1500 bokUpdateWidgets ${W}
}
