#!/bin/bash


# +
#
# Name:        Galil_DMC_22x0.sh
# Description: Galil_DMC_22x0 control
# Author:      Phil Daly (pndaly@arizona.edu)
# Date:        20220131
# Execute:     % bash Galil_DMC_22x0.sh --help
#
# -


# +
# set defaults: edit as you see fit
# -
def_command="status"

# c_read=0
dry_run=0
# py_read=0
web_site=0


# +
# utility functions
# -
source ${BOK_GALIL_BIN}/functions.sh
usage () {
  write_blue   ""                                                                                                2>&1
  write_blue   "Galil_DMC_22x0 Control"                                                                          2>&1
  write_blue   ""                                                                                                2>&1
  write_green  "Use:"                                                                                            2>&1
  write_green  " %% bash $0 --command=<str> [--dry-run]"                                                         2>&1
  write_magenta ""                                                                                               2>&1
  write_magenta "Input(s):"                                                                                      2>&1
  write_magenta "  --command=<str>,  choice of 'status', 'start', 'stop',              default=${def_command}"   2>&1
  write_magenta ""                                                                                               2>&1
  write_cyan    "Flag(s):"                                                                                       2>&1
  write_cyan    "  --dry-run,        show (but do not execute) commands,               default=false"            2>&1
  # write_cyan    "  --c-read,         start shared memory read in C (debugging),        default=false"            2>&1
  # write_cyan    "  --py-read,        start shared memory read in Python (debugging),   default=false"            2>&1
  write_cyan    "  --web-site,       start web-site (debugging),                       default=false"            2>&1
  echo          ""                                                                                               2>&1
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
    # --c-read|--C-READ)
    #   c_read=1
    #   shift
    #   ;;
    --dry-run|--DRY-RUN)
      dry_run=1
      shift
      ;;
    # --py-read|--PY-READ)
    #   py_read=1
    #   shift
    #   ;;
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
# _prc1="Galil_DMC_22x0_TCP_Read"
# _prc2="Galil_DMC_22x0_TCP_Read.py"
_prc3="Galil_DMC_22x0_UDP_Write"
# _prc4="Galil_DMC_22x0_UDP_Read"
# _prc5="Galil_DMC_22x0_UDP_Read.py"
_prc6="bokGalilIndiDriver"
_prc7="indiserver"
_prc8="/dev/shm/tcp_shm"
_prc9="/dev/shm/udp_shm"
_prca="pyindi_bok.py"

_nam0="Galil_DMC_22x0_TCP_Write           "
# _nam1="Galil_DMC_22x0_TCP_Read            "
# _nam2="Galil_DMC_22x0_TCP_Read.py         "
_nam3="Galil_DMC_22x0_UDP_Write           "
# _nam4="Galil_DMC_22x0_UDP_Read            "
# _nam5="Galil_DMC_22x0_UDP_Read.py         "
_nam6="bokGalilIndiDriver                 "
_nam7="indiserver                         "
_nam8="TCP Shared Memory File             "
_nam9="UDP Shared Memory File             "
_nama="PyINDI Bok Web-site (pyindi_bok.py)"

_pid0=$(ps -ef | grep ${_prc0} | grep -v grep | awk '{print $2}')
# _pid1=$(ps -ef | grep ${_prc1} | grep -v py | grep -v grep | awk '{print $2}')
# _pid2=$(ps -ef | grep ${_prc2} | grep -v grep | awk '{print $2}')
_pid3=$(ps -ef | grep ${_prc3} | grep -v grep | awk '{print $2}')
# _pid4=$(ps -ef | grep ${_prc4} | grep -v py | grep -v grep | awk '{print $2}')
# _pid5=$(ps -ef | grep ${_prc5} | grep -v grep | awk '{print $2}')
_pid6=$(ps -ef | grep ${_prc6} | grep -v ${_prc7} | grep -v grep | awk '{print $2}')
_pid7=$(ps -ef | grep ${_prc7} | grep -v grep | awk '{print $2}')
_pid8=''
_pid9=''
_pida=$(ps -ef | grep ${_prca} | grep -v grep | awk '{print $2}')

_pyindi_bok=$(service mtnops.pyindi status | grep 'Active:' | cut -d':' -f2 | cut -d'(' -f2 | cut -d')' -f1)


# +
# execute
# -
case $(echo ${_command}) in

  # --command=start
  start)
    if [[ ${dry_run} -eq 1 ]]; then
      write_magenta "Dry-Run> nohup ${BOK_GALIL_BIN}/${_prc0} >> ${BOK_GALIL_LOG}/${_prc0}.log 2>&1 &"
      write_magenta "Dry-Run> sleep 1"
      # [[ ${c_read} -eq 1 ]] && write_magenta "Dry-Run> nohup ${BOK_GALIL_BIN}/${_prc1} >> ${BOK_GALIL_LOG}/${_prc1}.log 2>&1 &"
      # [[ ${c_read} -eq 1 ]] && write_magenta "Dry-Run> sleep 1"
      # [[ ${py_read} -eq 1 ]] && write_magenta "Dry-Run> nohup python3 ${BOK_GALIL_BIN}/${_prc2} --loop >> ${BOK_GALIL_LOG}/${_prc2}.log 2>&1 &"
      # [[ ${py_read} -eq 1 ]] && write_magenta "Dry-Run> sleep 1"
      write_magenta "Dry-Run> nohup ${BOK_GALIL_BIN}/${_prc3} >> ${BOK_GALIL_LOG}/${_prc3}.log 2>&1 &"
      write_magenta "Dry-Run> sleep 1"
      # [[ ${c_read} -eq 1 ]] && write_magenta "Dry-Run> nohup ${BOK_GALIL_BIN}/${_prc4} >> ${BOK_GALIL_LOG}/${_prc4}.log 2>&1 &"
      # [[ ${c_read} -eq 1 ]] && write_magenta "Dry-Run> sleep 1"
      # [[ ${py_read} -eq 1 ]] && write_magenta "Dry-Run> nohup python3 ${BOK_GALIL_BIN}/${_prc5} --loop >> ${BOK_GALIL_LOG}/${_prc5}.log 2>&1 &"
      # [[ ${py_read} -eq 1 ]] && write_magenta "Dry-Run> sleep 1"
      write_magenta "Dry-Run> nohup ${_prc7} -vvv ${BOK_GALIL_BIN}/${_prc6} >> ${BOK_GALIL_LOG}/${_prc6}.log 2>&1 &"
      if [[ ${web_site} -eq 1 ]]; then
        write_magenta "Dry-Run> service mtnops.pyindi start"
      fi
    else
      write_green "Executing> nohup ${BOK_GALIL_BIN}/${_prc0} >> ${BOK_GALIL_LOG}/${_prc0}.log 2>&1 &"
      [[ -z ${_pid0} ]] && (nohup ${BOK_GALIL_BIN}/${_prc0} >> ${BOK_GALIL_LOG}/${_prc0}.log 2>&1 &) || write_error "${_nam0}" "ALREADY RUNNING"
      write_green "Executing> sleep 1"
      sleep 1
      # if [[ ${c_read} -eq 1 ]]; then
      #   write_green "Executing> nohup ${BOK_GALIL_BIN}/${_prc1} >> ${BOK_GALIL_LOG}/${_prc1}.log 2>&1 &"
      #   [[ -z ${_pid1} ]] && (nohup ${BOK_GALIL_BIN}/${_prc1} >> ${BOK_GALIL_LOG}/${_prc1}.log 2>&1 &) || write_error "${_nam1}" "ALREADY RUNNING"
      #   write_green "Executing> sleep 1"
      #   sleep 1
      # fi
      # if [[ ${py_read} -eq 1 ]]; then
      #   write_green "Executing> nohup python3 ${BOK_GALIL_BIN}/${_prc2} --loop >> ${BOK_GALIL_LOG}/${_prc2}.log 2>&1 &"
      #   [[ -z ${_pid2} ]] && (nohup python3 ${BOK_GALIL_BIN}/${_prc2} --loop >> ${BOK_GALIL_LOG}/${_prc2}.log 2>&1 &) || write_error "${_nam2}" "ALREADY RUNNING"
      #   write_green "Executing> sleep 1"
      #   sleep 1
      # fi
      write_green "Executing> nohup ${BOK_GALIL_BIN}/${_prc3} >> ${BOK_GALIL_LOG}/${_prc3}.log 2>&1 &"
      [[ -z ${_pid3} ]] && (nohup ${BOK_GALIL_BIN}/${_prc3} >> ${BOK_GALIL_LOG}/${_prc3}.log 2>&1 &) || write_error "${_nam3}" "ALREADY RUNNING"
      write_green "Executing> sleep 1"
      sleep 1
      # if [[ ${c_read} -eq 1 ]]; then
      #   write_green "Executing> nohup ${BOK_GALIL_BIN}/${_prc4} >> ${BOK_GALIL_LOG}/${_prc4}.log 2>&1 &"
      #   [[ -z ${_pid4} ]] && (nohup ${BOK_GALIL_BIN}/${_prc4} >> ${BOK_GALIL_LOG}/${_prc4}.log 2>&1 &) || write_error "${_nam4}" "ALREADY RUNNING"
      #   write_green "Executing> sleep 1"
      #   sleep 1
      # fi
      # if [[ ${py_read} -eq 1 ]]; then
      #   write_green "Executing> nohup python3 ${BOK_GALIL_BIN}/${_prc5} --loop >> ${BOK_GALIL_LOG}/${_prc5}.log 2>&1 &"
      #   [[ -z ${_pid5} ]] && (nohup python3 ${BOK_GALIL_BIN}/${_prc5} --loop >> ${BOK_GALIL_LOG}/${_prc5}.log 2>&1 &) || write_error "${_nam5}" "ALREADY RUNNING"
      #   write_green "Executing> sleep 1"
      #   sleep 1
      # fi
      write_green "Executing> nohup ${_prc7} -vvv ${BOK_GALIL_BIN}/${_prc6} >> ${BOK_GALIL_LOG}/${_prc6}.log 2>&1 &"
      [[ -z ${_pid6} ]] && (nohup indiserver -vvv ${BOK_GALIL_BIN}/${_prc6} >> ${BOK_GALIL_LOG}/${_prc6}.log 2>&1 &) || write_error "${_nam6}" "ALREADY RUNNING"
      if [[ ${web_site} -eq 1 ]]; then
        write_green "Executing> service mtnops.pyindi start"
        service mtnops.pyindi start
      fi
    fi
    ;;

  # --command=stop
  stop)
    if [[ ${dry_run} -eq 1 ]]; then
      write_magenta "Dry-Run> kill -SIGINT pidof(${_prc0})"
      # write_magenta "Dry-Run> kill -SIGINT pidof(${_prc1})"
      # write_magenta "Dry-Run> kill -SIGINT pidof(${_prc2})"
      write_magenta "Dry-Run> kill -SIGINT pidof(${_prc3})"
      # write_magenta "Dry-Run> kill -SIGINT pidof(${_prc4})"
      # write_magenta "Dry-Run> kill -SIGINT pidof(${_prc5})"
      write_magenta "Dry-Run> kill -9 pidof(${_prc6})"
      write_magenta "Dry-Run> kill -9 pidof(${_prc7})"
      write_magenta "Dry-Run> rm -f ${_prc8}"
      write_magenta "Dry-Run> rm -f ${_prc9}"
      if [[ ${web_site} -eq 1 ]]; then
        write_magenta "Dry-Run> service mtnops.pyindi stop"
      fi
    else
      write_green "Executing> kill -SIGINT pidof(${_prc0})"
      [[ ! -z ${_pid0} ]] && kill -SIGINT ${_pid0} || write_error "${_nam0}" "NOT RUNNING"
      # write_green "Executing> kill -SIGINT pidof(${_prc1})"
      # [[ ! -z ${_pid1} ]] && kill -SIGINT ${_pid1} || write_error "${_nam1}" "NOT RUNNING"
      # write_green "Executing> kill -SIGINT pidof(${_prc2})"
      # [[ ! -z ${_pid2} ]] && kill -SIGINT ${_pid2} || write_error "${_nam2}" "NOT RUNNING"
      write_green "Executing> kill -SIGINT pidof(${_prc3})"
      [[ ! -z ${_pid3} ]] && kill -SIGINT ${_pid3} || write_error "${_nam3}" "NOT RUNNING"
      # write_green "Executing> kill -SIGINT pidof(${_prc4})"
      # [[ ! -z ${_pid4} ]] && kill -SIGINT ${_pid4} || write_error "${_nam4}" "NOT RUNNING"
      # write_green "Executing> kill -SIGINT pidof(${_prc5})"
      # [[ ! -z ${_pid5} ]] && kill -SIGINT ${_pid5} || write_error "${_nam5}" "NOT RUNNING"
      write_green "Executing> kill -9 pidof(${_prc6})"
      [[ ! -z ${_pid6} ]] && kill -9 ${_pid6}      || write_error "${_nam6}" "NOT RUNNING"
      write_green "Executing> kill -9 pidof(${_prc7})"
      [[ ! -z ${_pid7} ]] && kill -9 ${_pid7}      || write_error "${_nam7}" "NOT RUNNING"
      write_green "Executing> rm -f ${_prc8}"
      [[ -f ${_prc8} ]]   && rm -f ${_prc8}        || write_error "${_nam8}" "NOT FOUND"
      write_green "Executing> rm -f ${_prc9}"
      [[ -f ${_prc9} ]]   && rm -f ${_prc9}        || write_error "${_nam9}" "NOT FOUND"
      if [[ ${web_site} -eq 1 ]]; then
        write_green "Executing> service mtnops.pyindi stop"
        service mtnops.pyindi stop
      fi
    fi
   ;;

  # --command=status
  status|*)
    if [[ ${dry_run} -eq 1 ]]; then
      write_magenta "Dry-Run> [[ ! -z pidof(${_prc0}) ]] && write_ok '${_prc0}' 'OK (pidof(${_prc0}))' || write_error '${_prc0}' 'NOT RUNNING'"
      # write_magenta "Dry-Run> [[ ! -z pidof(${_prc1}) ]] && write_ok '${_prc1}' 'OK (pidof(${_prc1}))' || write_error '${_prc1}' 'NOT RUNNING'"
      # write_magenta "Dry-Run> [[ ! -z pidof(${_prc2}) ]] && write_ok '${_prc2}' 'OK (pidof(${_prc2}))' || write_error '${_prc2}' 'NOT RUNNING'"
      write_magenta "Dry-Run> [[ ! -z pidof(${_prc3}) ]] && write_ok '${_prc3}' 'OK (pidof(${_prc3}))' || write_error '${_prc3}' 'NOT RUNNING'"
      # write_magenta "Dry-Run> [[ ! -z pidof(${_prc4}) ]] && write_ok '${_prc4}' 'OK (pidof(${_prc4}))' || write_error '${_prc4}' 'NOT RUNNING'"
      # write_magenta "Dry-Run> [[ ! -z pidof(${_prc5}) ]] && write_ok '${_prc5}' 'OK (pidof(${_prc5}))' || write_error '${_prc5}' 'NOT RUNNING'"
      write_magenta "Dry-Run> [[ ! -z pidof(${_prc6}) ]] && write_ok '${_prc6}' 'OK (pidof(${_prc6}))' || write_error '${_prc6}' 'NOT RUNNING'"
      write_magenta "Dry-Run> [[ ! -z pidof(${_prc7}) ]] && write_ok '${_prc7}' 'OK (pidof(${_prc7}))' || write_error '${_prc7}' 'NOT RUNNING'"
      write_magenta "Dry-Run> [[ -f ${_prc8} ]]  && write_ok '${_prc8}' 'EXISTS' || write_error '${_prc8}' 'NOT FOUND'"
      write_magenta "Dry-Run> [[ -f ${_prc9} ]]  && write_ok '${_prc9}' 'EXISTS' || write_error '${_prc9}' 'NOT FOUND'"
      write_magenta "Dry-Run> [[ ! -z pidof(${_prca}) ]] && write_ok '${_prca}' 'OK (pidof(${_prca}))' || write_error '${_prca}' 'NOT RUNNING'"
    else
      [[ ! -z ${_pid0} ]] && write_ok "${_nam0}" "OK (${_pid0})" || write_error "${_nam0}" "NOT RUNNING"
      # [[ ! -z ${_pid1} ]] && write_ok "${_nam1}" "OK (${_pid1})" || write_error "${_nam1}" "NOT RUNNING"
      # [[ ! -z ${_pid2} ]] && write_ok "${_nam2}" "OK (${_pid2})" || write_error "${_nam2}" "NOT RUNNING"
      [[ ! -z ${_pid3} ]] && write_ok "${_nam3}" "OK (${_pid3})" || write_error "${_nam3}" "NOT RUNNING"
      # [[ ! -z ${_pid4} ]] && write_ok "${_nam4}" "OK (${_pid4})" || write_error "${_nam4}" "NOT RUNNING"
      # [[ ! -z ${_pid5} ]] && write_ok "${_nam5}" "OK (${_pid5})" || write_error "${_nam5}" "NOT RUNNING"
      [[ ! -z ${_pid6} ]] && write_ok "${_nam6}" "OK (${_pid6})" || write_error "${_nam6}" "NOT RUNNING"
      [[ ! -z ${_pid7} ]] && write_ok "${_nam7}" "OK (${_pid7})" || write_error "${_nam7}" "NOT RUNNING"
      [[ -f ${_prc8} ]]   && write_ok "${_nam8}" "EXISTS"        || write_error "${_nam8}" "NOT FOUND"
      [[ -f ${_prc9} ]]   && write_ok "${_nam9}" "EXISTS"        || write_error "${_nam9}" "NOT FOUND"
      [[ ! -z ${_pida} ]] && write_ok "${_nama}" "OK (http://10.30.1.2:5905/ninety-prime)" || write_error "${_nama}" "NOT RUNNING"
    fi
    ;;
esac


# +
# exit
# -
exit 0
