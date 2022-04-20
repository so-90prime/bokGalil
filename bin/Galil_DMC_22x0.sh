#!/bin/bash


# +
#
# Name:        Galil_DMC_22x0.sh
# Description: Galil_DMC_22x0 control
# Author:      Phil Daly (pndaly@arizona.edu)
# Date:        20220411
# Execute:     % bash Galil_DMC_22x0.sh --help
#
# -


# +
# set defaults: edit as you see fit
# -
def_command="status"

c_read=0
dry_run=0
ng_server=0
py_read=0
web_site=0
[[ -z $(command -v service) ]] && _service=0 || _service=1
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
  write_cyan    "  --ng-server,      start ng server,                       default=false"          2>&1
  write_cyan    "  --web-site,       start web-site,                        default=false"          2>&1
  echo          ""                                                                                  2>&1
  write_yellow  "Debugging:"                                                                        2>&1
  write_yellow  "  --c-read,         start shared memory read in C,         default=false"          2>&1
  write_yellow  "  --py-read,        start shared memory read in Python,    default=false"          2>&1
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
    --c-read|--C-READ)
      c_read=1
      shift
      ;;
    --dry-run|--DRY-RUN)
      dry_run=1
      shift
      ;;
    --ng-server|--NG-SERVER)
      ng_server=1
      shift
      ;;
    --py-read|--PY-READ)
      py_read=1
      shift
      ;;
    --web-site|--WEB-SITE)
      web_site=1
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
_prc1="Galil_DMC_22x0_TCP_Read"
_prc2="Galil_DMC_22x0_TCP_Read.py"
_prc3="Galil_DMC_22x0_UDP_Write"
_prc4="Galil_DMC_22x0_UDP_Read"
_prc5="Galil_DMC_22x0_UDP_Read.py"
_prc6="bokGalilIndiDriver"
_prc7="indiserver"
_prc8="/dev/shm/tcp_shm"
_prc9="/dev/shm/udp_shm"
_prca="Galil_DMC_22x0_NgServer"
_prcb="bok.py"

_nam0="Galil_DMC_22x0_TCP_Write            "
_nam1="Galil_DMC_22x0_TCP_Read             "
_nam2="Galil_DMC_22x0_TCP_Read.py          "
_nam3="Galil_DMC_22x0_UDP_Write            "
_nam4="Galil_DMC_22x0_UDP_Read             "
_nam5="Galil_DMC_22x0_UDP_Read.py          "
_nam6="bokGalilIndiDriver                  "
_nam7="indiserver                          "
_nam8="TCP Shared Memory File              "
_nam9="UDP Shared Memory File              "
_nama="Galil_DMC_22x0_NgServer             "
_namb="PyINDI Bok Web-site (bok.py)        "

_pid0=$(ps -ef | grep ${_prc0} | grep -v grep | awk '{print $2}')
_pid1=$(ps -ef | grep ${_prc1} | grep -v py | grep -v grep | awk '{print $2}')
_pid2=$(ps -ef | grep ${_prc2} | grep -v grep | awk '{print $2}')
_pid3=$(ps -ef | grep ${_prc3} | grep -v grep | awk '{print $2}')
_pid4=$(ps -ef | grep ${_prc4} | grep -v py | grep -v grep | awk '{print $2}')
_pid5=$(ps -ef | grep ${_prc5} | grep -v grep | awk '{print $2}')
_pid6=$(ps -ef | grep ${_prc6} | grep -v ${_prc7} | grep -v grep | awk '{print $2}')
_pid7=$(ps -ef | grep ${_prc7} | grep ${_prc6} | grep -v grep | awk '{print $2}')
_pid8=''
_pid9=''
_pida=$(ps -ef | grep ${_prca} | grep -v grep | awk '{print $2}')
_pidb=$(ps -ef | grep ${_prcb} | grep -v grep | awk '{print $2}')

# [[ ${_service} -eq 1 ]] && _pyindi_bok=$(service mtnops.pyindi status | grep 'Active:' | cut -d':' -f2 | cut -d'(' -f2 | cut -d')' -f1) || _pyindi_bok='NO SERVICE'


# +
# execute
# -
case $(echo ${_command}) in

  # --command=start
  start)
    if [[ ${dry_run} -eq 1 ]]; then
      # standard
      write_magenta "Dry-Run> nohup ${BOK_GALIL_BIN}/${_prc0} >> ${BOK_GALIL_LOG}/${_prc0}.log 2>&1 &"
      write_magenta "Dry-Run> nohup ${BOK_GALIL_BIN}/${_prc3} >> ${BOK_GALIL_LOG}/${_prc3}.log 2>&1 &"
      write_magenta "Dry-Run> nohup ${_prc7} -vv ${BOK_GALIL_BIN}/${_prc6} >> ${BOK_GALIL_LOG}/${_prc6}.log 2>&1 &"
      # optional
      [[ ${c_read} -eq 1 ]] && write_magenta "Dry-Run> nohup ${BOK_GALIL_BIN}/${_prc1} >> ${BOK_GALIL_LOG}/${_prc1}.log 2>&1 &"
      [[ ${c_read} -eq 1 ]] && write_magenta "Dry-Run> nohup ${BOK_GALIL_BIN}/${_prc4} >> ${BOK_GALIL_LOG}/${_prc4}.log 2>&1 &"
      [[ ${py_read} -eq 1 ]] && write_magenta "Dry-Run> nohup python3 ${BOK_GALIL_BIN}/${_prc2} --loop >> ${BOK_GALIL_LOG}/${_prc2}.log 2>&1 &"
      [[ ${py_read} -eq 1 ]] && write_magenta "Dry-Run> nohup python3 ${BOK_GALIL_BIN}/${_prc5} --loop >> ${BOK_GALIL_LOG}/${_prc5}.log 2>&1 &"
      [[ ${ng_server} -eq 1 ]] && write_magenta "Dry-Run> nohup ${BOK_GALIL_BIN}/${_prca} >> ${BOK_GALIL_LOG}/${_prca}.log 2>&1 &"
      if [[ ${web_site} -eq 1 ]]; then
        if [[ ${_service} -eq 1 ]]; then
          write_magenta "Dry-Run> service mtnops.pyindi start"
       fi
      fi
    else
      # standard
      [[ -z ${_pid0} ]] && write_green "Starting ${_nam0}" && (nohup ${BOK_GALIL_BIN}/${_prc0} >> ${BOK_GALIL_LOG}/${_prc0}.log 2>&1 &) && write_ok "${_nam0}" "STARTED OK" && sleep 1 || write_error "${_nam0}" "ALREADY RUNNING"
      [[ -z ${_pid3} ]] && write_green "Starting ${_nam3}" && (nohup ${BOK_GALIL_BIN}/${_prc3} >> ${BOK_GALIL_LOG}/${_prc3}.log 2>&1 &) && write_ok "${_nam3}" "STARTED OK" && sleep 1 || write_error "${_nam3}" "ALREADY RUNNING"
      if [[ ${_indiserver} -eq 1 ]]; then
        [[ -z ${_pid6} ]] && [[ -z ${_pid7} ]] && write_green "Starting ${_nam6}" && (nohup ${_prc7} -vv ${BOK_GALIL_BIN}/${_prc6} >> ${BOK_GALIL_LOG}/${_prc6}.log 2>&1 &) && write_ok "${_nam6}" "STARTED OK" && sleep 1 || write_error "${_nam6}" "ALREADY RUNNING"
      fi
      # optional
      if [[ ${c_read} -eq 1 ]]; then
        [[ -z ${_pid1} ]] && write_green "Starting ${_nam1}" && (nohup ${BOK_GALIL_BIN}/${_prc1} >> ${BOK_GALIL_LOG}/${_prc1}.log 2>&1 &) && write_ok "${_nam1}" "STARTED OK" && sleep 1 || write_error "${_nam1}" "ALREADY RUNNING"
        [[ -z ${_pid4} ]] && write_green "Starting ${_nam4}" && (nohup ${BOK_GALIL_BIN}/${_prc4} >> ${BOK_GALIL_LOG}/${_prc4}.log 2>&1 &) && write_ok "${_nam4}" "STARTED OK" && sleep 1 || write_error "${_nam4}" "ALREADY RUNNING"
      fi
      if [[ ${py_read} -eq 1 ]]; then
        [[ -z ${_pid2} ]] && write_green "Starting ${_nam2}" && (nohup python3 ${BOK_GALIL_BIN}/${_prc2} --loop >> ${BOK_GALIL_LOG}/${_prc2}.log 2>&1 &) && write_ok "${_nam2}" "STARTED OK" && sleep 1 || write_error "${_nam2}" "ALREADY RUNNING"
        [[ -z ${_pid5} ]] && write_green "Starting ${_nam5}" && (nohup python3 ${BOK_GALIL_BIN}/${_prc5} --loop >> ${BOK_GALIL_LOG}/${_prc5}.log 2>&1 &) && write_ok "${_nam5}" "STARTED OK" && sleep 1 || write_error "${_nam5}" "ALREADY RUNNING"
      fi
      if [[ ${ng_server} -eq 1 ]]; then
        [[ -z ${_pida} ]] && write_green "Starting ${_nama}" && (nohup ${BOK_GALIL_BIN}/${_prca} >> ${BOK_GALIL_LOG}/${_prca}.log 2>&1 &) && write_ok "${_nama}" "STARTED OK" && sleep 1 || write_error "${_nama}" "ALREADY RUNNING"
      fi
      if [[ ${web_site} -eq 1 ]]; then
        if [[ ${_service} -eq 1 ]]; then
          [[ -z ${_pidb} ]] && write_green "Starting ${_namb}" && service mtnops.pyindi start && write_ok "${_namb}" "STARTED OK" && sleep 1 || write_error "${_namb}" "ALREADY RUNNING"
        fi
      fi
    fi
    ;;

  # --command=stop
  stop)
    if [[ ${dry_run} -eq 1 ]]; then
      # standard
      write_magenta "Dry-Run> kill -SIGINT pidof(${_prc0})"
      write_magenta "Dry-Run> kill -SIGINT pidof(${_prc3})"
      write_magenta "Dry-Run> kill -9 pidof(${_prc6})"
      # write_magenta "Dry-Run> kill -9 pidof(${_prc7})"
      write_magenta "Dry-Run> rm -f ${_prc8}"
      write_magenta "Dry-Run> rm -f ${_prc9}"
      # optional
      [[ ${c_read} -eq 1 ]] && write_magenta "Dry-Run> kill -SIGINT pidof(${_prc1})"
      [[ ${c_read} -eq 1 ]] && write_magenta "Dry-Run> kill -SIGINT pidof(${_prc4})"
      [[ ${py_read} -eq 1 ]] && write_magenta "Dry-Run> kill -SIGINT pidof(${_prc2})"
      [[ ${py_read} -eq 1 ]] && write_magenta "Dry-Run> kill -SIGINT pidof(${_prc5})"
      [[ ${ng_server} -eq 1 ]] && write_magenta "Dry-Run> kill -9 pidof(${_prca})"
      [[ ${web_site} -eq 1 ]] && [[ ${_service} -eq 1 ]] && write_magenta "Dry-Run> service mtnops.pyindi stop"
    else
      # standard
      [[ ! -z ${_pid0} ]] && echo "kill -SIGINT ${_pid0}" && kill -SIGINT ${_pid0} && write_ok "${_nam0}" "KILLED OK" || write_error "${_nam0}" "NOT RUNNING"
      [[ ! -z ${_pid3} ]] && echo "kill -SIGINT ${_pid3}" && kill -SIGINT ${_pid3} && write_ok "${_nam3}" "KILLED OK" || write_error "${_nam3}" "NOT RUNNING"
      [[ ! -z ${_pid6} ]] && echo "kill -9 ${_pid6}" && kill -9 ${_pid6} &&  write_ok "${_nam6}" "KILLED OK" || write_error "${_nam6}" "NOT RUNNING"
      # [[ ! -z ${_pid7} ]] && echo "kill -9 ${_pid7}" && kill -9 ${_pid7} && write_ok "${_nam7}" "KILLED OK" || write_error "${_nam7}" "NOT RUNNING"
      [[ -f ${_prc8} ]] && echo "rm -rf ${_prc8}" && rm -f ${_prc8} || write_error "${_nam8}" "NOT FOUND"
      [[ -f ${_prc9} ]] && echo "rm -rf ${_prc9}" && rm -f ${_prc9} || write_error "${_nam9}" "NOT FOUND"
      # optional
      if [[ ${c_read} -eq 1 ]]; then
        [[ ! -z ${_pid1} ]] && echo "kill -SIGINT ${_pid1}" && kill -SIGINT ${_pid1} &&  write_ok "${_nam1}" "KILLED OK" || write_error "${_nam1}" "NOT RUNNING"
        [[ ! -z ${_pid4} ]] && echo "kill -SIGINT ${_pid4}" && kill -SIGINT ${_pid4} && write_ok "${_nam4}" "KILLED OK" || write_error "${_nam4}" "NOT RUNNING"
      fi
      if [[ ${py_read} -eq 1 ]]; then
        [[ ! -z ${_pid2} ]] &&echo "kill -SIGINT ${_pid2}" && kill -SIGINT ${_pid2} && write_ok "${_nam2}" "KILLED OK" || write_error "${_nam2}" "NOT RUNNING"
        [[ ! -z ${_pid5} ]] && echo "kill -SIGINT ${_pid5}" && kill -SIGINT ${_pid5} && write_ok "${_nam5}" "KILLED OK" || write_error "${_nam5}" "NOT RUNNING"
      fi
      if [[ ${ng_server} -eq 1 ]]; then
        [[ ! -z ${_pida} ]] && echo "kill -9 ${_pida}" && kill -9 ${_pida} && write_ok "${_nama}" "KILLED OK" || write_error "${_nama}" "NOT RUNNING"
      fi
      if [[ ${web_site} -eq 1 ]]; then
        if [[ ${_service} -eq 1 ]]; then
          service mtnops.pyindi stop
        fi
      fi
    fi
   ;;

  # --command=status - no need for a --dry-run option as it's all passive
  status|*)
    # standard
    [[ ! -z ${_pid0} ]] && write_ok "${_nam0}" "OK (${_pid0})" || write_error "${_nam0}" "NOT RUNNING"
    [[ ! -z ${_pid3} ]] && write_ok "${_nam3}" "OK (${_pid3})" || write_error "${_nam3}" "NOT RUNNING"
    [[ ! -z ${_pid6} ]] && write_ok "${_nam6}" "OK (${_pid6})" || write_error "${_nam6}" "NOT RUNNING"
    [[ ! -z ${_pid7} ]] && write_ok "${_nam7}" "OK (${_pid7})" || write_error "${_nam7}" "NOT RUNNING"
    [[ -f ${_prc8} ]] && write_ok "${_nam8}" "EXISTS" || write_error "${_nam8}" "NOT FOUND"
    [[ -f ${_prc9} ]] && write_ok "${_nam9}" "EXISTS" || write_error "${_nam9}" "NOT FOUND"

    # optional
    if [[ ${c_read} -eq 1 ]]; then
      [[ ! -z ${_pid1} ]] && write_ok "${_nam1}" "OK (${_pid1})" || write_error "${_nam1}" "NOT RUNNING"
      [[ ! -z ${_pid4} ]] && write_ok "${_nam4}" "OK (${_pid4})" || write_error "${_nam4}" "NOT RUNNING"
    fi
    if [[ ${py_read} -eq 1 ]]; then
      [[ ! -z ${_pid2} ]] && write_ok "${_nam2}" "OK (${_pid2})" || write_error "${_nam2}" "NOT RUNNING"
      [[ ! -z ${_pid5} ]] && write_ok "${_nam5}" "OK (${_pid5})" || write_error "${_nam5}" "NOT RUNNING"
    fi
    if [[ ${ng_server} -eq 1 ]]; then
      [[ ! -z ${_pida} ]] && write_ok "${_nama}" "OK (${_pida})" || write_error "${_nama}" "NOT RUNNING"
    fi
    if [[ ${web_site} -eq 1 ]]; then
      if [[ ${_service} -eq 1 ]]; then
        [[ ! -z ${_pidb} ]] && write_ok "${_namb}" "OK (http://10.30.1.2:5905)" || write_error "${_namb}" "NOT RUNNING"
      fi
    fi
    ;;
esac


# +
# exit
# -
exit 0
