#!/bin/sh
# the next line restarts using wish \
exec `which wish` "$0" "$@"

  # global(s)
  package require csv
  global env bokLocals bokVersions bokVariables bokWidgets bokXopts
  auto_mkindex $env(BOK_GALIL_TCL) $env(BOK_GALIL_TCL)/*.tcl
  lappend auto_path $env(BOK_GALIL_TCL)

  # init everything and create frame
  bokInit
  bokHosts
  bokBuildFrame .bokGalil

  # event loop for widgets
  bokUpdateWidgets .bokGalil
