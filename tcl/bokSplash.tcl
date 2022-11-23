#!/bin/sh
# the next line restarts using wish \
exec `which wish` "$0" "$@"

  # global(s)
  package require csv
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  auto_mkindex $env(BOK_GALIL_TCL) $env(BOK_GALIL_TCL)/*.tcl
  lappend auto_path $env(BOK_GALIL_TCL)

  # init everything and create frame
  bokSetXopt $env(BOK_GALIL_TCL)/bokSplash.xopt
  bokSetProjectInit $env(BOK_GALIL_TCL)/bokParams.txt
  bokInit
  bokBuildFrame .bokGalil

  # event loop for widgets
  bokUpdateWidgets .bokGalil
