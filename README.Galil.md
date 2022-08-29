# Galil DMC 22x0 Information

## Installation of Galil Software

  - Add key from http://www.galil.com/sw/pub/all/crypto/GALIL-GPG-KEY-E29D0E4B.asc

  - Add repository from http://www.galil.com/sw/pub/ubuntu/20.04/galil.list

  - sudo apt update && sudo apt install gclib && sudo apt install gcapsd

## Caveat(s)

Information presented here was derived from perusing galil_90prime.dmc and various schematics
by software engineers. Make of that what you will!

## Hardware Connection(s)

```bash
  Axis A: actuator A
  Axis B: actuator B
  Axis C: actuator C
  Axis D: 
  Axis E: guider focus translation
  Axis F: instrument filter rotation
  Axis G: instrument filter translation
  Axis H: guider filter rotation (aka 'Oriel Filter Wheel')
```

Further, Dan Avner writes:

```bash
  Analog In 1 = Focus pot A VB    = Axis A
  Analog In 2 = Focus pot A Wiper = Axis B = LVDT A
  Analog In 3 = Focus pot B VB    = Axis C
  Analog In 4 = Focus pot B Wiper = Axis D = LVDT B
  Analog In 5 = Focus pot C VB    = Axis E
  Analog In 6 = Focus pot C Wiper = Axis F = LVDT C
  Analog In 7 = Temp sensor VB (NOT IN USE)
  Analog In 8 = Temp sensor (NOT IN USE)
```

Further, Greg Stafford writes regarding the Oriel (Guider) Filter Wheel:

```bash
  Filter ID       | Filter ID           | Hall Effect Sensors |
  (as labelled)   | (by sharpie)        | ISO1 | ISO2 | ISO3  |
      1                   5                       Y      Y
      2                   4                              Y
      3                   3                Y             Y
      4                   2                Y              
      5                   1                Y      Y       
      6                   6                       Y       
```

## Mapping(s)

+ Encoder A = aaxis_motor_position
+ Encoder B = baxis_motor_position
+ Encoder C = caxis_motor_position

+ Focus A = baxis_analog_in * (10.0/32767.0)
+ Focus B = daxis_analog_in * (10.0/32767.0)
+ Focus C = faxis_analog_in * (10.0/32767.0)

+ Guider Focus Position = eaxis_reference_position
+ Guider Focus Limit Switch = eaxis_stop_code (2, at inner limit at +4158 steps, 3 at outer limit at -5979 steps)
+ Guider Filter Wheel Rotating = haxis_moving

+ Instrument Filter Wheel Rotating = faxis_moving
+ Instrument Filter Wheel Translating = gaxis_moving

Oriel Filter Wheel (DMC code mapping):

```bash
  FNUM | SNUM
    0      7
    1      1
    2      3
    3      2
    4      6
    5      4
    6      5
    7      0
```

## Guider Filter Wheel Tests of 20220208

```bash
  Filter   | GFILTN | FNUM_IN | FNUM | SNUM_IN | SNUM | Eyeball
                                  7                0      
   green       1         1        1       1        1      5
   open        2         2        ?       3        3      4
   neutral     3         3        ?       2        2      3
   red         4         4        ?       6        6      2
   open        5         5        ?       4        4      1
   blue        6         6        ?       5        5      6
                                  0                7       
```

During the tests FNUM was inconsistent, but it's not really used by the DMC code either!
So, bok_gfilters.txt should read:

```bash
#This file contains the master look-up table for the 90prime guider filter wheel.
#Filter\tCode\tName
0	F0	Ignore_as_never_used
1	green	green
2	open	open
3	neutral	neutral
4	red	red
5	open	open
6	blue	blue
```

## Changing Instrument Focus

FW-GUI shows (for example):

```bash
 'A'    'A Ref'  'A Lim'  'A Encoder' 
  0.297  0        None     -79
  'B'    'B Ref'  'B Lim'  'B Encoder' 
  0.292  0        None     1337
  'C'    'C Ref'  'C Lim'  'C Encoder' 
  0.307  0        None     -970
```

 - The 'A Encoder' value (-79) is associated with the A axis motor position;
 - The 'B Encoder' value (+1337) is associated with the B axis motor position;
 - The 'C Encoder' value (-970) is associated with the C axis motor position;
 - The 'A' value (0.297) is associated with the B axis analog input multiplied by 10.0/32767.0;
 - The 'B' value (0.292) is associated with the D axis analog input multiplied by 10.0/32767.0;
 - The 'C' value (0.307) is associated with the F axis analog input multiplied by 10.0/32767.0;
 
Now, to change the instrument focus, we use the algorithm:

```bash
  1. Get values get from user:
     Eg, A=-200, B=-150, C=-100;
  2. Divide values by 1000.0:
     Eg, A/=1000.0=-0.2, B/=1000.0=-0.15, C/=1000.0=-0.1
  3. Subtract from current values:
     Eg A-=0.2=0.097, B-=0.15=0.142, C-=0.1=0.207
  4. Multiply values by A/D conversion factor -1.0/0.00055=-1818.2
     Eg, A*=-1818.2=-176.37, B*=-1818.2=-258.18, C*=-1818.2=-376.37
  5. Send nearest integer value to galil DISTA, DISTB, DISTC values:
     Eg, DISTA=-176;DISTB=-258;DISTC=-376;
  6. Execute the 'XQ #FOCIND;'
 ```

--------------------------------------

Last Modified: 20220201

Last Author: Phil Daly (pndaly@arizona.edu)
