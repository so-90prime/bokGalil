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

 - Adding the shared memory readers (not required for operations):

```bash
 # check
 % bash Galil_DMC_22x0.sh --command=start --dry-run
 # mare sure we have Python > 3.6
 conda activate
 # for real
 % bash Galil_DMC_22x0.sh --command=start --c-read --py-read
```

Log files are written to $BOK_GALIL_LOG.

## Status

 - 20210923
```bash
   DONE:
     Tested during nights of 21/22/23 September 2021
     Instrument filter translation completed successfully
     Instrument filter rotation completed successfully
     Instrument filter engineering commands (Populate, PopDone, ReadWheel, Initialize) completed successfully
     Instrument standard focus completed successfully
     Instrument delta focus completed successfully
     Instrument reference focus save/restore completed successfully
     Instrument nominal plane save/restore completed successfully
     Telemetry from TCP/UDP shared memory segments completed successfully
     Telemetry indicators (lights) functionality tested successfully
     Guider focus tested
     Guider filter selection tested but does not agree with the guider GUI
     Added bash control script for easy start | stop | status
     V1.0.0 Released via GitHub so-90prime repository
     Code cloned to both /home/mtnops/PycharmProjects/bokGalil (for software development and engineering)
     and /home/primefocus/bokGalil for production. A "git pull" should be executed from time to time 
     in those directories to ensure the end-user is running the latest revision.

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
