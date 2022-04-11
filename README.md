# bokGalil Software

## Requirement(s)

 - Ubuntu 16.04.7 LTS (or later)

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
 % git clone https://github.com/so-90prime/bokGalil.git
 % cd bokGalil
 % source etc/bokGalil.sh `pwd` load
 % cd $BOK_GALIL_HOME
 % sudo python3 -m pip install -r requirements.txt
 % mkdir lib log
 % cd $BOK_GALIL_SRC
 % make everything
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
 % bash Galil_DMC_22x0.sh --command=status --web-site
```

 - To stop the software:

```bash
 # check
 % bash Galil_DMC_22x0.sh --command=stop --web-site --dry-run
 # for real
 % bash Galil_DMC_22x0.sh --command=stop --web-site
```

 - To start the software:

```bash
 # check
 % bash Galil_DMC_22x0.sh --command=start --web-site --dry-run
 # for real
 % bash Galil_DMC_22x0.sh --command=start --web-site
```

Log files are written to $BOK_GALIL_LOG.

## Status

 - 20220201
```bash
   TODO:
    - Add light(s) for FILTTSC to indicate any limits encountered
    - Work to resolve guider issues. I am reasonably convinced that this driver is doing the right thing 
      but it is not showing up in the GUI so it may be, like FW-GUI, that it has internal logic;
    - Complex commands (eg focus) are currently executed as a sequence of "atomic" statements. At some 
      point we can abandon that for daisy-chained one-shot commands;
    - Initialization: I think this should be done once when the code is started so we need to agree what 
      it is that needs to be done. I think "XQ #GFWINIT;" (for the guider) and "XQ #FILTRD;" for the 
      instrument should be all that is needed. We can then retire the other commands or put them in a
      debug or engineering menu;
    - Will need to write a new SOP for Joe et al so that they know where the filter files are etc
    - Possibly reduce the amount of logging
```

--------------------------------------

Last Modified: 20220120

Last Author: Phil Daly (pndaly@arizona.edu)
