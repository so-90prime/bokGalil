# bokGalil Software

## Requirement(s)

 - Ubuntu 20.04 LTS (or later)

 - INDI code (www.indilib.org and see README.indi.md)

 - gclib (www.galil.com and see README.Galil.md)

 - Python3 (3.6 or later)

## Dependencies

```bash
 % apt -y install python3-pip
 % sudo python3 -m pip install --upgrade pip
 % apt -y install libxml2-utils
```

 - dataserver (https://github.com/so-mops/dataserver.git)

 - bok-90prime-gui (https://github.com/so-90prime/bok-90prime-gui.git and pyIndi from git+https://github.com/MMTObservatory/pyINDI.git)

## Caveat(s)

This software only supports /dev/shm shared memory under Unix.

## Set Up

```bash
 % git clone https://github.com/so-90prime/bokGalil.git
 % cd bokGalil
 % source etc/bokGalil.sh `pwd` load
 % cd ${BOK_GALIL_HOME}
 % sudo python3 -m pip install -r requirements.txt
 % mkdir lib log
 % cd ${BOK_GALIL_SRC}
 % make bonsai
 % cd ${BOK_GALIL_TCL}
 % make bonsai
```

## Quick Test(s) (At Bok)

```bash
 % make -f test_galil.make
 % ./test_galil -b1
```

```bash
 % telnet 10.30.3.31
```

then execute the `LV;` command. Data should appear. Use `ctrl-]` to escape to the telnet prompt and enter `quit`.

To test the NG Server running on 10.30.1.7 5750

```bash
 % echo 'BOK 90PRIME 123 REQUEST IFILTERS' | nc -w 5 10.30.1.7 5750

 BOK 90PRIME 123 OK 0=0:F0 1=9:F9 2=0:F0 3=1:Open 4=0:F0 5=8:sagev
```

## CLI: Start, Stop, Status

```bash
 % cd /home/primefocus/bokGalil
 % source etc/bokGalil.sh `pwd` load
 % bash ${BOK_GALIL_BIN}/Galil_DMC_22x0.sh --help
```

 - Check status:

```bash
 % bash ${BOK_GALIL_BIN}/Galil_DMC_22x0.sh --command=status
```

 - To stop the software:

```bash
 % bash ${BOK_GALIL_BIN}/Galil_DMC_22x0.sh --command=stop
```

 - To start the software:

```bash
 % bash ${BOK_GALIL_BIN}/Galil_DMC_22x0.sh --command=start
```

## GUI: Start, Stop, Status

```bash
 % cd /home/primefocus/bokGalil
 % source etc/bokGalil.sh `pwd` gui
```

Log files are written to $BOK_GALIL_LOG.

## Status

 - 20221129
```bash
   TODO:
    - Complex commands (eg focus) are currently executed as a sequence of "atomic" statements. At some 
      point we can abandon that for daisy-chained one-shot commands;
    - Initialization: I think this should be done once when the code is started so we need to agree what 
      it is that needs to be done. I think "XQ #GFWINIT;" (for the guider) and "XQ #FILTRD;" for the 
      instrument should be all that is needed. We can then retire the other commands or put them in a
      debug or engineering menu;
    - Will need to write a new SOP for Joe et al so that they know where the filter files are etc
```

--------------------------------------

Last Modified: 20230124

Last Author: Phil Daly (pndaly@arizona.edu)
