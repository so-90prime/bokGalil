#!/usr/bin/env python3
# -*- coding: utf-8 -*-


# +
# import(s)
# -
from astropy.time import Time
from datetime import datetime
from datetime import timedelta

import socket


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

        # set default(s)
        self.__answer = f""
        self.__command = f""
        self.__error = f""
        self.__ifilters = {}
        self.__gfilters = {}
        self.__sock = None

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
    def gfilters(self):
        return self.__gfilters

    @property
    def ifilters(self):
        return self.__ifilters

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
        self.__sock = None
        self.__error = f""

    # +
    # method: get_ifilters()
    # -
    def get_ifilters(self) -> None:
        """ send: BOK 90PRIME <cmd-id> REQUEST IFILTERS """
        self.__answer, self.__command, self.__error = f"", f"BOK 90PRIME {get_jd()} REQUEST IFILTERS\r\n", f""
        try:
            self.__sock.send(self.__command.encode())
            self.__answer = self.__sock.recv(BOK_NG_STRING).decode()
            if 'ERROR' in self.__answer:
                self.__error = f"{self.__answer}"
            if 'OK' in self.__answer:
                self.__error, self.__ifilters = f"", {}
                # expect BOK 90PRIME 2459671.4276736835 OK 0=18:Bob 1=2:g 2=3:r 3=4:i 4=5:z 5=6:u
                for _e in self.__answer.split():
                    if '=' in _e:
                        self.__ifilters = {**self.__ifilters, **{
                            f"Slot {_e.split('=')[0]}": {"Number": int(_e.split('=')[1].split(':')[0]),
                                                         "Name": f"{_e.split('=')[1].split(':')[1]}"}}}
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"

    # +
    # method: get_gfilters()
    # -
    def get_gfilters(self) -> None:
        """ send: BOK 90PRIME <cmd-id> REQUEST GFILTERS """
        self.__answer, self.__command, self.__error = f"", f"BOK 90PRIME {get_jd()} REQUEST GFILTERS\r\n", f""
        try:
            self.__sock.send(self.__command.encode())
            self.__answer = self.__sock.recv(BOK_NG_STRING).decode()
            if 'ERROR' in self.__answer:
                self.__error = f"{self.__answer}"
            if 'OK' in self.__answer:
                self.__error, self.__gfilters = f"", {}
                # expect BOK 90PRIME 2459671.4276736835 OK 1=1:green 2=2:open 3=3:neutral 4=4:red 5=5:open 6=6:blue
                for _e in self.__answer.split():
                    if '=' in _e:
                        self.__gfilters = {**self.__gfilters, **{
                            f"Slot {_e.split('=')[0]}": {"Number": int(_e.split('=')[1].split(':')[0]),
                                                         "Name": f"{_e.split('=')[1].split(':')[1]}"}}}
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"


# +
# main()
# -
if __name__ == '__main__':
    _client = NgClient()
    _client.connect()
    _client.get_ifilters()
    _client.get_gfilters()
    _client.disconnect()

    print(f"Instrument filters loaded: {_client.ifilters}")
    print(f"Guider filters loaded: {_client.gfilters}")
