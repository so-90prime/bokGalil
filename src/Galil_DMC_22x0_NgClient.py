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
BOK_NG_BUCKETS = 64
BOK_NG_COMMAND = "COMMAND"
BOK_NG_FALSE = [0, '0', 'false', False]
BOK_NG_GFILTER_SLOTS = [1, 2, 3, 4, 5, 6]
BOK_NG_HOST = "127.0.0.1"
BOK_NG_IFILTER_SLOTS = [0, 1, 2, 3, 4, 5]
BOK_NG_INSTRUMENT = "90PRIME"
BOK_NG_PORT = 5750
BOK_NG_REQUEST = "REQUEST"
BOK_NG_STRING = 1024
BOK_NG_TELESCOPE = "BOK"
BOK_NG_TIMEOUT = 60.0
BOK_NG_TRUE = [1, '1', 'true', True]
BOK_NG_WORD = 256


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
        self.__encoder_a = math.nan
        self.__encoder_b = math.nan
        self.__encoder_c = math.nan
        self.__error = f""
        self.__gfilters = {}
        self.__gfilter_name = f""
        self.__gfilter_number = -1
        self.__gfilter_rotating = False
        self.__gfocus = math.nan
        self.__ifilters = {}
        self.__ifilter_inbeam = False
        self.__ifilter_name = f""
        self.__ifilter_number = -1
        self.__ifilter_rotating = False
        self.__ifilter_translating = False
        self.__ifocus_a = math.nan
        self.__ifocus_b = math.nan
        self.__ifocus_c = math.nan
        self.__ifocus_mean = math.nan
        self.__sock = None

    # +
    # property(s)
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

    # +
    # getter(s)
    # -
    @property
    def answer(self):
        return self.__answer

    @property
    def command(self):
        return self.__command

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
    def error(self):
        return self.__error

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
    def gfocus(self):
        return self.__gfocus

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
    def ifocus_a(self):
        return self.__ifocus_a

    @property
    def ifocus_b(self):
        return self.__ifocus_b

    @property
    def ifocus_c(self):
        return self.__ifocus_c

    @property
    def ifocus_mean(self):
        return self.__ifocus_mean

    @property
    def sock(self):
        return self.__sock

    # +
    # method: command_exit()
    # -
    def command_exit(self) -> bool:
        """ BOK 90PRIME <cmd-id> COMMAND EXIT """

        # set variable(s)
        self.__answer, self.__command, self.__error = f"", f"BOK 90PRIME {get_jd()} COMMAND EXIT\r\n", f""

        # send command and receive response
        try:
            self.__sock.send(self.__command.encode())
            self.__answer = self.__sock.recv(BOK_NG_STRING).decode()
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"
        else:
            self.__error = f""

        # parse answer, eg 'BOK 90PRIME <cmd-id> EXIT OK'
        if 'EXIT' not in self.__answer and 'OK' not in self.__answer:
            self.__error = f"{self.__command.replace('EXIT OK', 'ERROR (no response)')}"
            return False
        else:
            return True

    # +
    # method: command_ifilter_load()
    # -
    def command_ifilter_load(self) -> bool:
        """ BOK 90PRIME <cmd-id> COMMAND IFILTER LOAD """

        # set variable(s)
        self.__answer, self.__command, self.__error = f"", f"BOK 90PRIME {get_jd()} COMMAND IFILTER LOAD\r\n", f""

        # send command and receive response
        try:
            self.__sock.send(self.__command.encode())
            self.__answer = self.__sock.recv(BOK_NG_STRING).decode()
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"
        else:
            self.__error = f""

        # parse answer, eg 'BOK 90PRIME <cmd-id> ERROR (reason)'
        if 'ERROR' in self.__answer:
            self.__error = f"{self.__answer}"
            return False
        elif 'OK' in self.__answer:
            self.__error = f""
            return True

    # +
    # method: command_ifilter_unload()
    # -
    def command_ifilter_unload(self) -> bool:
        """ BOK 90PRIME <cmd-id> COMMAND IFILTER UNLOAD """

        # set variable(s)
        self.__answer, self.__command, self.__error = f"", f"BOK 90PRIME {get_jd()} COMMAND IFILTER UNLOAD\r\n", f""

        # send command and receive response
        try:
            self.__sock.send(self.__command.encode())
            self.__answer = self.__sock.recv(BOK_NG_STRING).decode()
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"
        else:
            self.__error = f""

        # parse answer, eg 'BOK 90PRIME <cmd-id> ERROR (reason)'
        if 'ERROR' in self.__answer:
            self.__error = f"{self.__answer}"
            return False
        elif 'OK' in self.__answer:
            self.__error = f""
            return True

    # +
    # method: command_test()
    # -
    def command_test(self) -> bool:
        """ BOK 90PRIME <cmd-id> COMMAND TEST """

        # set variable(s)
        self.__answer, self.__command, self.__error = f"", f"BOK 90PRIME {get_jd()} COMMAND TEST\r\n", f""

        # send command and receive response
        try:
            self.__sock.send(self.__command.encode())
            self.__answer = self.__sock.recv(BOK_NG_STRING).decode()
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"
        else:
            self.__error = f""

        # parse answer, eg 'BOK 90PRIME <cmd-id> TEST OK'
        if 'TEST' not in self.__answer and 'OK' not in self.__answer:
            self.__error = f"{self.__command.replace('TEST OK', 'ERROR (no response')}"
            return False
        else:
            return True

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
        else:
            self.__error = f""

    # +
    # method: disconnect()
    # -
    def disconnect(self) -> None:
        """ disconnects socket """
        if self.__sock is not None and hasattr(self.__sock, 'close'):
            try:
                self.__sock.close()
            except Exception as _:
                self.__error = f"{_}"
            else:
                self.__error = f""
        self.__sock = None

    # +
    # method: request_encoders()
    # -
    def request_encoders(self) -> None:
        """ BOK 90PRIME <cmd-id> REQUEST ENCODERS """

        # set variable(s)
        self.__answer, self.__command, self.__error = f"", f"BOK 90PRIME {get_jd()} REQUEST ENCODERS\r\n", f""

        # send command and receive response
        try:
            self.__sock.send(self.__command.encode())
            self.__answer = self.__sock.recv(BOK_NG_STRING).decode()
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"
        else:
            self.__error = f""

        # parse answer, eg 'BOK 90PRIME <cmd-id> ERROR (reason)'
        if 'ERROR' in self.__answer:
            self.__error = f"{self.__answer}"

        # parse answer, eg 'BOK 90PRIME <cmd-id> OK A=-0.355 B=1.443 C=0.345'
        elif 'OK' in self.__answer:
            for _elem in self.__answer.split():
                if 'A=' in _elem:
                    try:
                        self.__encoder_a = float(_elem.split('=')[1])
                    except Exception as _ea:
                        self.__error = f"{_ea}"
                        self.__encoder_a = math.nan
                    else:
                        self.__error = f""
                elif 'B=' in _elem:
                    try:
                        self.__encoder_b = float(_elem.split('=')[1])
                    except Exception as _eb:
                        self.__error = f"{_eb}"
                        self.__encoder_b = math.nan
                    else:
                        self.__error = f""
                elif 'C=' in _elem:
                    try:
                        self.__encoder_c = float(_elem.split('=')[1])
                    except Exception as _ec:
                        self.__error = f"{_ec}"
                        self.__encoder_c = math.nan
                    else:
                        self.__error = f""

    # +
    # method: request_gfilter()
    # -
    def request_gfilter(self) -> None:
        """ BOK 90PRIME <cmd-id> REQUEST GFILTER """

        # set variable(s)
        self.__answer, self.__command, self.__error = f"", f"BOK 90PRIME {get_jd()} REQUEST GFILTER\r\n", f""

        # send command and receive response
        try:
            self.__sock.send(self.__command.encode())
            self.__answer = self.__sock.recv(BOK_NG_STRING).decode()
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"
        else:
            self.__error = f""

        # parse answer, eg 'BOK 90PRIME <cmd-id> ERROR (reason)'
        if 'ERROR' in self.__answer:
            self.__error = f"{self.__error}, {self.__answer}"

        # parse answer, eg 'BOK 90PRIME <cmd-id> OK GFILTN=4:red ROTATING=False'
        elif 'OK' in self.__answer:
            for _elem in self.__answer.split():
                if 'GFILTN=' in _elem:
                    try:
                        self.__gfilter_name = f"{_elem.split('=')[1].split(':')[1]}"
                        self.__gfilter_number = int(_elem.split('=')[1].split(':')[0])
                    except Exception as _eg:
                        self.__error = f"{_eg}"
                        self.__gfilter_name = f""
                        self.__gfilter_number = -1
                    else:
                        self.__error = f""
                elif 'ROTATING=' in _elem:
                    try:
                        self.__gfilter_rotating = True if _elem.split('=')[1].lower() in BOK_NG_TRUE else False
                    except Exception as _er:
                        self.__error = f"{_er}"
                        self.__gfilter_rotating = f"Unknown"
                    else:
                        self.__error = f""

    # +
    # method: request_gfilters()
    # -
    def request_gfilters(self) -> None:
        """ BOK 90PRIME <cmd-id> REQUEST GFILTERS """

        # set variable(s)
        self.__answer, self.__command, self.__error = f"", f"BOK 90PRIME {get_jd()} REQUEST GFILTERS\r\n", f""

        # send command and receive response
        try:
            self.__sock.send(self.__command.encode())
            self.__answer = self.__sock.recv(BOK_NG_STRING).decode()
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"
        else:
            self.__error = f""

        # parse answer, eg 'BOK 90PRIME <cmd-id> ERROR (reason)'
        if 'ERROR' in self.__answer:
            self.__error = f"{self.__error}, {self.__answer}"

        # parse answer, eg 'BOK 90PRIME <cmd-id> OK 1=1:green 2=2:open 3=3:neutral 4=4:red 5=5:open 6=6:blue'
        elif 'OK' in self.__answer:
            self.__error, self.__gfilters = f"", {}
            for _elem in self.__answer.split():
                if '=' in _elem:
                    try:
                        _slot = int(_elem.split('=')[0])
                    except Exception as _es:
                        self.__error = f"{_es}"
                        _slot = -1
                    else:
                        self.__error = f""
                    if _slot in BOK_NG_GFILTER_SLOTS:
                        try:
                            _name = _elem.split('=')[1].split(':')[1]
                            _number = int(_elem.split('=')[1].split(':')[0])
                        except Exception as _en:
                            self.__error = f"{_en}"
                            _name = f""
                            _number = -1
                        else:
                            self.__error = f""
                            self.__gfilters = {**self.__gfilters,
                                               **{f"Slot {_slot}": {"Number": _number, "Name": _name}}}

    # +
    # method: request_gfocus()
    # -
    def request_gfocus(self) -> None:
        """ BOK 90PRIME <cmd-id> REQUEST GFOCUS """

        # set variable(s)
        self.__answer, self.__command, self.__error = f"", f"BOK 90PRIME {get_jd()} REQUEST GFOCUS\r\n", f""

        # send command and receive response
        try:
            self.__sock.send(self.__command.encode())
            self.__answer = self.__sock.recv(BOK_NG_STRING).decode()
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"
        else:
            self.__error = f""

        # parse answer, eg 'BOK 90PRIME <cmd-id> ERROR (reason)'
        if 'ERROR' in self.__answer:
            self.__error = f"{self.__error}, {self.__answer}"

        # parse answer, eg 'BOK 90PRIME <cmd-id> OK GFOCUS=-0.355'
        elif 'OK' in self.__answer:
            for _elem in self.__answer.split():
                if 'GFOCUS=' in _elem:
                    try:
                        self.__gfocus = float(_elem.split('=')[1])
                    except Exception as _eg:
                        self.__error = f"{_eg}"
                        self.__gfocus = math.nan
                    else:
                        self.__error = f""

    # +
    # method: request_ifilter()
    # -
    def request_ifilter(self) -> None:
        """ BOK 90PRIME <cmd-id> REQUEST IFILTER """

        # set variable(s)
        self.__answer, self.__command, self.__error = f"", f"BOK 90PRIME {get_jd()} REQUEST IFILTER\r\n", f""

        # send command and receive response
        try:
            self.__sock.send(self.__command.encode())
            self.__answer = self.__sock.recv(BOK_NG_STRING).decode()
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"
        else:
            self.__error = f""

        # parse answer, eg 'BOK 90PRIME <cmd-id> ERROR (reason)'
        if 'ERROR' in self.__answer:
            self.__error = f"{self.__error}, {self.__answer}"

        # parse answer, eg 'BOK 90PRIME <cmd-id> OK FILTVAL=18:Bob INBEAM=True ROTATING=False TRANSLATING=False'
        elif 'OK' in self.__answer:
            self.__error = f""
            for _elem in self.__answer.split():
                if 'FILTVAL=' in _elem:
                    try:
                        self.__ifilter_name = f"{_elem.split('=')[1].split(':')[1]}"
                        self.__ifilter_number = int(_elem.split('=')[1].split(':')[0])
                    except Exception as _ef:
                        self.__error = f"{_ef}"
                        self.__ifilter_name = f""
                        self.__ifilter_number = -1
                    else:
                        self.__error = f""
                elif 'INBEAM=' in _elem:
                    try:
                        self.__ifilter_inbeam = True if _elem.split('=')[1].lower() in BOK_NG_TRUE else False
                    except Exception as _ei:
                        self.__error = f"{_ei}"
                        self.__ifilter_inbeam = f"Unknown"
                    else:
                        self.__error = f""
                elif 'ROTATING=' in _elem:
                    try:
                        self.__ifilter_rotating = True if _elem.split('=')[1].lower() in BOK_NG_TRUE else False
                    except Exception as _er:
                        self.__error = f"{_er}"
                        self.__ifilter_rotating = f"Unknown"
                    else:
                        self.__error = f""
                elif 'TRANSLATING=' in _elem:
                    try:
                        self.__ifilter_translating = True if _elem.split('=')[1].lower() in BOK_NG_TRUE else False
                    except Exception as _et:
                        self.__error = f"{_et}"
                        self.__ifilter_translating = f"Unknown"
                    else:
                        self.__error = f""

    # +
    # method: request_ifilters()
    # -
    def request_ifilters(self) -> None:
        """ BOK 90PRIME <cmd-id> REQUEST IFILTERS """

        # set variable(s)
        self.__answer, self.__command, self.__error = f"", f"BOK 90PRIME {get_jd()} REQUEST IFILTERS\r\n", f""

        # send command and receive response
        try:
            self.__sock.send(self.__command.encode())
            self.__answer = self.__sock.recv(BOK_NG_STRING).decode()
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"
        else:
            self.__error = f""

        # parse answer, eg 'BOK 90PRIME <cmd-id> ERROR (reason)'
        if 'ERROR' in self.__answer:
            self.__error = f"{self.__error}, {self.__answer}"

        # parse answer , eg 'BOK 90PRIME <cmd-id> OK 0=18:Bob 1=2:g 2=3:r 3=4:i 4=5:z 5=6:u'
        elif 'OK' in self.__answer:
            self.__error, self.__ifilters = f"", {}
            for _elem in self.__answer.split():
                if '=' in _elem:
                    try:
                        _slot = int(_elem.split('=')[0])
                    except Exception as _es:
                        self.__error = f"{_es}"
                        _slot = -1
                    else:
                        self.__error = f""
                    if _slot in BOK_NG_IFILTER_SLOTS:
                        try:
                            _name = _elem.split('=')[1].split(':')[1]
                            _number = int(_elem.split('=')[1].split(':')[0])
                        except Exception as _en:
                            self.__error = f"{_en}"
                            _name = f""
                            _number = -1
                        else:
                            self.__error = f""
                            self.__ifilters = {**self.__ifilters,
                                               **{f"Slot {_slot}": {"Number": _number, "Name": _name}}}

    # +
    # method: request_ifocus()
    # -
    def request_ifocus(self) -> None:
        """ BOK 90PRIME <cmd-id> REQUEST IFOCUS """

        # set variable(s)
        self.__answer, self.__command, self.__error = f"", f"BOK 90PRIME {get_jd()} REQUEST IFOCUS\r\n", f""

        # send command and receive response
        try:
            self.__sock.send(self.__command.encode())
            self.__answer = self.__sock.recv(BOK_NG_STRING).decode()
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"
        else:
            self.__error = f""

        # parse answer, eg 'BOK 90PRIME <cmd-id> ERROR (reason)'
        if 'ERROR' in self.__answer:
            self.__error = f"{self.__answer}"

        # parse answer, eg 'BOK 90PRIME <cmd-id> OK A=-0.355 B=1.443 C=0.345'
        elif 'OK' in self.__answer:
            for _elem in self.__answer.split():
                if 'A=' in _elem:
                    try:
                        self.__ifocus_a = float(_elem.split('=')[1])
                    except Exception as _ea:
                        self.__error = f"{_ea}"
                        self.__ifocus_a = math.nan
                    else:
                        self.__error = f""
                elif 'B=' in _elem:
                    try:
                        self.__ifocus_b = float(_elem.split('=')[1])
                    except Exception as _eb:
                        self.__error = f"{_eb}"
                        self.__ifocus_b = math.nan
                    else:
                        self.__error = f""
                elif 'C=' in _elem:
                    try:
                        self.__ifocus_c = float(_elem.split('=')[1])
                    except Exception as _ec:
                        self.__error = f"{_ec}"
                        self.__ifocus_c = math.nan
                    else:
                        self.__error = f""
                elif 'MEAN=' in _elem:
                    try:
                        self.__ifocus_mean = float(_elem.split('=')[1])
                    except Exception as _em:
                        self.__error = f"{_em}"
                        self.__ifocus_mean = math.nan
                    else:
                        self.__error = f""


# +
# function: checkout_requests()
# -
def checkout_requests(_host: str = BOK_NG_HOST, _port: int = BOK_NG_PORT, _timeout: float = BOK_NG_TIMEOUT) -> None:

    # exercise command(s) and request(s)
    _client = None
    try:

        # instantiate client and connect to server
        _client = NgClient(host=_host, port=_port, timeout=_timeout)
        _client.connect()
        print(f"Client instantiated: sock={_client.sock}")

        # request encoders
        _client.request_encoders()
        print(f"Encoder A: {_client.encoder_a}")
        print(f"Encoder B: {_client.encoder_b}")
        print(f"Encoder C: {_client.encoder_c}")

        # request guider filters
        _client.request_gfilters()
        print(f"Guider filters loaded: {_client.gfilters}")

        # request current guider filter
        _client.request_gfilter()
        print(f"Guider current filter name: {_client.ifilter_name}")
        print(f"Guider current filter number: {_client.ifilter_number}")
        print(f"Guider current filter rotating: {_client.ifilter_rotating}")

        # request guider focus
        _client.request_gfocus()
        print(f"Guider Focus: {_client.gfocus}")

        # request instrument filters
        _client.request_ifilters()
        print(f"Instrument filters loaded: {_client.ifilters}")

        # request current instrument filter
        _client.request_ifilter()
        print(f"Instrument current filter inbeam: {_client.ifilter_inbeam}")
        print(f"Instrument current filter name: {_client.ifilter_name}")
        print(f"Instrument current filter number: {_client.ifilter_number}")
        print(f"Instrument current filter rotating: {_client.ifilter_rotating}")
        print(f"Instrument current filter translating: {_client.ifilter_translating}")

        # request instrument focus
        _client.request_ifocus()
        print(f"Instrument Focus A: {_client.ifocus_a}")
        print(f"Instrument Focus B: {_client.ifocus_b}")
        print(f"Instrument Focus C: {_client.ifocus_c}")

    except Exception as _x:
        # report this error
        print(f"{_x}")
        # report last client error
        if _client is not None and hasattr(_client, 'error'):
            print(f"{_client.error}")

    # disconnect from server
    if _client is not None and hasattr(_client, 'disconnect'):
        _client.disconnect()


# +
# function: checkout_commands()
# -
def checkout_commands(_host: str = BOK_NG_HOST, _port: int = BOK_NG_PORT, _timeout: float = BOK_NG_TIMEOUT) -> None:

    # exercise command(s) and request(s)
    _client = None
    try:

        # instantiate client and connect to server
        _client = NgClient(host=_host, port=_port, timeout=_timeout)
        _client.connect()
        print(f"Client instantiated: sock={_client.sock}")

        # command ifilter load
        if _client.command_ifilter_load():
            print(f"client passed test")
        else:
            print(f"client failed test, {_client.error}")

        # command ifilter unload
        if _client.command_ifilter_unload():
            print(f"client passed test")
        else:
            print(f"client failed test, {_client.error}")

        # command test
        if _client.command_test():
            print(f"client passed test")
        else:
            print(f"client failed test, {_client.error}")

        # command exit
        if _client.command_exit():
            print(f"client passed exit")
            if _client is not None and hasattr(_client, 'disconnect'):
                _client.disconnect()
            _client = None
        else:
            print(f"client failed exit, {_client.error}")

    except Exception as _x:
        # report this error
        print(f"{_x}")
        # report last client error
        if _client is not None and hasattr(_client, 'error'):
            print(f"{_client.error}")

    # disconnect from server
    if _client is not None and hasattr(_client, 'disconnect'):
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
        checkout_requests(_host=_args.host, _port=int(_args.port), _timeout=float(_args.timeout))
        checkout_commands(_host=_args.host, _port=int(_args.port), _timeout=float(_args.timeout))
    except Exception as _:
        print(f"{_}\nUse: {__doc__}")
