#!/usr/bin/env python3
# -*- coding: utf-8 -*-


# +
# import(s)
# -
from astropy.time import Time
from datetime import datetime
from datetime import timedelta

import argparse
import math
import socket


# +
# __doc__
# -
__doc__ = """ python3 Galil_DMC_22x0_NgClient.py --help """


# +
# constant(s)
# -
BOK_NG_HOST = "127.0.0.1"
BOK_NG_PORT = 5750
BOK_NG_TELESCOPE = "BOK"
BOK_NG_INSTRUMENT = "90PRIME"
BOK_NG_COMMAND = "COMMAND"
BOK_NG_REQUEST = "REQUEST"
BOK_NG_TIMEOUT = 60.0
BOK_NG_STRING = 1024
BOK_NG_BUCKETS = 64
BOK_NG_WORD = 256

TRUE = [1, '1', 'true', True]
FALSE = [0, '0', 'false', False]


# +
# function: get_utc()
# -
def get_utc(_days: int = 0) -> str:
    return (datetime.utcnow() + timedelta(days=_days)).isoformat()


# +
# function: get_jd()
# -
def get_jd(_days: int = 0) -> str:
    return Time(get_utc(_days=_days)).jd


# +
# class: NgClient()
# -
# noinspection PyBroadException
class NgClient(object):

    # +
    # method: __init__()
    # -
    def __init__(self, host: str = BOK_NG_HOST, port: int = BOK_NG_PORT, timeout: float = BOK_NG_TIMEOUT):

        # get input(s)
        self.host = host
        self.port = port
        self.timeout = timeout

        # set variable(s)
        self.__answer = f""
        self.__command = f""
        self.__error = f""
        self.__sock = None

        self.__encoder_a = math.nan
        self.__encoder_b = math.nan
        self.__encoder_c = math.nan

        self.__ifocus_a = math.nan
        self.__ifocus_b = math.nan
        self.__ifocus_c = math.nan
        self.__gfocus = math.nan

        self.__ifilters = {}
        self.__ifilter_inbeam = False
        self.__ifilter_name = f""
        self.__ifilter_number = -1
        self.__ifilter_rotating = False
        self.__ifilter_translating = False

        self.__gfilters = {}
        self.__gfilter_name = f""
        self.__gfilter_number = -1
        self.__gfilter_rotating = False

    # +
    # decorator(s)
    # -
    @property
    def host(self):
        return self.__host

    @host.setter
    def host(self, host: str = BOK_NG_HOST) -> None:
        self.__host = host if host.strip() != '' else BOK_NG_HOST

    @property
    def port(self):
        return self.__port

    @port.setter
    def port(self, port: int = BOK_NG_PORT) -> None:
        self.__port = port if port > 0 else BOK_NG_PORT

    @property
    def timeout(self):
        return self.__timeout

    @timeout.setter
    def timeout(self, timeout: float = BOK_NG_PORT) -> None:
        self.__timeout = timeout if timeout > 0.0 else BOK_NG_TIMEOUT

    @property
    def answer(self):
        return self.__answer

    @property
    def command(self):
        return self.__command

    @property
    def error(self):
        return self.__error

    @property
    def encoder_a(self):
        return self.__encoder_a

    @property
    def encoder_b(self):
        return self.__encoder_b

    @property
    def encoder_c(self):
        return self.__encoder_c

    @property
    def gfocus(self):
        return self.__gfocus

    @property
    def ifocus_a(self):
        return self.__ifocus_a

    @property
    def ifocus_b(self):
        return self.__ifocus_b

    @property
    def ifocus_c(self):
        return self.__ifocus_c

    @property
    def gfilters(self):
        return self.__gfilters

    @property
    def gfilter_name(self):
        return self.__gfilter_name

    @property
    def gfilter_number(self):
        return self.__gfilter_number

    @property
    def gfilter_rotating(self):
        return self.__gfilter_rotating

    @property
    def ifilters(self):
        return self.__ifilters

    @property
    def ifilter_inbeam(self):
        return self.__ifilter_inbeam

    @property
    def ifilter_name(self):
        return self.__ifilter_name

    @property
    def ifilter_number(self):
        return self.__ifilter_number

    @property
    def ifilter_rotating(self):
        return self.__ifilter_rotating

    @property
    def ifilter_translating(self):
        return self.__ifilter_translating

    @property
    def sock(self):
        return self.__sock

    # +
    # method: connect()
    # -
    def connect(self) -> None:
        """ connects to host:port via socket """
        try:
            self.__sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.__sock.connect((socket.gethostbyname(self.__host), self.__port))
            self.__sock.settimeout(self.__timeout)
        except Exception as _:
            self.__error = f"{_}"
            self.__sock = None

    # +
    # method: disconnect()
    # -
    def disconnect(self) -> None:
        """ disconnects socket """
        if self.__sock is not None and hasattr(self.__sock, 'close'):
            self.__sock.close()
        self.__error = f""
        self.__sock = None

    # +
    # method: get_encoders()
    # -
    def get_encoders(self) -> None:
        """ BOK 90PRIME <cmd-id> REQUEST ENCODERS """
        self.__answer, self.__command, self.__error = f"", f"BOK 90PRIME {get_jd()} REQUEST ENCODERS\r\n", f""
        try:
            self.__sock.send(self.__command.encode())
            self.__answer = self.__sock.recv(BOK_NG_STRING).decode()
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"

        # parse answer, eg 'BOK 90PRIME <cmd-id> ERROR (reason)'
        if 'ERROR' in self.__answer:
            self.__error = f"{self.__error}, {self.__answer}"

        # parse answer, eg 'BOK 90PRIME <cmd-id> OK A=-0.355 B=1.443 C=0.345'
        elif 'OK' in self.__answer:
            for _e in self.__answer.split():
                if 'A=' in _e:
                    self.__encoder_a = float(_e.split('=')[1])
                elif 'B=' in _e:
                    self.__encoder_b = float(_e.split('=')[1])
                elif 'C=' in _e:
                    self.__encoder_c = float(_e.split('=')[1])

    # +
    # method: get_ifocus()
    # -
    def get_ifocus(self) -> None:
        """ BOK 90PRIME <cmd-id> REQUEST IFOCUS """
        self.__answer, self.__command, self.__error = f"", f"BOK 90PRIME {get_jd()} REQUEST IFOCUS\r\n", f""
        try:
            self.__sock.send(self.__command.encode())
            self.__answer = self.__sock.recv(BOK_NG_STRING).decode()
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"

        # parse answer, eg 'BOK 90PRIME <cmd-id> ERROR (reason)'
        if 'ERROR' in self.__answer:
            self.__error = f"{self.__error}, {self.__answer}"

        # parse answer, eg 'BOK 90PRIME <cmd-id> OK A=-0.355 B=1.443 C=0.345'
        elif 'OK' in self.__answer:
            for _e in self.__answer.split():
                if 'A=' in _e:
                    self.__ifocus_a = float(_e.split('=')[1])
                elif 'B=' in _e:
                    self.__ifocus_b = float(_e.split('=')[1])
                elif 'C=' in _e:
                    self.__ifocus_c = float(_e.split('=')[1])

    # +
    # method: get_gfocus()
    # -
    def get_gfocus(self) -> None:
        """ BOK 90PRIME <cmd-id> REQUEST GFOCUS """
        self.__answer, self.__command, self.__error = f"", f"BOK 90PRIME {get_jd()} REQUEST GFOCUS\r\n", f""
        try:
            self.__sock.send(self.__command.encode())
            self.__answer = self.__sock.recv(BOK_NG_STRING).decode()
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"

        # parse answer, eg 'BOK 90PRIME <cmd-id> ERROR (reason)'
        if 'ERROR' in self.__answer:
            self.__error = f"{self.__error}, {self.__answer}"

        # parse answer, eg 'BOK 90PRIME <cmd-id> OK GFOCUS=-0.355'
        elif 'OK' in self.__answer:
            for _e in self.__answer.split():
                if 'GFOCUS=' in _e:
                    self.__gfocus = float(_e.split('=')[1])

    # +
    # method: get_gfilter()
    # -
    def get_gfilter(self) -> None:
        """ BOK 90PRIME <cmd-id> REQUEST GFILTER """
        self.__answer, self.__command, self.__error = f"", f"BOK 90PRIME {get_jd()} REQUEST GFILTER\r\n", f""
        try:
            self.__sock.send(self.__command.encode())
            self.__answer = self.__sock.recv(BOK_NG_STRING).decode()
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"

        # parse answer, eg 'BOK 90PRIME <cmd-id> ERROR (reason)'
        if 'ERROR' in self.__answer:
            self.__error = f"{self.__error}, {self.__answer}"

        # parse answer, eg 'BOK 90PRIME <cmd-id> OK GFILTN=4:red ROTATING=False'
        elif 'OK' in self.__answer:
            self.__error = f""
            for _e in self.__answer.split():
                if 'GFILTN=' in _e:
                    self.__gfilter_number = int(_e.split('=')[1].split(':')[0])
                    self.__gfilter_name = f"{_e.split('=')[1].split(':')[1]}"
                elif 'ROTATING=' in _e:
                    self.__gfilter_rotating = True if _e.split('=')[1].lower() in TRUE else False

    # +
    # method: get_gfilters()
    # -
    def get_gfilters(self) -> None:
        """ BOK 90PRIME <cmd-id> REQUEST GFILTERS """
        self.__answer, self.__command, self.__error = f"", f"BOK 90PRIME {get_jd()} REQUEST GFILTERS\r\n", f""
        try:
            self.__sock.send(self.__command.encode())
            self.__answer = self.__sock.recv(BOK_NG_STRING).decode()
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"

        # parse answer, eg 'BOK 90PRIME <cmd-id> ERROR (reason)'
        if 'ERROR' in self.__answer:
            self.__error = f"{self.__error}, {self.__answer}"

        # parse answer, eg 'BOK 90PRIME <cmd-id> OK 1=1:green 2=2:open 3=3:neutral 4=4:red 5=5:open 6=6:blue'
        elif 'OK' in self.__answer:
            self.__error, self.__gfilters = f"", {}
            for _e in self.__answer.split():
                if '=' in _e:
                    self.__gfilters = {**self.__gfilters, **{
                        f"Slot {_e.split('=')[0]}": {"Number": int(_e.split('=')[1].split(':')[0]),
                                                     "Name": f"{_e.split('=')[1].split(':')[1]}"}}}

    # +
    # method: get_ifilter()
    # -
    def get_ifilter(self) -> None:
        """ BOK 90PRIME <cmd-id> REQUEST IFILTER """
        self.__answer, self.__command, self.__error = f"", f"BOK 90PRIME {get_jd()} REQUEST IFILTER\r\n", f""
        try:
            self.__sock.send(self.__command.encode())
            self.__answer = self.__sock.recv(BOK_NG_STRING).decode()
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"

        # parse answer, eg 'BOK 90PRIME <cmd-id> ERROR (reason)'
        if 'ERROR' in self.__answer:
            self.__error = f"{self.__error}, {self.__answer}"

        # parse answer, eg 'BOK 90PRIME <cmd-id> OK FILTVAL=18:Bob INBEAM=True ROTATING=False TRANSLATING=False'
        elif 'OK' in self.__answer:
            self.__error = f""
            for _e in self.__answer.split():
                if 'FILTVAL=' in _e:
                    self.__ifilter_number = int(_e.split('=')[1].split(':')[0])
                    self.__ifilter_name = f"{_e.split('=')[1].split(':')[1]}"
                elif 'INBEAM=' in _e:
                    self.__ifilter_inbeam = True if _e.split('=')[1].lower() in TRUE else False
                elif 'ROTATING=' in _e:
                    self.__ifilter_rotating = True if _e.split('=')[1].lower() in TRUE else False
                elif 'TRANSLATING=' in _e:
                    self.__ifilter_translating = True if _e.split('=')[1].lower() in TRUE else False

    # +
    # method: get_ifilters()
    # -
    def get_ifilters(self) -> None:
        """ BOK 90PRIME <cmd-id> REQUEST IFILTERS """
        self.__answer, self.__command, self.__error = f"", f"BOK 90PRIME {get_jd()} REQUEST IFILTERS\r\n", f""
        try:
            self.__sock.send(self.__command.encode())
            self.__answer = self.__sock.recv(BOK_NG_STRING).decode()
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"

        # parse answer, eg 'BOK 90PRIME <cmd-id> ERROR (reason)'
        if 'ERROR' in self.__answer:
            self.__error = f"{self.__error}, {self.__answer}"

        # parse answer , eg 'BOK 90PRIME <cmd-id> OK 0=18:Bob 1=2:g 2=3:r 3=4:i 4=5:z 5=6:u'
        elif 'OK' in self.__answer:
            self.__error, self.__ifilters = f"", {}
            for _e in self.__answer.split():
                if '=' in _e:
                    self.__ifilters = {**self.__ifilters, **{
                        f"Slot {_e.split('=')[0]}": {"Number": int(_e.split('=')[1].split(':')[0]),
                                                     "Name": f"{_e.split('=')[1].split(':')[1]}"}}}


# +
# function: checkout_commands()
# -
def checkout_commands(_host: str = BOK_NG_HOST, _port: int = BOK_NG_PORT, _timeout: float = BOK_NG_TIMEOUT) -> None:

    # instantiate client
    _client = NgClient(host=_host, port=_port, timeout=_timeout)

    # connect to server
    _client.connect()

    # get dictionary of instrument filters
    _client.get_ifilters()
    print(f"Instrument filters loaded: {_client.ifilters}")

    # get current instrument filter
    _client.get_ifilter()
    print(f"Instrument current filter inbeam: {_client.ifilter_inbeam}")
    print(f"Instrument current filter name: {_client.ifilter_name}")
    print(f"Instrument current filter number: {_client.ifilter_number}")
    print(f"Instrument current filter rotating: {_client.ifilter_rotating}")
    print(f"Instrument current filter translating: {_client.ifilter_translating}")

    # get dictionary of guider filters
    _client.get_gfilters()
    print(f"Guider filters loaded: {_client.gfilters}")

    # get current guider filter
    _client.get_gfilter()
    print(f"Guider current filter name: {_client.ifilter_name}")
    print(f"Guider current filter number: {_client.ifilter_number}")
    print(f"Guider current filter rotating: {_client.ifilter_rotating}")

    # get encoders
    _client.get_encoders()
    print(f"Encoder A: {_client.encoder_a}")
    print(f"Encoder B: {_client.encoder_b}")
    print(f"Encoder C: {_client.encoder_c}")

    # get focus
    _client.get_ifocus()
    print(f"Instrument Focus A: {_client.ifocus_a}")
    print(f"Instrument Focus B: {_client.ifocus_b}")
    print(f"Instrument Focus C: {_client.ifocus_c}")

    _client.get_gfocus()
    print(f"Guider Focus: {_client.gfocus}")

    # disconnect from server
    _client.disconnect()


# +
# main()
# -
if __name__ == '__main__':

    # get command line argument(s)
    _p = argparse.ArgumentParser(description='Galil_DMC_22x0_TCP_Read', formatter_class=argparse.RawTextHelpFormatter)
    _p.add_argument('--host', default=f"{BOK_NG_HOST}", help="""Host [%(default)s]""")
    _p.add_argument('--port', default=BOK_NG_PORT, help="""Port [%(default)s]""")
    _p.add_argument('--timeout', default=BOK_NG_TIMEOUT, help="""Timeout (s) [%(default)s]""")
    _args = _p.parse_args()

    # noinspection PyBroadException
    try:
        checkout_commands(_host=_args.host, _port=int(_args.port), _timeout=float(_args.timeout))
    except Exception as _:
        print(f"{_}\nUse: {__doc__}")
