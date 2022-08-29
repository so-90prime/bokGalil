# Indi Software

## Requirement(s)

 - Ubuntu 20.04

 - www.indilib.org

 - https://github.com/indilib/indi.git

## Pre-Requisite(s)

```bash
  % apt update
  % apt-get install -y \
     git \
     cdbs \
     dkms \
     cmake \
     swig \
     fxload \
     libev-dev \
     libgps-dev \
     libgsl-dev \
     libraw-dev \
     libusb-dev \
     zlib1g-dev \
     libftdi-dev \
     libgsl0-dev \
     libjpeg-dev \
     libkrb5-dev \
     libnova-dev \
     libtiff-dev \
     libfftw3-dev \
     librtlsdr-dev \
     libcfitsio-dev \
     libgphoto2-dev \
     build-essential \
     libusb-1.0-0-dev \
     libdc1394-22-dev \
     libboost-regex-dev \
     libcurl4-gnutls-dev \
     libtheora-dev
```

## Build (as root)

```bash
  % mkdir -p /usr/local/IndiProjects/
  % cd /usr/local/IndiProjects/
  % git clone https://github.com/indilib/indi.git
  % mkdir -p /usr/local/IndiProjects/build/indi-core
  % cd /usr/local/IndiProjects/build/indi-core
  % cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Debug /usr/local/IndiProjects/indi
  % make -j4
  % make install
```

## Test

```bash
  % indiserver -v /usr/local/IndiProjects/build/indi-core/examples/tutorial_one/tutorial_one
```

Then connect with XEphem -> View -> Sky View -> Telescope -> INDI panel -> Connect


## PyIndi Client

```bash
  % python3 -m pip install pyindi-client
```

--------------------------------------

Last Modified: 20220829

Last Author: Phil Daly (pndaly@arizona.edu)
