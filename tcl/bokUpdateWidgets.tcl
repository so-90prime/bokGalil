proc bokUpdateWidgets { W } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  update
  bokUpdateVariables
  after 1500 bokUpdateWidgets ${W}
}
