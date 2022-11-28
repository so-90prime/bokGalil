proc bokBindWidgets { W } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  $bokWidgets(bok_text_widget) tag configure errs -background pink -foreground red -relief raised
  $bokWidgets(bok_text_widget) tag configure info -background green -foreground yellow -relief raised
  $bokWidgets(bok_text_widget) tag configure warn -background thistle -foreground blue -relief raised
}
