proc bokAbout { W } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  tk_dialog ${W}.dialog "About ..." $bokVersions(HELP) info 0 Dismiss
  return 0
}

proc bokClear { } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  $bokWidgets(bok_text_widget) configure -state normal
  $bokWidgets(bok_text_widget) delete 1.0 end
  $bokWidgets(bok_text_widget) configure -state disabled
}

proc bokExit { N } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
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

proc bokInform { M } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  if { $bokWidgets(bok_text_widget) != "" } {
    scan [$bokWidgets(bok_text_widget) index end] %d N
    if { $N > 250 } { bokClear }
    $bokWidgets(bok_text_widget) configure -state normal
    $bokWidgets(bok_text_widget) insert end "$M\n"
    $bokWidgets(bok_text_widget) yview -pickplace end
    $bokWidgets(bok_text_widget) configure -state disabled
    bokStringMatch ERROR " $bokWidgets(bok_text_widget) tag add errs first last "
    bokStringMatch INFO  " $bokWidgets(bok_text_widget) tag add info first last "
    bokStringMatch WARN  " $bokWidgets(bok_text_widget) tag add warn first last "
  }
}

proc bokInit { } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  bokInitVersions
  set bokVariables(txt_memory)     "Shared Memory Handler"
  set bokVariables(txt_tcpmem)     "TCP Memory Segment (/dev/shm/tcp_mem)"
  set bokVariables(txt_udpmem)     "UDP Memory Segment (/dev/shm/udp_mem)"
  set bokVariables(txt_indidriver) "bokGalilIndiDriver"
  set bokVariables(txt_indiserver) "IndiServer (host=$bokParams(BOK_INDI_ADDR) port=$bokParams(BOK_INDI_PORT))"
  set bokVariables(txt_ngserver)   "NG Server (host=$bokParams(BOK_NG_ADDR) port=$bokParams(BOK_NG_PORT))"
  set bokVariables(txt_website)    "WebServer (http://$bokParams(BOK_WEB_ADDR):$bokParams(BOK_WEB_PORT)/)"
  set bokVariables(txt_dataserver) "DataServer (host=$bokParams(BOK_DATA_ADDR) port=$bokParams(BOK_DATA_PORT))"
  set bokVariables(txt_ds9)        "ds9"
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

proc bokInitVersions { } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt tcl_version tk_version
  set bokVersions(AUTHOR)  "Philip N. Daly"
  set bokVersions(EMAIL)   "pndaly@arizona.edu"
  set bokVersions(DATE)    "21 November 2022"
  set bokVersions(VERSION) 20221121
  set bokVersions(OWNER)   "Steward Observatory"
  set bokVersions(HELP) "$bokParams(BOK_INSTRUMENT):\t v$bokVersions(VERSION)\nCopyLeft:\t $bokVersions(OWNER)\nAuthor:\t $bokVersions(AUTHOR)\nE-mail:\t $bokVersions(EMAIL)\nDate:\t $bokVersions(DATE)\nTcl:\t v$tcl_version\nTk:\t v$tk_version\n"
}

proc bokPidOf { N } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  set _s [catch {exec pidof -zcxd, ${N}} _r]
  if {${_s} == 0} {return ${_r}}
  if {${_s} == 1 && [string compare -nocase ${_r} "child process exited abnormally"]==0} {return 0}
  return "${_s}:${_r}" 
}

proc bokPidOf2 { N } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  set _s [catch {exec ps -ef | grep -v grep | grep ${N}} _r]
  if {${_s} == 0} {
    return [lindex [csv::split [string map {" " ","} [regexp -all -inline {\w+} ${_r}]]] 1]
  } else {
    return 0
  }
}

proc bokSetXopt { F } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  if { [file exists $F] == 0 } { return }
  set D [open $F r]
  while { [gets $D T] >= 0 } {
    if { ([llength $T]==2) && ([string index $T 0]!="#") } { set bokXopt([lindex $T 0]) [lindex $T 1] }
  }
  close $D
  foreach O [array names bokXopt {x*}] { option add *[string range $O 1 end] $bokXopt($O) }
}

proc bokShowParameters { } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  foreach E [array names bokParams] { bokInform "$E = $bokParams($E)" }
}

proc bokShowVariables { } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  foreach E [array names bokVariables] { bokInform "$E = $bokVariables($E)" }
}

proc bokShowWidgets { } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  foreach E [array names bokWidgets] { bokInform "$E = $bokWidgets($E)" }
}

proc bokShowXopt { } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  foreach E [array names bokXopt] { bokInform "$E = $bokXopt($E)" }
}

proc bokStringMatch { P S } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  if { $bokWidgets(bok_text_widget) != "" } {
    scan [$bokWidgets(bok_text_widget) index end] %d n
    for {set i 1} {$i < $n} {incr i} {
      $bokWidgets(bok_text_widget) delete 0.0 1.0
      $bokWidgets(bok_text_widget) mark set last $i.0
      while {[regexp -indices $P [$bokWidgets(bok_text_widget) get last "last lineend"] x]} {
        $bokWidgets(bok_text_widget) mark set first "last + [lindex $x 0] chars"
        $bokWidgets(bok_text_widget) mark set last "last + 1 chars + [lindex $x 1] chars"
        uplevel $S
      }
    }
  }
}

proc bokTerminate { N } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  set _s [catch {exec kill -9 ${N}} _r]
  if {${_s} == 0} {return ${_s}}
  return "${_s}:${_r}" 
}

proc bokTraceVariables { name element op } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  if { ${element} != "" } { set name ${name}(${element}) }
  upvar ${name} x
  set _elem [string trim ${element}]
  if { [info exists bokWidgets(${_elem})] } {
    if { $x == 0 } {
      $bokWidgets(${_elem}) configure -bg #B40000 -fg #0000B4
    } else {
      $bokWidgets(${_elem}) configure -bg #00B400 -fg #0000B4
    }
  }
}

proc bokUpdateVariables { } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
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
