# bokGalil Software

## Requirement(s)

 - Ubuntu 16.04.7 LTS (or later)

 - INDI code (www.indilib.org)

 - gclib (www.galil.com), specifically: gcapsd_130-1_amd64.deb, gclib_396-1_amd64.deb and gdk_1.0.7.353-1_amd64.deb

 - Python3 (3.6 or later)

## Caveat(s)

This software only supports /dev/shm shared memory under Unix.

## Pre-Requisite(s)

```bash
 % apt -y install libxml2-util
```

## Set Up

```bash
 % git clone https://github.com/so-90prime/bokGalil.git
 % cd bokGalil
 % source etc/bokGalil.sh `pwd` load
 % cd $BOK_GALIL_HOME
 % sudo python3 -m pip install -r requirements.txt
 % mkdir lib log
 % cd $BOK_GALIL_SRC
 % make everything
```

## Quick Test

```bash
 % make -f test_galil.make
 % ./test_galil
```

## Start, Stop, Status

NB: At the Bok telescope, this code is run under either the *mtnops* or *primefocus* account after a `conda activate` !

```bash
 % cd <code-path>/bokGalil
 % source etc/bokGalil.sh `pwd`
 % cd $BOK_GALIL_BIN
 % bash Galil_DMC_22x0.sh --help
```

 - Check status:

```bash
 % bash Galil_DMC_22x0.sh --command=status --web-site --ng-server
```

 - To stop the software:

```bash
 # check
 % bash Galil_DMC_22x0.sh --command=stop --web-site --ng-server --dry-run
 # for real
 % bash Galil_DMC_22x0.sh --command=stop --web-site --ng-server
```

 - To start the software:

```bash
 # check
 % bash Galil_DMC_22x0.sh --command=start --web-site --ng-server --dry-run
 # for real
 % bash Galil_DMC_22x0.sh --command=start --web-site --ng-server
```

Log files are written to $BOK_GALIL_LOG.

## Status

 - 20220419
```bash
   TODO:
    - Add light(s) for FILTTSC to indicate any limits encountered
    - Complex commands (eg focus) are currently executed as a sequence of "atomic" statements. At some 
      point we can abandon that for daisy-chained one-shot commands;
    - Initialization: I think this should be done once when the code is started so we need to agree what 
      it is that needs to be done. I think "XQ #GFWINIT;" (for the guider) and "XQ #FILTRD;" for the 
      instrument should be all that is needed. We can then retire the other commands or put them in a
      debug or engineering menu;
    - Will need to write a new SOP for Joe et al so that they know where the filter files are etc
```

--------------------------------------

Last Modified: 20220527

Last Author: Phil Daly (pndaly@arizona.edu)
