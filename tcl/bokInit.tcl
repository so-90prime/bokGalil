proc bokAbout { W } {
  global env bokLocals bokVersions bokVariables bokWidgets bokXopts
  bokSetXopts bokSplash.xdialog
  tk_dialog ${W}.dialog "About ..." $bokVersions(HELP) info 0 Dismiss
  bokSetXopts bokSplash.xopt
  return 0
}

proc bokExit { N } {
  global env bokLocals bokVersions bokVariables bokWidgets bokXopts
  trace vdelete bokVariables(bok_memory)     w bokTraceVariables
  trace vdelete bokVariables(bok_tcpmem)     w bokTraceVariables
  trace vdelete bokVariables(bok_udpmem)     w bokTraceVariables
  trace vdelete bokVariables(bok_indidriver) w bokTraceVariables
  trace vdelete bokVariables(bok_indiserver) w bokTraceVariables
  trace vdelete bokVariables(bok_ngserver)   w bokTraceVariables
  trace vdelete bokVariables(bok_website)    w bokTraceVariables
  trace vdelete bokVariables(bok_dataserver) w bokTraceVariables
  trace vdelete bokVariables(bok_ds9)        w bokTraceVariables
  exit ${N}
}

proc bokInit { } {
  global env bokLocals bokVersions bokVariables bokWidgets bokXopts
  bokInitLocals
  bokInitVersions
  bokSetXopts bokSplash.xopt
  trace variable bokVariables(bok_memory)     w bokTraceVariables
  trace variable bokVariables(bok_tcpmem)     w bokTraceVariables
  trace variable bokVariables(bok_udpmem)     w bokTraceVariables
  trace variable bokVariables(bok_indidriver) w bokTraceVariables
  trace variable bokVariables(bok_indiserver) w bokTraceVariables
  trace variable bokVariables(bok_ngserver)   w bokTraceVariables
  trace variable bokVariables(bok_website)    w bokTraceVariables
  trace variable bokVariables(bok_dataserver) w bokTraceVariables
  trace variable bokVariables(bok_ds9)        w bokTraceVariables
  bokUpdateVariables
}

proc bokInitLocals { } {
  global env bokLocals bokVersions bokVariables bokWidgets bokXopts
  set bokLocals(instrument) "90Prime"
}

proc bokInitVersions { } {
  global env bokLocals bokVersions bokVariables bokWidgets bokXopts tcl_version tk_version
  set bokVersions(AUTHOR)  "Philip N. Daly"
  set bokVersions(EMAIL)   "pndaly@arizona.edu"
  set bokVersions(DATE)    "21 November 2022"
  set bokVersions(VERSION) 20221121
  set bokVersions(OWNER)   "Steward Observatory"
  set bokVersions(HELP) "$bokLocals(instrument):\t v$bokVersions(VERSION)\nCopyLeft:\t $bokVersions(OWNER)\nAuthor:\t $bokVersions(AUTHOR)\nE-mail:\t $bokVersions(EMAIL)\nDate:\t $bokVersions(DATE)\nTcl:\t v$tcl_version\nTk:\t v$tk_version\n"
}

proc bokPidOf { N } {
  global env bokLocals bokVersions bokVariables bokWidgets bokXopts
  set _s [catch {exec pidof -zcxd, ${N}} _r]
  if {${_s} == 0} {return ${_r}}
  if {${_s} == 1 && [string compare -nocase ${_r} "child process exited abnormally"]==0} {return 0}
  return "${_s}:${_r}" 
}

proc bokPidOf2 { N } {
  global env bokLocals bokVersions bokVariables bokWidgets bokXopts
  set _s [catch {exec ps -ef | grep -v grep | grep ${N}} _r]
  if {${_s} == 0} {
    return [lindex [csv::split [string map {" " ","} [regexp -all -inline {\w+} ${_r}]]] 1]
  } else {
    return 0
  }
}

proc bokSetXopts { F } {
  global env bokLocals bokVersions bokVariables bokWidgets bokXopts
  if { [file exists $F] == 0 } { return }
  set D [open $F r]
  while { [gets $D T] >= 0 } {
    if { ([llength $T]==2) && ([string index $T 0]!="#") } { set bokXopts([lindex $T 0]) [lindex $T 1] }
  }
  close $D
  foreach O [array names bokXopts {x*}] { option add *[string range $O 1 end] $bokXopts($O) }
}

proc bokShowLocals { } {
  global env bokLocals bokVersions bokVariables bokWidgets bokXopts
  foreach E [array names bokLocals] { puts stdout "\nbokShowLocals <INFO>: $E = $bokLocals($E)" }
}

proc bokShowVariables { } {
  global env bokLocals bokVersions bokVariables bokWidgets bokXopts
  foreach E [array names bokVariables] { puts stdout "\nbokShowVariables <INFO>: $E = $bokVariables($E)" }
}

proc bokShowWidgets { } {
  global env bokLocals bokVersions bokVariables bokWidgets bokXopts
  foreach E [array names bokWidgets] { puts stdout "\nbokShowWidgets <INFO>: $E = $bokWidgets($E)" }
}

proc bokStop { } {
  global env bokLocals bokVersions bokVariables bokWidgets bokXopts
  if {[info exists bokVariables(bok_indidriver)] && $bokVariables(bok_indidriver) > 0} {
    puts "\nbokStop <bok_indidriver>: after 500 bokTerminate $bokVariables(bok_indidriver)"
    after 500 bokTerminate $bokVariables(bok_indidriver)
  }
  if {[info exists bokVariables(bok_indiserver)] && $bokVariables(bok_indiserver) > 0} {
    puts "\nbokStop <bok_indiserver>: after 500 bokTerminate $bokVariables(bok_indiserver)"
    after 500 bokTerminate $bokVariables(bok_indiserver)
  }
  if {[info exists bokVariables(bok_ngserver)] && $bokVariables(bok_ngserver) > 0} {
    puts "\nbokStop <bok_ngserver>: after 500 bokTerminate $bokVariables(bok_ngserver)"
    after 500 bokTerminate $bokVariables(bok_ngserver)
  }
  if {[info exists bokVariables(bok_memory)] && $bokVariables(bok_memory) > 0} {
    puts "\nbokStop <bok_memory>: after 500 bokTerminate $bokVariables(bok_memory)"
    after 500 bokTerminate $bokVariables(bok_memory)
  }
  if {[info exists bokVariables(bok_website)] && $bokVariables(bok_website) > 0} {
    puts "\nbokStop <bok_webiste>: after 500 bokTerminate $bokVariables(bok_website)"
    after 500 bokTerminate $bokVariables(bok_website)
  }
  if {[info exists bokVariables(bok_ds9)] && $bokVariables(bok_ds9) > 0} {
    puts "\nbokStop <bok_dataserver>: after 500 bokTerminate $bokVariables(bok_ds9)"
    after 500 bokTerminate $bokVariables(bok_ds9)
  }
  if {[info exists bokVariables(bok_dataserver)] && $bokVariables(bok_dataserver) > 0} {
    puts "\nbokStop <bok_dataserver>: after 500 bokTerminate $bokVariables(bok_dataserver)"
    after 500 bokTerminate $bokVariables(bok_dataserver)
  }
  if {[info exists bokVariables(bok_tcpmem)] && $bokVariables(bok_tcpmem) > 0} {
    puts "\nbokStop <bok_tcpmem>: file delete -force /dev/shm/tcp_mem"
    file delete -force /dev/shm/tcp_shm
  }
  if {[info exists bokVariables(bok_udpmem)] && $bokVariables(bok_udpmem) > 0} {
    puts "\nbokStop <bok_udpmem>: file delete -force /dev/shm/udp_mem"
    file delete -force /dev/shm/udp_shm
  }
}

proc bokTerminate { N } {
  global env bokLocals bokVersions bokVariables bokWidgets bokXopts
  set _s [catch {exec kill -9 ${N}} _r]
  if {${_s} == 0} {return ${_s}}
  return "${_s}:${_r}" 
}

proc bokTraceVariables { name element op } {
  global env bokLocals bokVersions bokVariables bokWidgets bokXopts
  if { ${element} != "" } { set name ${name}(${element}) }
  upvar ${name} x
  set _elem [string trim ${element}]
  if { [info exists bokWidgets(${_elem})] } {
    if { $x == 0 } {
      $bokWidgets(${_elem}) configure -bg LightSalmon -fg DarkRed
    } else {
      $bokWidgets(${_elem}) configure -bg PaleGreen -fg DarkGreen
    }
  }
}

proc bokUpdateVariables { } {
  global env bokLocals bokVersions bokVariables bokWidgets bokXopts
  set bokVariables(bok_memory)     [bokPidOf Galil_DMC_22x0_Write_Memory]
  set bokVariables(bok_tcpmem)     [file exists /dev/shm/tcp_shm] 
  set bokVariables(bok_udpmem)     [file exists /dev/shm/udp_shm] 
  set bokVariables(bok_indidriver) [bokPidOf bokGalilIndiDriver]
  set bokVariables(bok_indiserver) [bokPidOf indiserver]
  set bokVariables(bok_ngserver)   [bokPidOf Galil_DMC_22x0_NgServer]
  set bokVariables(bok_website)    [bokPidOf2 bok.py]
  set bokVariables(bok_dataserver) [bokPidOf dataserver]
  set bokVariables(bok_ds9)        [bokPidOf2 ds9.tcl]
}
