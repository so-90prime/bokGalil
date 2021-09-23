# bokGalil Software

## Requirement(s)

 - Ubuntu 20.04.2 LTS

 - Indi code (www.indilib.org)

 - gclib (www.galil.com)

## Caveat(s)

This software only supports /dev/shm shared memory under Unix.

## Pre-Requisite(s)

```bash
 % apt -y install libxml2-util
```

## Set Up

```bash
 % cd <code-path>/bokGalil
 % source etc/bokGalil.sh `pwd` load
 % cd $BOK_GALIL_HOME
 % sudo python3 -m pip install -r requirements.txt
 % cd $BOK_GALIL_SRC
 % make everything
```

## Start, Stop, Status

NB: At the Bok Telescope, this code is run under the mtnops account and after a `conda activate`!

```bash
 % cd <code-path>/bokGalil
 % source etc/bokGalil.sh `pwd`
 % cd $BOK_GALIL_BIN
 % bash Galil_DMC_22x0.sh --help
```

 - Check status:

```bash
 % bash Galil_DMC_22x0.sh --command=status
```

 - To stop the software:

```bash
 # check
 % bash Galil_DMC_22x0.sh --command=stop --dry-run
 # for real
 % bash Galil_DMC_22x0.sh --command=stop
```

 - To start the software:

```bash
 # check
 % bash Galil_DMC_22x0.sh --command=start --dry-run
 # for real
 % bash Galil_DMC_22x0.sh --command=start
```

Log files are held in $BOK_GALIL_LOG.

--------------------------------------

Last Modified: 20210917

Last Author: Phil Daly (pndaly@arizona.edu)
