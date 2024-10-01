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
dry_run=0
[[ -z $(command -v indiserver) ]] && _indiserver=0 || _indiserver=1


source ${BOK_GALIL_BIN}/functions.sh
bok_90prime_gui="${HOME}/bok-90prime-gui"
[[ -f "${bok_90prime_gui}/.env" ]] && source ${bok_90prime_gui}/.env || write_red "<ERROR> No web environment found!"

dataserver="${HOME}/dataserver"
export PATH=${dataserver}:${PATH}
[[ -z $(command -v dataserver) ]] && _dataserver=0 || _dataserver=1


# +
# utility functions
# -
usage () {
  write_blue   ""                                                                                  2>&1
  write_blue   "Galil_DMC_22x0 Control"                                                            2>&1
  write_blue   ""                                                                                  2>&1
  write_green  "Use:"                                                                              2>&1
  write_green  " %% bash $0 --command=<str> [--dry-run]"                                           2>&1
  write_magenta ""                                                                                 2>&1
  write_magenta "Input(s):"                                                                        2>&1
  write_magenta "  --command=<str>,  choice of 'status', 'start', 'stop',  default=${def_command}" 2>&1
  write_magenta ""                                                                                 2>&1
  write_cyan    "Flag(s):"                                                                         2>&1
  write_cyan    "  --dry-run,        show (but do not execute) commands,   default=false"          2>&1
  echo          ""                                                                                 2>&1
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
_prc0="Galil_DMC_22x0_Write_Memory"
_nam0="Galil_DMC_22x0_Write_Memory         "
_pid0=$(ps -ef | grep ${_prc0} | grep -v grep | awk '{print $2}')

_prc2="/dev/shm/tcp_shm"
_nam2="TCP Shared Memory File              "
_pid2=""

_prc3="/dev/shm/udp_shm"
_nam3="UDP Shared Memory File              "
_pid3=""

_prc4="bokGalilIndiDriver"
_nam4="bokGalilIndiDriver                  "
_pid4=$(ps -ef | grep ${_prc4} | grep -v indiserver | grep -v grep | awk '{print $2}')

_prc5="Galil_DMC_22x0_NgServer"
_nam5="Galil_DMC_22x0_NgServer             "
_pid5=$(ps -ef | grep ${_prc5} | grep -v grep | awk '{print $2}')

_prc6="bok.py"
_nam6="PyINDI http://${WEBHOST}:${WEBPORT}/      "
_pid6=$(ps -ef | grep ${_prc6} | grep -v grep | awk '{print $2}')

_prc7="dataserver"
_nam7="xterm -e dataserver                  "
_pid7=$(ps -ef | grep ${_prc7} | grep -v xterm | grep -v grep | awk '{print $2}')


# +
# execute
# -
case $(echo ${_command}) in

  # --command=start
  start)
    if [[ ${dry_run} -eq 1 ]]; then
      write_magenta "Dry-Run> nohup ${BOK_GALIL_BIN}/${_prc0} >> ${BOK_GALIL_LOG}/${_prc0}.log 2>&1 &"
      write_magenta "Dry-Run> nohup indiserver -vvv ${BOK_GALIL_BIN}/${_prc4} >> ${BOK_GALIL_LOG}/${_prc4}.log 2>&1 &"
      write_magenta "Dry-Run> nohup ${BOK_GALIL_BIN}/${_prc5} >> ${BOK_GALIL_LOG}/${_prc5}.log 2>&1 &"
      write_magenta "Dry-Run> nohup python3 ${bok_90prime_gui}/src/${_prc6} >> ${BOK_GALIL_LOG}/${_prc6}.log 2>&1 &"
      write_magenta "Dry-Run> xterm -e ${dataserver}/${_prc7} &"
    else
      [[ -z ${_pid0} ]] && write_green "Starting ${_nam0}" && (nohup ${BOK_GALIL_BIN}/${_prc0} >> ${BOK_GALIL_LOG}/${_prc0}.log 2>&1 &) && write_ok "${_nam0}" "STARTED OK" && sleep 1 || write_error "${_nam0}" "ALREADY RUNNING"
      if [[ ${_indiserver} -eq 1 ]]; then
        [[ -z ${_pid4} ]] && write_green "Starting ${_nam4}" && (nohup indiserver -vv ${BOK_GALIL_BIN}/${_prc4} >> ${BOK_GALIL_LOG}/${_prc4}.log 2>&1 &) && write_ok "${_nam4}" "STARTED OK" && sleep 1 || write_error "${_nam4}" "ALREADY RUNNING"
      fi
      [[ -z ${_pid5} ]] && write_green "Starting ${_nam5}" && (nohup ${BOK_GALIL_BIN}/${_prc5} >> ${BOK_GALIL_LOG}/${_prc5}.log 2>&1 &) && write_ok "${_nam5}" "STARTED OK" && sleep 1 || write_error "${_nam5}" "ALREADY RUNNING"
      [[ -z ${_pid6} ]] && write_green "Starting ${_nam6}" && (nohup python3 ${bok_90prime_gui}/src/${_prc6} >> ${BOK_GALIL_LOG}/${_prc6}.log 2>&1 &) && write_ok "${_nam6}" "STARTED OK" && sleep 1 || write_error "${_nam6}" "ALREADY RUNNING"
      if [[ ${_dataserver} -eq 1 ]]; then
        [[ -z ${_pid7} ]] && write_green "Starting ${_nam7}" && (xterm -e dataserver &) && write_ok "${_nam7}" "STARTED OK" && sleep 1 || write_error "${_nam7}" "ALREADY RUNNING"
      fi
    fi
    ;;

  # --command=stop
  stop)
    if [[ ${dry_run} -eq 1 ]]; then
      write_magenta "Dry-Run> kill -SIGINT pidof(${_prc0})"
      write_magenta "Dry-Run> kill -9 pidof(${_prc4})"
      write_magenta "Dry-Run> kill -9 pidof(${_prc5})"
      write_magenta "Dry-Run> kill -9 pidof(${_prc6})"
      write_magenta "Dry-Run> kill -9 pidof(${_prc7})"
      write_magenta "Dry-Run> rm -f ${_prc2}"
      write_magenta "Dry-Run> rm -f ${_prc3}"
    else
      [[ ! -z ${_pid0} ]] && echo "kill -SIGINT ${_pid0}" && kill -SIGINT ${_pid0} && write_ok "${_nam0}" "KILLED OK" || write_error "${_nam0}" "NOT RUNNING"
      # [[ ! -z ${_pid1} ]] && echo "kill -SIGINT ${_pid1}" && kill -SIGINT ${_pid1} && write_ok "${_nam1}" "KILLED OK" || write_error "${_nam1}" "NOT RUNNING"
      [[ ! -z ${_pid4} ]] && echo "kill -9 ${_pid4}" && kill -9 ${_pid4} &&  write_ok "${_nam4}" "KILLED OK" || write_error "${_nam4}" "NOT RUNNING"
      [[ ! -z ${_pid5} ]] && echo "kill -9 ${_pid5}" && kill -9 ${_pid5} && write_ok "${_nam5}" "KILLED OK" || write_error "${_nam5}" "NOT RUNNING"
      [[ ! -z ${_pid6} ]] && echo "kill -9 ${_pid6}" && kill -9 ${_pid6} && write_ok "${_nam6}" "KILLED OK" || write_error "${_nam6}" "NOT RUNNING"
      [[ ! -z ${_pid7} ]] && echo "kill -9 ${_pid7}" && kill -9 ${_pid7} && write_ok "${_nam7}" "KILLED OK" || write_error "${_nam7}" "NOT RUNNING"
      [[ -f ${_prc2} ]] && echo "rm -rf ${_prc2}" && rm -f ${_prc2} || write_error "${_nam2}" "NOT FOUND"
      [[ -f ${_prc3} ]] && echo "rm -rf ${_prc3}" && rm -f ${_prc3} || write_error "${_nam3}" "NOT FOUND"
    fi
   ;;

  # --command=status - no need for a --dry-run option as it's all passive
  status|*)
    [[ -f ${_prc2} ]] && write_ok "${_nam2}" "EXISTS" || write_error "${_nam2}" "NOT FOUND"
    [[ -f ${_prc3} ]] && write_ok "${_nam3}" "EXISTS" || write_error "${_nam3}" "NOT FOUND"
    [[ ! -z ${_pid0} ]] && write_ok "${_nam0}" "OK (${_pid0})" || write_error "${_nam0}" "NOT RUNNING"
    [[ ! -z ${_pid4} ]] && write_ok "${_nam4}" "OK (${_pid4})" || write_error "${_nam4}" "NOT RUNNING"
    [[ ! -z ${_pid5} ]] && write_ok "${_nam5}" "OK (${_pid5})" || write_error "${_nam5}" "NOT RUNNING"
    [[ ! -z ${_pid6} ]] && write_ok "${_nam6}" "OK (${_pid6})" || write_error "${_nam6}" "NOT RUNNING"
    [[ ! -z ${_pid7} ]] && write_ok "${_nam7}" "OK (${_pid7})" || write_error "${_nam7}" "NOT RUNNING"
    ;;
esac


# +
# exit
# -
exit 0
