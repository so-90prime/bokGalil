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
# function: read_tcp_mmap_file()
# -
def read_tcp_mmap_file(_file: str = BOK_TCP_MMAP_FILE, _delay: int = BOK_DELAY) -> None:
    _file = os.path.abspath(os.path.expanduser(_file))
    running = 1
    with open(_file, 'r') as _f:
        with contextlib.closing(mmap.mmap(_f.fileno(), 0, access=mmap.ACCESS_READ)) as _m:
            while running > 0:
                # reset to start of file
                _m.seek(0)
                # read data
                jd = struct.unpack('d', _m.read(8))
                status = struct.unpack('i', _m.read(4))
                filtvals_0 = struct.unpack('f', _m.read(4))
                filtvals_1 = struct.unpack('f', _m.read(4))
                filtvals_2 = struct.unpack('f', _m.read(4))
                filtvals_3 = struct.unpack('f', _m.read(4))
                filtvals_4 = struct.unpack('f', _m.read(4))
                filtvals_5 = struct.unpack('f', _m.read(4))
                position_0 = struct.unpack('f', _m.read(4))
                position_1 = struct.unpack('f', _m.read(4))
                position_2 = struct.unpack('f', _m.read(4))
                position_3 = struct.unpack('f', _m.read(4))
                position_4 = struct.unpack('f', _m.read(4))
                position_5 = struct.unpack('f', _m.read(4))
                position_6 = struct.unpack('f', _m.read(4))
                position_7 = struct.unpack('f', _m.read(4))
                lv_dista = struct.unpack('f', _m.read(4))
                lv_distall = struct.unpack('f', _m.read(4))
                lv_distb = struct.unpack('f', _m.read(4))
                lv_distc = struct.unpack('f', _m.read(4))
                lv_distgcam = struct.unpack('f', _m.read(4))
                lv_errfilt = struct.unpack('f', _m.read(4))
                lv_filtbit1 = struct.unpack('f', _m.read(4))
                lv_filtbit2 = struct.unpack('f', _m.read(4))
                lv_filtbit3 = struct.unpack('f', _m.read(4))
                lv_filtbit4 = struct.unpack('f', _m.read(4))
                lv_filtbit5 = struct.unpack('f', _m.read(4))
                lv_filtbit6 = struct.unpack('f', _m.read(4))
                lv_filtbit7 = struct.unpack('f', _m.read(4))
                lv_filtbit8 = struct.unpack('f', _m.read(4))
                lv_filthigh = struct.unpack('f', _m.read(4))
                lv_filtisin = struct.unpack('f', _m.read(4))
                lv_filtlow = struct.unpack('f', _m.read(4))
                lv_filtnum = struct.unpack('f', _m.read(4))
                lv_filtrac = struct.unpack('f', _m.read(4))
                lv_filtrbl = struct.unpack('f', _m.read(4))
                lv_filtrdc = struct.unpack('f', _m.read(4))
                lv_filtrfl = struct.unpack('f', _m.read(4))
                lv_filtrsp = struct.unpack('f', _m.read(4))
                lv_filttac = struct.unpack('f', _m.read(4))
                lv_filttdc = struct.unpack('f', _m.read(4))
                lv_filttdis = struct.unpack('f', _m.read(4))
                lv_filttnud = struct.unpack('f', _m.read(4))
                lv_filttsc = struct.unpack('f', _m.read(4))
                lv_filttsp = struct.unpack('f', _m.read(4))
                lv_filtval = struct.unpack('f', _m.read(4))
                lv_focac = struct.unpack('f', _m.read(4))
                lv_focbl = struct.unpack('f', _m.read(4))
                lv_focdc = struct.unpack('f', _m.read(4))
                lv_focfl = struct.unpack('f', _m.read(4))
                lv_focrefa = struct.unpack('f', _m.read(4))
                lv_focrefb = struct.unpack('f', _m.read(4))
                lv_focrefc = struct.unpack('f', _m.read(4))
                lv_focrfset = struct.unpack('f', _m.read(4))
                lv_focsp = struct.unpack('f', _m.read(4))
                lv_gfac = struct.unpack('f', _m.read(4))
                lv_gfcent = struct.unpack('f', _m.read(4))
                lv_gfdc = struct.unpack('f', _m.read(4))
                lv_gfiltac = struct.unpack('f', _m.read(4))
                lv_gfiltdc = struct.unpack('f', _m.read(4))
                lv_gfiltn = struct.unpack('f', _m.read(4))
                lv_gfiltq = struct.unpack('f', _m.read(4))
                lv_gfiltsp = struct.unpack('f', _m.read(4))
                lv_gfsp = struct.unpack('f', _m.read(4))
                lv_initfilt = struct.unpack('f', _m.read(4))
                lv_nmoves = struct.unpack('f', _m.read(4))
                lv_nrot = struct.unpack('f', _m.read(4))
                lv_reqfilt = struct.unpack('f', _m.read(4))
                lv_totfoca = struct.unpack('f', _m.read(4))
                lv_totfocb = struct.unpack('f', _m.read(4))
                lv_totfocc = struct.unpack('f', _m.read(4))
                lv_vecac = struct.unpack('f', _m.read(4))
                lv_vecdc = struct.unpack('f', _m.read(4))
                lv_vecsp = struct.unpack('f', _m.read(4))
                gstatus = struct.unpack('i', _m.read(4))
                simulate = struct.unpack('i', _m.read(4))
                shutdown = struct.unpack('i', _m.read(4))
                counter = struct.unpack('i', _m.read(4))
                timestamp = _m.read(BOK_TIME_STAMP)
                hardware = _m.read(BOK_STR_64)
                software = _m.read(BOK_STR_64)
                timestamp_d = timestamp.decode().rstrip('\x00')
                hardware_d = hardware.decode().rstrip('\x00')
                software_d = software.decode().rstrip('\x00')
                # dump record
                print(f"\n{'<SIMULATE> ' if simulate[0] > 0 else '<DIRECT> '}"
                      f"status={status[0]}, "
                      f"filtvals[0]={filtvals_0[0]}, "
                      f"filtvals[1]={filtvals_1[0]}, "
                      f"filtvals[2]={filtvals_2[0]}, "
                      f"filtvals[3]={filtvals_3[0]}, "
                      f"filtvals[4]={filtvals_4[0]}, "
                      f"filtvals[5]={filtvals_5[0]}, "
                      f"position[0]={position_0[0]}, "
                      f"position[1]={position_1[0]}, "
                      f"position[2]={position_2[0]}, "
                      f"position[3]={position_3[0]}, "
                      f"position[4]={position_4[0]}, "
                      f"position[5]={position_5[0]}, "
                      f"position[6]={position_6[0]}, "
                      f"position[7]={position_7[0]}, "
                      f"lv.dista={lv_dista[0]}, "
                      f"lv.distall={lv_distall[0]}, "
                      f"lv.distb={lv_distb[0]}, "
                      f"lv.distc={lv_distc[0]}, "
                      f"lv.distgcam={lv_distgcam[0]}, "
                      f"lv.errfilt={lv_errfilt[0]}, "
                      f"lv.filtbit1={lv_filtbit1[0]}, "
                      f"lv.filtbit2={lv_filtbit2[0]}, "
                      f"lv.filtbit3={lv_filtbit3[0]}, "
                      f"lv.filtbit4={lv_filtbit4[0]}, "
                      f"lv.filtbit5={lv_filtbit5[0]}, "
                      f"lv.filtbit6={lv_filtbit6[0]}, "
                      f"lv.filtbit7={lv_filtbit7[0]}, "
                      f"lv.filtbit8={lv_filtbit8[0]}, "
                      f"lv.filthigh={lv_filthigh[0]}, "
                      f"lv.filtisin={lv_filtisin[0]}, "
                      f"lv.filtlow={lv_filtlow[0]}, "
                      f"lv.filtnum={lv_filtnum[0]}, "
                      f"lv.filtrac={lv_filtrac[0]}, "
                      f"lv.filtrbl={lv_filtrbl[0]}, "
                      f"lv.filtrdc={lv_filtrdc[0]}, "
                      f"lv.filtrfl={lv_filtrfl[0]}, "
                      f"lv.filtrsp={lv_filtrsp[0]}, "
                      f"lv.filttac={lv_filttac[0]}, "
                      f"lv.filttdc={lv_filttdc[0]}, "
                      f"lv.filttdis={lv_filttdis[0]}, "
                      f"lv.filttnud={lv_filttnud[0]}, "
                      f"lv.filttsc={lv_filttsc[0]}, "
                      f"lv.filttsp={lv_filttsp[0]}, "
                      f"lv.filtval={lv_filtval[0]}, "
                      f"lv.focac={lv_focac[0]}, "
                      f"lv.focbl={lv_focbl[0]}, "
                      f"lv.focdc={lv_focdc[0]}, "
                      f"lv.focfl={lv_focfl[0]}, "
                      f"lv.focrefa={lv_focrefa[0]}, "
                      f"lv.focrefb={lv_focrefb[0]}, "
                      f"lv.focrefc={lv_focrefc[0]}, "
                      f"lv.focrfset={lv_focrfset[0]}, "
                      f"lv.focsp={lv_focsp[0]}, "
                      f"lv.gfac={lv_gfac[0]}, "
                      f"lv.gfcent={lv_gfcent[0]}, "
                      f"lv.gfdc={lv_gfdc[0]}, "
                      f"lv.gfiltac={lv_gfiltac[0]}, "
                      f"lv.gfiltdc={lv_gfiltdc[0]}, "
                      f"lv.gfiltn={lv_gfiltn[0]}, "
                      f"lv.gfiltq={lv_gfiltq[0]}, "
                      f"lv.gfiltsp={lv_gfiltsp[0]}, "
                      f"lv.gfsp={lv_gfsp[0]}, "
                      f"lv.initfilt={lv_initfilt[0]}, "
                      f"lv.nmoves={lv_nmoves[0]}, "
                      f"lv.nrot={lv_nrot[0]}, "
                      f"lv.reqfilt={lv_reqfilt[0]}, "
                      f"lv.totfoca={lv_totfoca[0]}, "
                      f"lv.totfocb={lv_totfocb[0]}, "
                      f"lv.totfocc={lv_totfocc[0]}, "
                      f"lv.vecac={lv_vecac[0]}, "
                      f"lv.vecdc={lv_vecdc[0]}, "
                      f"lv.vecsp={lv_vecsp[0]}, "
                      f"gstatus={gstatus[0]}, "
                      f"simulate={simulate[0]}, "
                      f"shutdown={shutdown[0]}, "
                      f"counter={counter[0]}, "
                      f"jd={jd[0]:.8f}, "
                      f"timestamp='{timestamp_d}', "
                      f"hardware='{hardware_d}', "
                      f"software='{software_d}'")
                # check shutdown flag
                if shutdown[0] > 0:
                    running = 0
                msleep(_delay)


# +
# main()
# -
if __name__ == '__main__':

    # get command line argument(s)
    _p = argparse.ArgumentParser(description='Galil_DMC_22x0_TCP_Read', formatter_class=argparse.RawTextHelpFormatter)
    _p.add_argument('-f', '--file', default=f"{BOK_TCP_MMAP_FILE}", help="""Input file [%(default)s]""")
    _p.add_argument('-d', '--delay', default=f"{BOK_DELAY}", help="""Delay (ms) [%(default)s]""")
    _args = _p.parse_args()

    # noinspection PyBroadException
    try:
        read_tcp_mmap_file(_file=_args.file, _delay=int(_args.delay))
    except Exception as _e:
        print(f'{_e}')
        print(f'Use: python3 {sys.argv[0]} --help')
