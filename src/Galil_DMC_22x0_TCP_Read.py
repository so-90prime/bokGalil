#!/usr/bin/env python3


# +
# import(s)
# -
import argparse
import contextlib
import mmap
import os
import struct
import sys
import time


# +
# function: msleep()
# -
def msleep(_seconds: int = 1):
    time.sleep(_seconds/1000.0)


# +
# constant(s)
# -
BOK_DELAY = 1000
BOK_STR_64 = 64
BOK_TIME_STAMP = 26
BOK_TCP_MMAP_FILE = "/dev/shm/tcp_shm"


# +
# function: read_mmap_to_dict()
# -
def read_mmap_to_dict(_m: mmap.mmap = None, _verbose: bool = False) -> dict:

    # declare variables and initialize them
    _tcp_d, _hardware, _software, _timestamp = {}, '', '', ''

    # decode memory map (caveat lector: this is very fickle and sensitive to changes!)
    try:
        _m.seek(0)
        _tcp_d['jd'] = float(f"{struct.unpack('d', _m.read(8))[0]}")
        _tcp_d['status'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _tcp_d['filtvals_0'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['filtvals_1'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['filtvals_2'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['filtvals_3'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['filtvals_4'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['filtvals_5'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['position_0'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['position_1'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['position_2'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['position_3'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['position_4'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['position_5'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['position_6'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['position_7'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_dista'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_distall'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_distb'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_distc'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_distgcam'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_errfilt'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filtbit1'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filtbit2'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filtbit3'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filtbit4'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filtbit5'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filtbit6'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filtbit7'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filtbit8'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filthigh'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filtisin'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filtlow'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filtnum'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filtrac'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filtrbl'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filtrdc'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filtrfl'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filtrsp'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filttac'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filttdc'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filttdis'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filttnud'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filttsc'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filttsp'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_filtval'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_focac'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_focbl'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_focdc'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_focfl'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_focrefa'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_focrefb'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_focrefc'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_focrfset'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_focsp'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_gfac'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_gfcent'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_gfdc'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_gfiltac'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_gfiltdc'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_gfiltn'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_gfiltq'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_gfiltsp'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_gfsp'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_initfilt'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_nmoves'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_nrot'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_reqfilt'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_totfoca'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_totfocb'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_totfocc'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_vecac'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_vecdc'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['lv_vecsp'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _tcp_d['gstatus'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _tcp_d['simulate'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _tcp_d['shutdown'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _tcp_d['counter'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _timestamp = _m.read(BOK_TIME_STAMP)
        _hardware = _m.read(BOK_STR_64)
        _software = _m.read(BOK_STR_64)
        _tcp_d['timestamp'] = _timestamp.decode().rstrip('\x00')
        _tcp_d['hardware'] = _hardware.decode().rstrip('\x00')
        _tcp_d['software'] = _software.decode().rstrip('\x00')
        _tcp_d['file_read_error'] = ''
    except Exception as _x:
        return {'file_read_error': f'{_x}'}
    else:
        if _verbose:
            print(f"_tcp_d={_tcp_d}")

    # return
    return _tcp_d


# +
# function: read_tcp_memory_as_dict()
# -
def read_tcp_memory_as_dict(_file: str = BOK_TCP_MMAP_FILE, _verbose: bool = False) -> dict:
    with open(os.path.abspath(os.path.expanduser(_file)), 'r') as _f:
        with contextlib.closing(mmap.mmap(_f.fileno(), 0, access=mmap.ACCESS_READ)) as _m:
            return read_mmap_to_dict(_m=_m, _verbose=_verbose)


# +
# function: read_tcp_memory_in_loop()
# -
def read_tcp_memory_in_loop(_file: str = BOK_TCP_MMAP_FILE, _delay: int = BOK_DELAY, _verbose: bool = False) -> None:
    _running = 1
    with open(os.path.abspath(os.path.expanduser(_file)), 'r') as _f:
        with contextlib.closing(mmap.mmap(_f.fileno(), 0, access=mmap.ACCESS_READ)) as _m:
            while _running > 0:
                _d = read_mmap_to_dict(_m=_m, _verbose=_verbose)
                if 'shutdown' in _d and _d['shutdown'] > 0:
                    _running = 0
                msleep(_delay)


# +
# main()
# -
if __name__ == '__main__':

    # get command line argument(s)
    _p = argparse.ArgumentParser(description='Galil_DMC_22x0_TCP_Read', formatter_class=argparse.RawTextHelpFormatter)
    _p.add_argument('--file', default=f"{BOK_TCP_MMAP_FILE}", help="""Input file [%(default)s]""")
    _p.add_argument('--delay', default=f"{BOK_DELAY}", help="""Delay (ms) [%(default)s]""")
    _p.add_argument('--loop', default=False, action='store_true', help='if present, read memory in loop')
    _p.add_argument('--verbose', default=False, action='store_true', help='if present, print output')
    _args = _p.parse_args()

    # noinspection PyBroadException
    try:
        if bool(_args.loop):
            read_tcp_memory_in_loop(_file=_args.file, _delay=int(_args.delay), _verbose=bool(_args.verbose))
        else:
            read_tcp_memory_as_dict(_file=_args.file, _verbose=bool(_args.verbose))
    except Exception as _e:
        print(f'{_e}\nUse: python3 {sys.argv[0]} --help')
