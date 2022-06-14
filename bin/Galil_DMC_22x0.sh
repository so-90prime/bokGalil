#!/bin/bash


# +
#
# Name:        Galil_DMC_22x0.sh
# Description: Galil_DMC_22x0 control
# Author:      Phil Daly (pndaly@arizona.edu)
# Date:        20220613
# Execute:     % bash Galil_DMC_22x0.sh --help
#
# -


# +
# set defaults: edit as you see fit
# -
def_command="status"
_home=$(env | grep '^HOME' | cut -d'=' -f2)
bok_90prime_gui="${_home}/PycharmProjects/bok-90prime-gui"
dry_run=0
[[ -z $(command -v indiserver) ]] && _indiserver=0 || _indiserver=1


# +
# utility functions
# -
source ${BOK_GALIL_BIN}/functions.sh
usage () {
  write_blue   ""                                                                                   2>&1
  write_blue   "Galil_DMC_22x0 Control"                                                             2>&1
  write_blue   ""                                                                                   2>&1
  write_green  "Use:"                                                                               2>&1
  write_green  " %% bash $0 --command=<str> [--dry-run]"                                            2>&1
  write_magenta ""                                                                                  2>&1
  write_magenta "Input(s):"                                                                         2>&1
  write_magenta "  --command=<str>,  choice of 'status', 'start', 'stop',   default=${def_command}" 2>&1
  write_magenta ""                                                                                  2>&1
  write_cyan    "Flag(s):"                                                                          2>&1
  write_cyan    "  --dry-run,        show (but do not execute) commands,    default=false"          2>&1
  echo          ""                                                                                  2>&1
}


# +
# check command line argument(s)
# -
while test $# -gt 0; do
  case "${1}" in
    --command*|--COMMAND*)
      _command=$(echo $1 | cut -d'=' -f2)
      [[ -z ${_command} ]] && _command=${def_command}
      shift
      ;;
    --dry-run|--DRY-RUN)
      dry_run=1
      shift
      ;;
    --help|*)
      usage
      exit 0
      ;;
  esac
done


# +
# variable(s)
# -
_prc0="Galil_DMC_22x0_TCP_Write"
_prc3="Galil_DMC_22x0_UDP_Write"
_prc6="bokGalilIndiDriver"
_prc8="/dev/shm/tcp_shm"
_prc9="/dev/shm/udp_shm"
_prca="Galil_DMC_22x0_NgServer"
_prcb="bok.py"

_nam0="Galil_DMC_22x0_TCP_Write            "
_nam3="Galil_DMC_22x0_UDP_Write            "
_nam6="bokGalilIndiDriver                  "
_nam8="TCP Shared Memory File              "
_nam9="UDP Shared Memory File              "
_nama="Galil_DMC_22x0_NgServer             "
_namb="PyINDI Bok Web-site (bok.py)        "

_pid0=$(ps -ef | grep ${_prc0} | grep -v grep | awk '{print $2}')
_pid3=$(ps -ef | grep ${_prc3} | grep -v grep | awk '{print $2}')
_pid6=$(ps -ef | grep ${_prc6} | grep -v indiserver | grep -v grep | awk '{print $2}')
_pid8=''
_pid9=''
_pida=$(ps -ef | grep ${_prca} | grep -v grep | awk '{print $2}')
_pidb=$(ps -ef | grep ${_prcb} | grep -v grep | awk '{print $2}')


# +
# execute
# -
case $(echo ${_command}) in

  # --command=start
  start)
    if [[ ${dry_run} -eq 1 ]]; then
      write_magenta "Dry-Run> nohup ${BOK_GALIL_BIN}/${_prc0} >> ${BOK_GALIL_LOG}/${_prc0}.log 2>&1 &"
      write_magenta "Dry-Run> nohup ${BOK_GALIL_BIN}/${_prc3} >> ${BOK_GALIL_LOG}/${_prc3}.log 2>&1 &"
      write_magenta "Dry-Run> nohup indiserver -vv ${BOK_GALIL_BIN}/${_prc6} >> ${BOK_GALIL_LOG}/${_prc6}.log 2>&1 &"
      write_magenta "Dry-Run> nohup ${BOK_GALIL_BIN}/${_prca} >> ${BOK_GALIL_LOG}/${_prca}.log 2>&1 &"
      write_magenta "Dry-Run> conda activate pyindi; cd ${bok_90prime_gui}/src; python3 ${bok_90prime_gui}/src/bok.py"
    else
      [[ -z ${_pid0} ]] && write_green "Starting ${_nam0}" && (nohup ${BOK_GALIL_BIN}/${_prc0} >> ${BOK_GALIL_LOG}/${_prc0}.log 2>&1 &) && write_ok "${_nam0}" "STARTED OK" && sleep 1 || write_error "${_nam0}" "ALREADY RUNNING"
      [[ -z ${_pid3} ]] && write_green "Starting ${_nam3}" && (nohup ${BOK_GALIL_BIN}/${_prc3} >> ${BOK_GALIL_LOG}/${_prc3}.log 2>&1 &) && write_ok "${_nam3}" "STARTED OK" && sleep 1 || write_error "${_nam3}" "ALREADY RUNNING"
      if [[ ${_indiserver} -eq 1 ]]; then
        [[ -z ${_pid6} ]] && write_green "Starting ${_nam6}" && (nohup indiserver -vv ${BOK_GALIL_BIN}/${_prc6} >> ${BOK_GALIL_LOG}/${_prc6}.log 2>&1 &) && write_ok "${_nam6}" "STARTED OK" && sleep 1 || write_error "${_nam6}" "ALREADY RUNNING"
      fi
      [[ -z ${_pida} ]] && write_green "Starting ${_nama}" && (nohup ${BOK_GALIL_BIN}/${_prca} >> ${BOK_GALIL_LOG}/${_prca}.log 2>&1 &) && write_ok "${_nama}" "STARTED OK" && sleep 1 || write_error "${_nama}" "ALREADY RUNNING"
      [[ -z ${_pidb} ]] && write_green "Starting ${_namb}" && conda activate pyindi && cd ${bok_90prime_gui}/src && python3 ${bok_90prime_gui}/src/bok.py && write_ok "${_namb}" "STARTED OK" && sleep 1 || write_error "${_namb}" "ALREADY RUNNING"
    fi
    ;;

  # --command=stop
  stop)
    if [[ ${dry_run} -eq 1 ]]; then
      write_magenta "Dry-Run> kill -SIGINT pidof(${_prc0})"
      write_magenta "Dry-Run> kill -SIGINT pidof(${_prc3})"
      write_magenta "Dry-Run> kill -9 pidof(${_prc6})"
      write_magenta "Dry-Run> rm -f ${_prc8}"
      write_magenta "Dry-Run> rm -f ${_prc9}"
      write_magenta "Dry-Run> kill -9 pidof(${_prca})"
      write_magenta "Dry-Run> kill -9 pidof(${_prcb})"
    else
      [[ ! -z ${_pid0} ]] && echo "kill -SIGINT ${_pid0}" && kill -SIGINT ${_pid0} && write_ok "${_nam0}" "KILLED OK" || write_error "${_nam0}" "NOT RUNNING"
      [[ ! -z ${_pid3} ]] && echo "kill -SIGINT ${_pid3}" && kill -SIGINT ${_pid3} && write_ok "${_nam3}" "KILLED OK" || write_error "${_nam3}" "NOT RUNNING"
      [[ ! -z ${_pid6} ]] && echo "kill -9 ${_pid6}" && kill -9 ${_pid6} &&  write_ok "${_nam6}" "KILLED OK" || write_error "${_nam6}" "NOT RUNNING"
      [[ -f ${_prc8} ]] && echo "rm -rf ${_prc8}" && rm -f ${_prc8} || write_error "${_nam8}" "NOT FOUND"
      [[ -f ${_prc9} ]] && echo "rm -rf ${_prc9}" && rm -f ${_prc9} || write_error "${_nam9}" "NOT FOUND"
      [[ ! -z ${_pida} ]] && echo "kill -9 ${_pida}" && kill -9 ${_pida} && write_ok "${_nama}" "KILLED OK" || write_error "${_nama}" "NOT RUNNING"
      [[ ! -z ${_pidb} ]] && echo "kill -9 ${_pidb}" && kill -9 ${_pidb} && write_ok "${_nama}" "KILLED OK" || write_error "${_namb}" "NOT RUNNING"
    fi
   ;;

  # --command=status - no need for a --dry-run option as it's all passive
  status|*)
    [[ ! -z ${_pid0} ]] && write_ok "${_nam0}" "OK (${_pid0})" || write_error "${_nam0}" "NOT RUNNING"
    [[ ! -z ${_pid3} ]] && write_ok "${_nam3}" "OK (${_pid3})" || write_error "${_nam3}" "NOT RUNNING"
    [[ ! -z ${_pid6} ]] && write_ok "${_nam6}" "OK (${_pid6})" || write_error "${_nam6}" "NOT RUNNING"
    [[ -f ${_prc8} ]] && write_ok "${_nam8}" "EXISTS" || write_error "${_nam8}" "NOT FOUND"
    [[ -f ${_prc9} ]] && write_ok "${_nam9}" "EXISTS" || write_error "${_nam9}" "NOT FOUND"
    [[ ! -z ${_pida} ]] && write_ok "${_nama}" "OK (${_pida})" || write_error "${_nama}" "NOT RUNNING"
    [[ ! -z ${_pidb} ]] && write_ok "${_namb}" "OK (http://10.30.1.2:5905)" || write_error "${_namb}" "NOT RUNNING"
    ;;
esac


# +
# exit
# -
exit 0
