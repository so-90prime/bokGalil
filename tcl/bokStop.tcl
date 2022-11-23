# +
# BOK_INSTRUMENT \ Instrument                 \ 90Prime
# BOK_INDI_ADDR  \ Indiserver Address         \ 10.130.133.206
# BOK_INDI_PORT  \ Indiserver Port            \ 7624
# BOK_NG_ADDR    \ NG Server Address          \ 10.130.133.206
# BOK_NG_PORT    \ NG Server Port             \ 5750
# BOK_TCP_ADDR   \ Galil TCP Command Address  \ 10.130.133.206
# BOK_TCP_PORT   \ Galil TCP Command Port     \ 23
# BOK_UDP_ADDR   \ Galil UDP Command Address  \ 10.130.133.206
# BOK_UDP_PORT   \ Galil UDP Command Port     \ 5078
# BOK_WEB_ADDR   \ pyINDI Website Address     \ 10.130.133.206
# BOK_WEB_PORT   \ pyINDI Website Port        \ 5905
# -
proc bokStop { } {
  global env bokParams bokVariables bokVersions bokWidgets bokXopt
  if {[info exists bokVariables(bok_indidriver)] && $bokVariables(bok_indidriver) > 0} {
    bokInform "bokStop <bok_indidriver>: after 500 bokTerminate $bokVariables(bok_indidriver)"
    after 500 bokTerminate $bokVariables(bok_indidriver)
  } else {
    bokInform "Indidriver is already stopped!"
  }
  if {[info exists bokVariables(bok_indiserver)] && $bokVariables(bok_indiserver) > 0} {
    bokInform "bokStop <bok_indiserver>: after 500 bokTerminate $bokVariables(bok_indiserver)"
    after 500 bokTerminate $bokVariables(bok_indiserver)
  }
  if {[info exists bokVariables(bok_ngserver)] && $bokVariables(bok_ngserver) > 0} {
    bokInform "bokStop <bok_ngserver>: after 500 bokTerminate $bokVariables(bok_ngserver)"
    after 500 bokTerminate $bokVariables(bok_ngserver)
  }
  if {[info exists bokVariables(bok_memory)] && $bokVariables(bok_memory) > 0} {
    bokInform "bokStop <bok_memory>: after 500 bokTerminate $bokVariables(bok_memory)"
    after 500 bokTerminate $bokVariables(bok_memory)
  }
  if {[info exists bokVariables(bok_website)] && $bokVariables(bok_website) > 0} {
    bokInform "bokStop <bok_webiste>: after 500 bokTerminate $bokVariables(bok_website)"
    after 500 bokTerminate $bokVariables(bok_website)
  }
  if {[info exists bokVariables(bok_ds9)] && $bokVariables(bok_ds9) > 0} {
    bokInform "bokStop <bok_dataserver>: after 500 bokTerminate $bokVariables(bok_ds9)"
    after 500 bokTerminate $bokVariables(bok_ds9)
  }
  if {[info exists bokVariables(bok_dataserver)] && $bokVariables(bok_dataserver) > 0} {
    bokInform "bokStop <bok_dataserver>: after 500 bokTerminate $bokVariables(bok_dataserver)"
    after 500 bokTerminate $bokVariables(bok_dataserver)
  }
  if {[info exists bokVariables(bok_tcpmem)] && $bokVariables(bok_tcpmem) > 0} {
    bokInform "bokStop <bok_tcpmem>: file delete -force /dev/shm/tcp_mem"
    file delete -force /dev/shm/tcp_shm
  }
  if {[info exists bokVariables(bok_udpmem)] && $bokVariables(bok_udpmem) > 0} {
    bokInform "bokStop <bok_udpmem>: file delete -force /dev/shm/udp_mem"
    file delete -force /dev/shm/udp_shm
  }
}
