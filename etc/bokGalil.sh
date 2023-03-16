#!/bin/sh


# +
# % cd <path>; bash etc/bokGalil.sh `pwd` {load|gui}
# -


# +
# edit as required
# -
export BOK_GALIL_HOME=${1:-${PWD}}
export BOK_GALIL_BIN=${BOK_GALIL_HOME}/bin
export BOK_GALIL_DOCS=${BOK_GALIL_HOME}/docs
export BOK_GALIL_ETC=${BOK_GALIL_HOME}/etc
export BOK_GALIL_LIB=${BOK_GALIL_HOME}/lib
export BOK_GALIL_LOG=${BOK_GALIL_HOME}/log
export BOK_GALIL_SRC=${BOK_GALIL_HOME}/src
export BOK_GALIL_TCL=${BOK_GALIL_HOME}/tcl
export BOK_GALIL_TEX=${BOK_GALIL_HOME}/tex


# +
# PATH(s)
# -
_librarypath=$(env | grep LD_LIBRARY_PATH | cut -d'=' -f2)
if [[ -z "${_librarypath}" ]]; then
  export LD_LIBRARY_PATH=${BOK_GALIL_LIB}
else
  export LD_LIBRARY_PATH=${BOK_GALIL_LIB}:${LD_LIBRARY_PATH}
fi

_pythonpath=$(env | grep PYTHONPATH | cut -d'=' -f2)
if [[ -z "${_pythonpath}" ]]; then
  export PYTHONPATH=`pwd`
fi
export PYTHONPATH=${BOK_GALIL_HOME}:${BOK_GALIL_SRC}:${PYTHONPATH}


# +
# update ephemeris (any value will do but I use "load")
# -
if [[ ! -z ${2} ]]; then
  python3 -c 'from src import *; get_iers()'
  _gui=$(echo ${2} | tr '[A-Z]' '[a-z]')
  [[ ${_gui} = "gui" ]] && (nohup ${BOK_GALIL_TCL}/bokSplash.tcl >> /dev/null 2>&1 &)
fi
