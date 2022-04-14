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
import os
import random
import socket


# +
# __doc__
# -
__doc__ = """ python3 Galil_DMC_22x0_NgClient.py --help """


# +
# constant(s)
# -
BOK_NG_HELP = os.path.abspath(os.path.expanduser(os.path.join(os.getenv("BOK_GALIL_DOCS", os.getcwd()), 'bok_ng_commands.txt')))
BOK_NG_FALSE = [0, '0', 'false', False]
BOK_NG_GFILTER_SLOTS = [1, 2, 3, 4, 5, 6]
BOK_NG_HOST = "10.30.1.2"
BOK_NG_IFILTER_SLOTS = [0, 1, 2, 3, 4, 5]
BOK_NG_INSTRUMENT = "90PRIME"
BOK_NG_PORT = 5750
BOK_NG_STRING = 1024
BOK_NG_TELESCOPE = "BOK"
BOK_NG_TIMEOUT = 60.0
BOK_NG_TRUE = [1, '1', 'true', True]


# +
# initialize
# -
random.seed(os.getpid())


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
    def __init__(self, host: str = BOK_NG_HOST, port: int = BOK_NG_PORT,
                 timeout: float = BOK_NG_TIMEOUT, simulate: bool = False) -> None:

        # get input(s)
        self.host = host
        self.port = port
        self.timeout = timeout
        self.simulate = simulate

        # set variable(s)
        self.__answer = f""
        self.__command = f""
        self.__encoder_a = math.nan
        self.__encoder_b = math.nan
        self.__encoder_c = math.nan
        self.__error = f""
        self.__gfilters = {}
        self.__gfilters_names = []
        self.__gfilters_numbers = []
        self.__gfilters_slots = []
        self.__gfilter_name = f""
        self.__gfilter_number = -1
        self.__gfilter_rotating = False
        self.__gdelta = math.nan
        self.__gfocus = math.nan
        self.__ifilters = {}
        self.__ifilters_names = []
        self.__ifilters_numbers = []
        self.__ifilters_slots = []
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

    @property
    def simulate(self):
        return self.__simulate

    @simulate.setter
    def simulate(self, simulate: bool = False) -> None:
        self.__simulate = simulate

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
    def gfilters_names(self):
        return self.__gfilters_names

    @property
    def gfilters_numbers(self):
        return self.__gfilters_numbers

    @property
    def gfilters_slots(self):
        return self.__gfilters_slots

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
    def gdelta(self):
        return self.__gdelta

    @property
    def gfocus(self):
        return self.__gfocus

    @property
    def ifilters(self):
        return self.__ifilters

    @property
    def ifilters_names(self):
        return self.__ifilters_names

    @property
    def ifilters_numbers(self):
        return self.__ifilters_numbers

    @property
    def ifilters_slots(self):
        return self.__ifilters_slots

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
    # (hidden) method: __dump__()
    # -
    def __dump__(self):
        """ dump(s) variable(s) """

        print(f"self = {self}")
        print(f"self.__host = {self.__host}")
        print(f"self.__port = {self.__port}")
        print(f"self.__timeout = {self.__timeout}")
        print(f"self.__simulate = {self.__simulate}")

        print(f"self.__answer = '{self.__answer}'")
        print(f"self.__command = '{self.__command}'")
        print(f"self.__encoder_a = {self.__encoder_a}")
        print(f"self.__encoder_b = {self.__encoder_b}")
        print(f"self.__encoder_c = {self.__encoder_c}")
        print(f"self.__error = '{self.__error}'")
        print(f"self.__gfilters = {self.__gfilters}")
        print(f"self.__gfilters_names = {self.__gfilters_names}")
        print(f"self.__gfilters_numbers = {self.__gfilters_numbers}")
        print(f"self.__gfilters_slots = {self.__gfilters_slots}")
        print(f"self.__gfilter_name = '{self.__gfilter_name}'")
        print(f"self.__gfilter_number = {self.__gfilter_number}")
        print(f"self.__gfilter_rotating = {self.__gfilter_rotating}")
        print(f"self.__gdelta = {self.__gdelta}")
        print(f"self.__gfocus = {self.__gfocus}")
        print(f"self.__ifilters = {self.__ifilters}")
        print(f"self.__ifilters_names = {self.__ifilters_names}")
        print(f"self.__ifilters_numbers = {self.__ifilters_numbers}")
        print(f"self.__ifilters_slots = {self.__ifilters_slots}")
        print(f"self.__ifilter_inbeam = {self.__ifilter_inbeam}")
        print(f"self.__ifilter_name = '{self.__ifilter_name}'")
        print(f"self.__ifilter_number = {self.__ifilter_number}")
        print(f"self.__ifilter_rotating = {self.__ifilter_rotating}")
        print(f"self.__ifilter_translating = {self.__ifilter_translating}")
        print(f"self.__ifocus_a = {self.__ifocus_a}")
        print(f"self.__ifocus_b = {self.__ifocus_b}")
        print(f"self.__ifocus_c = {self.__ifocus_c}")
        print(f"self.__ifocus_mean = {self.__ifocus_mean}")
        print(f"self.__sock = None {self.__sock}")

    # +
    # method: connect()
    # -
    def connect(self) -> None:
        """ connects to host:port via socket """

        try:
            self.__sock = None
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
    # method: converse()
    # -
    def converse(self, talk: str = f"") -> str:
        """ converses across socket """

        # send and recv data
        if talk.strip() == "":
            return f""

        # initialize variable(s)
        self.__answer = f""
        self.__error = f""

        # change command if simulate is enabled
        if self.__simulate:
            _cmd = talk.split()
            _cmd[2] = f"SIMULATE"
            self.__command = f"{' '.join(_cmd)}\r\n"
        else:
            self.__command = f"{talk}\r\n"

        # converse
        try:
            self.__sock.send(self.__command.encode())
            self.__answer = self.__sock.recv(BOK_NG_STRING).decode()
        except Exception as _:
            self.__answer = f""
            self.__error = f"{_}"
        else:
            self.__error = f""

        # return
        return self.__answer

    # +
    # method: parse_command_response()
    # -
    def parse_command_response(self, reply: str = '') -> bool:
        """ parses command response from socket """

        _reply = reply.upper()
        if not _reply.startswith(BOK_NG_TELESCOPE):
            return False

        elif BOK_NG_INSTRUMENT not in _reply:
            return False

        else:
            if " OK" in _reply:
                return True
            elif " ERROR" in _reply:
                self.__error = f"{_reply}".replace('\n', '')
                return False
            else:
                self.__error = f"{_reply} ERROR (unknown response)".replace('\n', '')
                return False

    # +
    # method: command_exit()
    # -
    def command_exit(self) -> bool:
        """ BOK 90PRIME <cmd-id> COMMAND EXIT """

        _reply = self.converse(f"BOK 90PRIME {get_jd()} COMMAND EXIT")
        return self.parse_command_response(_reply)

    # +
    # method: command_gfilter_init()
    # -
    def command_gfilter_init(self) -> bool:
        """ BOK 90PRIME <cmd-id> COMMAND GFILTER INIT """

        _reply = self.converse(f"BOK 90PRIME {get_jd()} COMMAND GFILTER INIT")
        return self.parse_command_response(_reply)

    # +
    # method: command_gfilter_name()
    # -
    def command_gfilter_name(self, gname: str = '') -> bool:
        """ BOK 90PRIME <cmd-id> COMMAND GFILTER NAME <str> """

        if gname.strip() == "":
            return False

        if not self.__gfilters:
            self.request_gfilters()

        if gname.strip() not in self.__gfilters_names:
            return False

        _reply = self.converse(f"BOK 90PRIME {get_jd()} COMMAND GFILTER NAME {gname}")
        return self.parse_command_response(_reply)

    # +
    # method: command_gfilter_number()
    # -
    def command_gfilter_number(self, gnumber: int = -1) -> bool:
        """ BOK 90PRIME <cmd-id> COMMAND GFILTER NUMBER <int> """

        if gnumber not in BOK_NG_GFILTER_SLOTS:
            return False

        if not self.__gfilters:
            self.request_gfilters()

        if gnumber not in self.__gfilters_numbers:
            return False

        _reply = self.converse(f"BOK 90PRIME {get_jd()} COMMAND GFILTER NUMBER {gnumber}")
        return self.parse_command_response(_reply)

    # +
    # method: command_gfocus_delta()
    # -
    def command_gfocus_delta(self, gdelta: float = math.nan) -> bool:
        """ BOK 90PRIME <cmd-id> COMMAND GFOCUS DELTA <float> """

        if math.nan < gfocus < -math.nan:
            return False

        _reply = self.converse(f"BOK 90PRIME {get_jd()} COMMAND GFOCUS DELTA {gdelta}")
        return self.parse_command_response(_reply)

    # +
    # method: command_ifilter_init()
    # -
    def command_ifilter_init(self) -> bool:
        """ BOK 90PRIME <cmd-id> COMMAND IFILTER INIT """

        _reply = self.converse(f"BOK 90PRIME {get_jd()} COMMAND IFILTER INIT")
        return self.parse_command_response(_reply)

    # +
    # method: command_ifilter_load()
    # -
    def command_ifilter_load(self) -> bool:
        """ BOK 90PRIME <cmd-id> COMMAND IFILTER LOAD """

        _reply = self.converse(f"BOK 90PRIME {get_jd()} COMMAND IFILTER LOAD")
        return self.parse_command_response(_reply)

    # +
    # method: command_ifilter_name()
    # -
    def command_ifilter_name(self, iname: str = '') -> bool:
        """ BOK 90PRIME <cmd-id> COMMAND IFILTER NAME <str> """

        if iname.strip() == "":
            return False

        if not self.__ifilters:
            self.request_ifilters()

        if iname.strip() not in self.__ifilters_names:
            return False

        _reply = self.converse(f"BOK 90PRIME {get_jd()} COMMAND IFILTER NAME {iname}")
        return self.parse_command_response(_reply)

    # +
    # method: command_ifilter_number()
    # -
    def command_ifilter_number(self, inumber: int = -1) -> bool:
        """ BOK 90PRIME <cmd-id> COMMAND IFILTER NUMBER <int> """

        if inumber not in BOK_NG_IFILTER_SLOTS:
            return False

        if not self.__ifilters:
            self.request_ifilters()

        if inumber not in self.__gfilters_numbers:
            return False

        _reply = self.converse(f"BOK 90PRIME {get_jd()} COMMAND IFILTER NUMBER {inumber}")
        return self.parse_command_response(_reply)

    # +
    # method: command_ifilter_unload()
    # -
    def command_ifilter_unload(self) -> bool:
        """ BOK 90PRIME <cmd-id> COMMAND IFILTER UNLOAD """

        _reply = self.converse(f"BOK 90PRIME {get_jd()} COMMAND IFILTER UNLOAD")
        return self.parse_command_response(_reply)

    # +
    # method: command_ifocus()
    # -
    def command_ifocus(self, a: float = math.nan, b: float = math.nan, c: float = math.nan) -> bool:
        """ BOK 90PRIME <cmd-id> COMMAND IFOCUS A <float> B <float> C <float> """

        if (math.nan < a < -math.nan) or (math.nan < b < -math.nan) or (math.nan < c < -math.nan):
            return False

        _reply = self.converse(f"BOK 90PRIME {get_jd()} COMMAND IFOCUS A {a} B {b} C {c}")
        return self.parse_command_response(_reply)

    # +
    # method: command_ifocusall()
    # -
    def command_ifocusall(self, focus: float = math.nan) -> bool:
        """ BOK 90PRIME <cmd-id> COMMAND IFOCUSALL <float> """

        if math.nan < focus < -math.nan:
            return False

        _reply = self.converse(f"BOK 90PRIME {get_jd()} COMMAND IFOCUSALL {focus}")
        return self.parse_command_response(_reply)

    # +
    # method: command_lvdt()
    # -
    def command_lvdt(self, a: float = math.nan, b: float = math.nan, c: float = math.nan) -> bool:
        """ BOK 90PRIME <cmd-id> COMMAND LVDT A <float> B <float> C <float> """

        if (math.nan < a < -math.nan) or (math.nan < b < -math.nan) or (math.nan < c < -math.nan):
            return False

        _reply = self.converse(f"BOK 90PRIME {get_jd()} COMMAND LVDT A {a} B {b} C {c}")
        return self.parse_command_response(_reply)

    # +
    # method: command_lvdtall()
    # -
    def command_lvdtall(self, lvdt: float = math.nan) -> bool:
        """ BOK 90PRIME <cmd-id> COMMAND LVDTALL <float> """

        if math.nan < lvdt < -math.nan:
            return False

        _reply = self.converse(f"BOK 90PRIME {get_jd()} COMMAND LVDTALL {lvdt}")
        return self.parse_command_response(_reply)

    # +
    # method: command_test()
    # -
    def command_test(self) -> bool:
        """ BOK 90PRIME <cmd-id> COMMAND TEST """

        _reply = self.converse(f"BOK 90PRIME {get_jd()} COMMAND TEST")
        return self.parse_command_response(_reply)

    # +
    # method: request_encoders()
    # -
    def request_encoders(self) -> None:
        """ BOK 90PRIME <cmd-id> REQUEST ENCODERS """

        # talk to hardware
        self.converse(f"BOK 90PRIME {get_jd()} REQUEST ENCODERS")

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

        # talk to hardware
        self.converse(f"BOK 90PRIME {get_jd()} REQUEST GFILTER")

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

        # talk to hardware
        self.converse(f"BOK 90PRIME {get_jd()} REQUEST GFILTERS")

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

        # parse dictionary
        self.__gfilters_names = [_v['Name'] for _k, _v in self.__gfilters.items()]
        self.__gfilters_numbers = [_v['Number'] for _k, _v in self.__gfilters.items()]
        self.__gfilters_slots = [int(_.split()[1]) for _ in self.__gfilters]

    # +
    # method: request_gfocus()
    # -
    def request_gfocus(self) -> None:
        """ BOK 90PRIME <cmd-id> REQUEST GFOCUS """

        # talk to hardware
        self.converse(f"BOK 90PRIME {get_jd()} REQUEST GFOCUS")

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

        # talk to hardware
        self.converse(f"BOK 90PRIME {get_jd()} REQUEST IFILTER")

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

        # talk to hardware
        self.converse(f"BOK 90PRIME {get_jd()} REQUEST IFILTERS")

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

        # parse dictionary
        self.__ifilters_names = [_v['Name'] for _k, _v in self.__ifilters.items()]
        self.__ifilters_numbers = [_v['Number'] for _k, _v in self.__ifilters.items()]
        self.__ifilters_slots = [int(_.split()[1]) for _ in self.__ifilters]

    # +
    # method: request_ifocus()
    # -
    def request_ifocus(self) -> None:
        """ BOK 90PRIME <cmd-id> REQUEST IFOCUS """

        # talk to hardware
        self.converse(f"BOK 90PRIME {get_jd()} REQUEST IFOCUS")

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
# function: ngclient_test()
# -
def ngclient_test(_host: str = BOK_NG_HOST, _port: int = BOK_NG_PORT, _timeout: float = BOK_NG_TIMEOUT, 
                  _simulate: bool = False, _verbose: bool = False) -> None:

    # exercise command(s) and request(s)
    _client = None
    try:

        # instantiate client and connect to server
        _client = NgClient(host=_host, port=_port, timeout=_timeout, simulate=_simulate)
        _client.connect()
        print(f"Client instantiated OK, host={_client.host}, port={_client.port}, sock={_client.sock}")

        # +
        # request(s)
        # -
        _client.request_encoders()
        if _verbose:
            _ans, _err = _client.answer.replace('\n', ''), _client.error.replace('\n', ''),
            print(f"request_encoders() answer='{_ans}', error='{_err}'")

        _client.request_gfilters()
        if _verbose:
            _ans, _err = _client.answer.replace('\n', ''), _client.error.replace('\n', ''),
            print(f"request_gfilters() answer='{_ans}', error='{_err}'")

        _client.request_gfilter()
        if _verbose:
            _ans, _err = _client.answer.replace('\n', ''), _client.error.replace('\n', ''),
            print(f"request_gfilter()>  answer='{_ans}', error='{_err}'")

        _client.request_gfocus()
        if _verbose:
            _ans, _err = _client.answer.replace('\n', ''), _client.error.replace('\n', ''),
            print(f"request_gfocus()>   answer='{_ans}', error='{_err}'")

        _client.request_ifilters()
        if _verbose:
            _ans, _err = _client.answer.replace('\n', ''), _client.error.replace('\n', ''),
            print(f"request_ifilters() answer='{_ans}', error='{_err}'")

        _client.request_ifilter()
        if _verbose:
            _ans, _err = _client.answer.replace('\n', ''), _client.error.replace('\n', ''),
            print(f"request_ifilter()>  answer='{_ans}', error='{_err}'")

        _client.request_ifocus()
        if _verbose:
            _ans, _err = _client.answer.replace('\n', ''), _client.error.replace('\n', ''),
            print(f"request_ifocus()>   answer='{_ans}', error='{_err}'")

        # dump
        print(f"{_client.__dump__()}")


        # +
        # command(s)
        # -
        print(f"command_gfilter_init() {'succeeded' if _client.command_gfilter_init() else 'failed'}")
        if _verbose:
            _ans, _err = _client.answer.replace('\n', ''), _client.error.replace('\n', ''),
            print(f"command_gfilter_init() answer='{_ans}', error='{_err}'")

        _gfilter_name = random.choice(_client.gfilters_names)
        print(f"command_gfilter_name('{_gfilter_name}') {'succeeded' if _client.command_gfilter_name(gname=_gfilter_name) else 'failed'}")
        if _verbose:
            _ans, _err = _client.answer.replace('\n', ''), _client.error.replace('\n', ''),
            print(f"command_gfilter_name() answer='{_ans}', error='{_err}'")

        _gfilter_number = random.choice(_client.gfilters_numbers)
        print(f"command_gfilter_number({_gfilter_number}) {'succeeded' if _client.command_gfilter_number(gnumber=_gfilter_number) else 'failed'}")
        if _verbose:
            _ans, _err = _client.answer.replace('\n', ''), _client.error.replace('\n', ''),
            print(f"command_gfilter_number() answer='{_ans}', error='{_err}'")

        # test guider focus
        _gfocus_delta = random.uniform(-100.0, 100.0)
        print(f"command_gfocus_delta({_gfocus_delta}) {'succeeded' if _client.command_gfocus_delta(gdelta=_gfocus_delta) else 'failed'}")
        if _verbose:
            _ans, _err = _client.answer.replace('\n', ''), _client.error.replace('\n', ''),
            print(f"command_gfocus_delta({_gfocus_delta}) answer='{_ans}', error='{_err}'")

        _gfocus_delta *= -1.0
        print(f"command_gfocus_delta({_gfocus_delta}) {'succeeded' if _client.command_gfocus_delta(gdelta=_gfocus_delta) else 'failed'}")
        if _verbose:
            _ans, _err = _client.answer.replace('\n', ''), _client.error.replace('\n', ''),
            print(f"command_gfocus_delta({_gfocus_delta}) answer='{_ans}', error='{_err}'")

        # test instrument filter wheel
        print(f"command_ifilter_init() {'succeeded' if _client.command_ifilter_init() else 'failed'}")
        if _verbose:
            _ans, _err = _client.answer.replace('\n', ''), _client.error.replace('\n', ''),
            print(f"command_ifilter_init() answer='{_ans}', error='{_err}'")

        # print(f"command_ifilter_init() {'succeeded' if _client.command_ifilter_init() else f'failed, error={_client.error}'}")
        # print(f"answer='{_client.answer}', error='{_client.error}'")
        # _ifilter_name = random.choice(_client.ifilters_names)
        # print(f"command_ifilter_name('{_ifilter}') {'succeeded' if _client.command_ifilter_name(iname=_ifilter) else f'failed, error={_client.error}'}")
        # print(f"answer='{_client.answer}', error='{_client.error}'")
        # print(f"command_ifilter_number(4) {'succeeded' if _client.command_ifilter_number(inumber=4) else f'failed, error={_client.error}'}")
        # print(f"answer='{_client.answer}', error='{_client.error}'")
        # print(f"command_ifilter_load() {'succeeded' if _client.command_ifilter_load() else f'failed, error={_client.error}'}")
        # print(f"command_ifilter_unload() {'succeeded' if _client.command_ifilter_unload() else f'failed, error={_client.error}'}")
        # print(f"answer='{_client.answer}', error='{_client.error}'")

        # print(f"command_ifocus(22.0, 33.0, 44.0) {'succeeded' if _client.command_ifocus(a=22.0, b=33.0, c=44.0) else f'failed, error={_client.error}'}")
        # print(f"command_ifocusall(55.0) {'succeeded' if _client.command_ifocusall(focus=55.0) else f'failed, error={_client.error}'}")
        # print(f"command_lvdt(22.0, 33.0, 44.0) {'succeeded' if _client.command_lvdt(a=22.0, b=33.0, c=44.0) else f'failed, error={_client.error}'}")
        # print(f"command_lvdtall(55.0) {'succeeded' if _client.command_lvdtall(lvdt=55.0) else f'failed, error={_client.error}'}")
        # print(f"command_test() {'succeeded' if _client.command_test() else f'failed, error={_client.error}'}")
        # print(f"command_exit() {'succeeded' if _client.command_exit() else f'failed, error={_client.error}'}")
        if _client.command_exit():
            if _client is not None and hasattr(_client, 'disconnect'):
                _client.disconnect()
            _client = None

    except Exception as _x:
        print(f"{_x}")
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
    _p.add_argument('--commands', default=False, action='store_true', help='Show supported commands')
    _p.add_argument('--host', default=f"{BOK_NG_HOST}", help="""Host [%(default)s]""")
    _p.add_argument('--port', default=BOK_NG_PORT, help="""Port [%(default)s]""")
    _p.add_argument('--timeout', default=BOK_NG_TIMEOUT, help="""Timeout (s) [%(default)s]""")
    _p.add_argument('--simulate', default=False, action='store_true', help='Simulate')
    _p.add_argument('--verbose', default=False, action='store_true', help='Verbose')
    _args = _p.parse_args()

    # noinspection PyBroadException
    try:
        if bool(_args.commands):
            with open(BOK_NG_HELP, 'r') as _f:
                print(f"{_f.read()}")
        else:
            ngclient_test(_host=_args.host, _port=int(_args.port), _timeout=float(_args.timeout), _simulate=bool(_args.simulate), _verbose=bool(_args.verbose))
    except Exception as _:
        print(f"{_}\nUse: {__doc__}")
