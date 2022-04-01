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
        self.__error = f""
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
    def error(self):
        return self.__error

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
    def get_ifilters(self) -> str:
        """ send: BOK 90PRIME <cmd-id> REQUEST IFILTERS """
        self.__answer, self.__error = f"", f""
        try:
            self.connect()
            self.__sock.write(f"BOK 90PRIME {get_jd()} REQUEST IFILTERS\r\n")
            self.__answer = self.__sock.recv(BOK_NG_STRING)
            self.disconnect()
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"
        return self.__answer

    # +
    # method: get_gfilters()
    # -
    def get_gfilters(self) -> str:
        """ send: BOK 90PRIME <cmd-id> REQUEST GFILTERS """
        self.__answer, self.__error = f"", f""
        try:
            self.connect()
            self.__sock.write(f"BOK 90PRIME {get_jd()} REQUEST GFILTERS\r\n")
            self.__answer = self.__sock.recv(BOK_NG_STRING)
            self.disconnect()
        except Exception as _:
            self.__answer, self.__error = f"", f"{_}"
        return self.__answer


# +
# main()
# -
if __name__ == '__main__':
    _client = NgClient()
    print(f"{_client.get_ifilters()}")
    print(f"{_client.get_gfilters()}")
