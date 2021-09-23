# Indi Software

## Requirement(s)

 - Ubuntu 20.04.2 LTS

 - www.indilib.org

## Pre-Requisite(s)

```bash
 % apt update
 % apt install -y libnova-dev libcfitsio-dev libgsl-dev libjpeg-dev libtiff-dev libfftw3-dev
 % apt install -y libcurl4-gnutls-dev zlib1g-dev libusb-1.0-0-dev build-essential cmake
 % apt install -y ffmpeg ffmpeg2theora libtheora-dev swig libz3-dev
```

## GitHub (as root)

```bash
 % cd /usr/local
 % git clone https://github.com/indilib/indi.git
```

## Build (as root)

```bash
 % cd /usr/local/indi
 % git status
 % git pull
```

```bash
 % mkdir -p /usr/local/indi-build
 % cd /usr/local/indi-build
 % cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Debug /usr/local/indi
 % make -j4
 % make install
```

## PyIndi Client

```bash
  % python3 -m pip install pyindi-client
```

--------------------------------------

Last Modified: 20210917

Last Author: Phil Daly (pndaly@arizona.edu)
