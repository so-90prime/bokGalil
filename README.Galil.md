# Galil DMC 22x0 Information

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
 - The 'B' value (0.292) is associated with the E axis analog input multiplied by 10.0/32767.0;
 - The 'C' value (0.307) is associated with the F axis analog input multiplied by 10.0/32767.0;
 
Now, to change the instrument focus, we use the algorithm:

```bash
  1. Get values get from user:
     Eg, A=-200, B=-150, C=-100;
  2. Divide values by 1000.0:
     Eg, A/=1000.0=-0.2, B/=1000.0=-0.15, C/=1000.0=-0.1
  3. Add to current values:
     Eg A+=0.297=0.097, B+=0.292=0.142, C+=0.307=0.207
  4. Multiply values by A/D conversion factor -1.0/0.00055=-1818.2
     Eg, A*=-1818.2=-176.37, B*=-1818.2=-258.18, C*=-1818.2=-376.37
  5. Send nearest integer value to galil DISTA, DISTB, DISTC values:
     Eg, DISTA=-176;DISTB=-258;DISTC=-376;
  6. Execute the 'XQ #COORDMV;'
 ```

--------------------------------------

Last Modified: 20210928

Last Author: Phil Daly (pndaly@arizona.edu)