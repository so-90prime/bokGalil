#!/usr/bin/env python3
# -*- coding: utf-8 -*-


# +
# import(s)
# -
from astropy.time import Time
from datetime import datetime
from datetime import timedelta
from functools import wraps

import hashlib
import logging
import logging.config
import math
import os
import pytz
import re
import random
import time


# +
# initialize
# -
random.seed(os.getpid())


# +
# constant(s)
# -
BOOLEAN_VALUES = {'false': False, 'f': False, 'F': False, '0': False, 'true': True, 't': True, 'T': True, '1': True}
IERS_URL = 'https://datacenter.iers.org/data/9/finals2000A.all'
IERS_URL_ALTERNATE = 'ftp://cddis.gsfc.nasa.gov/pub/products/iers/finals2000A.all'
LOG_CLR_FMT = '%(log_color)s%(asctime)-20s %(levelname)-9s %(filename)-15s line:%(lineno)-5d %(message)s'
LOG_CSL_FMT = '%(asctime)-20s %(levelname)-9s %(filename)-15s line:%(lineno)-5d %(message)s'
LOG_FIL_FMT = '%(asctime)-20s %(levelname)-9s %(filename)-15s line:%(lineno)-5d %(message)s'
LOG_LEVELS = ['DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL']
LOG_MAX_BYTES = 9223372036854775807
MAXINT = 9223372036854775807
SUPPORTED_COLORS = ['black', 'blue', 'cyan', 'green', 'magenta', 'red', 'yellow']
TIMEZONE = pytz.timezone('America/Phoenix')
UTC_OFFSET = datetime.now(TIMEZONE).utcoffset().total_seconds()/60.0/60.0


# +
# pattern(s)
# -
# noinspection PyPep8
IP_PATTERN = '^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$'
IP_PATTERN_C = re.compile(IP_PATTERN)
ISO_PATTERN = '(1[89][0-9]{2}|2[0-9]{3})-(0[13578]-[012][0-9]|0[13578]-3[0-1]|' \
              '1[02]-[012][0-9]|1[02]-3[0-1]|02-[012][0-9]|0[469]-[012][0-9]|' \
              '0[469]-30|11-[012][0-9]|11-30)[ T](0[0-9]|1[0-9]|2[0-3]):[0-5][0-9]:[0-5][0-9](\.[0-9]*)?'
ISO_PATTERN_C = re.compile(ISO_PATTERN)


# +
# class: Logger()
# -
class Logger(object):

    # +
    # method: __init__
    # -
    def __init__(self, name: str = f"{os.getenv('USER', 'None')}", level: str = LOG_LEVELS[0]):

        # get arguments(s)
        self.name = name
        self.level = level

        # set default(s)
        self.__logdir = None
        self.__logdict = None
        self.__logfile = None

        # define some variables and initialize them
        self.__logdir = os.getenv("PWD", "/tmp")
        if self.__logdir is None or not os.path.exists(self.__logdir) or not os.access(self.__logdir, os.W_OK):
            self.__logdir = os.getcwd()
        self.__logfile = f'{self.__logdir}/{self.__name}.log'

        # logger dictionary
        self.__logdict = {

            # logging version
            'version': 1,

            # do not disable any existing loggers
            'disable_existing_loggers': False,

            # use the same formatter for everything
            'formatters': {
                'ObsColoredFormatter': {
                    '()': 'colorlog.ColoredFormatter',
                    'format': LOG_CLR_FMT,
                    'log_colors': {
                        'DEBUG': 'cyan',
                        'INFO': 'green',
                        'WARNING': 'yellow',
                        'ERROR': 'red',
                        'CRITICAL': 'white,bg_red',
                    }
                },
                'ObsConsoleFormatter': {
                    'format': LOG_CSL_FMT
                },
                'ObsFileFormatter': {
                    'format': LOG_FIL_FMT
                }
            },

            # define file and console handlers
            'handlers': {
                'colored': {
                    'class': 'logging.StreamHandler',
                    'formatter': 'ObsColoredFormatter',
                    'level': self.__level,
                },
                'console': {
                    'class': 'logging.StreamHandler',
                    'formatter': 'ObsConsoleFormatter',
                    'level': self.__level,
                    'stream': 'ext://sys.stdout'
                },
                'file': {
                    'backupCount': 10,
                    'class': 'logging.handlers.RotatingFileHandler',
                    'formatter': 'ObsFileFormatter',
                    'filename': self.__logfile,
                    'level': self.__level,
                    'maxBytes': LOG_MAX_BYTES
                }
            },

            # make this logger use file and console handlers
            'loggers': {
                self.__name: {
                    'handlers': ['colored', 'file'],
                    'level': self.__level,
                    'propagate': True
                }
            }
        }

        # configure logger
        logging.config.dictConfig(self.__logdict)

        # get logger
        self.logger = logging.getLogger(self.__name)

    # +
    # decorator(s)
    # -
    @property
    def name(self):
        return self.__name

    @name.setter
    def name(self, name: str) -> None:
        self.__name = name if (isinstance(name, str) and name.strip() != '') else os.getenv('USER')

    @property
    def level(self):
        return self.__level

    @level.setter
    def level(self, level: str) -> None:
        self.__level = level.upper() if \
            (isinstance(level, str) and level.strip() != '' and level.upper() in LOG_LEVELS) else LOG_LEVELS[0]

    @property
    def logdir(self):
        return self.__logdir

    @property
    def logdict(self):
        return self.__logdict

    @property
    def logfile(self):
        return self.__logfile


# +
# function: bok_print()
# -
def bok_print(msg: str = '', color: str = 'black', height: int = 1) -> None:
    """ prints message in single or double height in color (does not work on all screens) """

    _msg = msg if (isinstance(msg, str) and msg != '') else ''
    _clr = color.strip().lower() if (isinstance(color, str) and
                                     color.lower() in SUPPORTED_COLORS) else SUPPORTED_COLORS[0]
    _height = height if (isinstance(height, int) and height > 0) else 1

    if _msg != '':
        # print single height
        if _height == 1:
            if _clr == 'black':
                print(f"\033[0;30m{_msg}\033[0m")
            elif _clr == 'blue':
                print(f"\033[0;34m{_msg}\033[0m")
            elif _clr == 'cyan':
                print(f"\033[0;36m{_msg}\033[0m")
            elif _clr == 'green':
                print(f"\033[0;32m{_msg}\033[0m")
            elif _clr == 'magenta':
                print(f"\033[0;35m{_msg}\033[0m")
            elif _clr == 'red':
                print(f"\033[0;31m{_msg}\033[0m")
            elif _clr == 'yellow':
                print(f"\033[0;33m{_msg}\033[0m")
        # print double height
        elif _height == 2:
            if _clr == 'black':
                print(f"\033[0;30m\033#3{_msg}\n\033#4{_msg}\033[0m")
            elif _clr == 'blue':
                print(f"\033[0;34m\033#3{_msg}\n\033#4{_msg}\033[0m")
            elif _clr == 'cyan':
                print(f"\033[0;36m\033#3{_msg}\n\033#4{_msg}\033[0m")
            elif _clr == 'green':
                print(f"\033[0;32m\033#3{_msg}\n\033#4{_msg}\033[0m")
            elif _clr == 'magenta':
                print(f"\033[0;35m\033#3{_msg}\n\033#4{_msg}\033[0m")
            if _clr == 'red':
                print(f"\033[0;31m\033#3{_msg}\n\033#4{_msg}\033[0m")
            elif _clr == 'yellow':
                print(f"\033[0;33m\033#3{_msg}\n\033#4{_msg}\033[0m")
        # passthru
        else:
            print(f"{_msg}")


# +
# function: clear_bitn()
# -
def clear_bitn(data: int = 0, bit: int = 0) -> int:
    """ returns data with bit n cleared """
    return data & ~(1 << bit)


# +
# function: get_hash()
# -
def get_hash(seed: str = '') -> str:
    """ return unique 64-character string """
    _seed = seed.strip() if (isinstance(seed, str) and seed.strip() != '') else datetime.now().isoformat()
    return hashlib.sha256(_seed.encode('utf-8')).hexdigest()


# +
# function: get_iers_1():
# -
# noinspection PyBroadException
def get_iers_1() -> None:
    try:
        from astroplan import download_IERS_A
        download_IERS_A()
    except Exception:
        pass


# +
# function: get_iers_2():
# -
# noinspection PyBroadException
def get_iers_2() -> None:
    try:
        from astroplan import download_IERS_A
        from astropy.utils import iers
        from astropy.utils.data import clear_download_cache
        clear_download_cache()
        iers.IERS_A_URL = f'{IERS_URL_ALTERNATE}'
        download_IERS_A()
    except Exception:
        pass


# +
# function: get_iers():
# -
# noinspection PyBroadException
def get_iers() -> None:
    try:
        get_iers_2()
    except Exception:
        get_iers_1()


# +
# function: get_isot()
# -
# noinspection PyBroadException
def get_isot(day_offset: int = 0, utc: bool = False) -> str:
    """ return date in isot format for any day_offset or '' """
    if not isinstance(day_offset, int) or not isinstance(utc, bool):
        return ''
    try:
        return (datetime.utcnow() + timedelta(days=day_offset)).isoformat() if utc else \
            (datetime.now() + timedelta(days=day_offset)).isoformat()
    except Exception:
        return ''


# +
# function: get_jd()
# -
# noinspection PyBroadException
def get_jd(day_offset: int = 0, utc: bool = False) -> float:
    """ return date in jd format for any day_offset offset or nan """
    if not isinstance(day_offset, int) or not isinstance(utc, bool):
        return math.nan
    try:
        _date = get_isot(day_offset=day_offset, utc=utc)
        return Time(_date).jd if (re.match(ISO_PATTERN_C, _date) is not None) else math.nan
    except Exception:
        return math.nan


# +
# function: is_bitn_set()
# -
def is_bitn_set(data: int = 0, bit: int = 0) -> bool:
    """ returns True if bit n is set """
    return True if (data & (1 << bit)) else False


# +
# function: isot_to_jd()
# -
# noinspection PyBroadException
def isot_to_jd(isot: str = '') -> float:
    """ returns jd from isot date string or nan """
    try:
        return Time(isot).jd if (re.match(ISO_PATTERN_C, isot) is not None) else math.nan
    except Exception:
        return math.nan


# +
# function: jd_to_isot()
# -
# noinspection PyBroadException
def jd_to_isot(jd: float = math.nan) -> str:
    """ return isot from jd or '' """
    if not isinstance(jd, float) or abs(jd) is math.nan:
        return ''
    try:
        return Time(jd, format='jd', precision=6).isot if abs(jd) is not math.nan else ''
    except Exception:
        return ''


# +
# function: msleep()
# msleep: lambda _: time.sleep(_/1000.0)
# -
def msleep(_seconds: int = 1):
    time.sleep(_seconds / 1000.0)


# +
# function: set_bitn()
# -
def set_bitn(data: int = 0, bit: int = 0) -> int:
    """ returns data with bit n set """
    return data | (1 << bit)


# +
# decorator: timeit()
# -
def timeit(_infunc):
    @wraps(_infunc)
    def timed(*args, **kw):
        _ts = time.time()
        output = _infunc(*args, **kw)
        _te = time.time()
        _elapsed = (_te - _ts) * 1000.0
        print(f"'{_infunc.__name__}' took {_elapsed:.3f} ms to execute")
        return output
    return timed


# +
# function: usleep()
# usleep: lambda _: time.sleep(_/1000000.0)
# -
def usleep(_seconds: int = 1):
    time.sleep(_seconds / 1000000.0)


# +
# function: verify_dict()
# -
def verify_dict(_dict: dict = None, _types: tuple = None) -> bool:
    """ verify dictionary contains data types in tuple """
    return all(isinstance(_v, _types) for _k, _v in _dict.items())


# +
# function: verify_template()
# -
# noinspection PyTypeHints
def verify_template(_dict: dict = None, _temp: dict = None) -> bool:
    """ verify dictionary contains data types in template """
    return all(isinstance(_dict[_key], _temp[_key]) for _key in _dict)


# +
# function: verify_keys()
# -
def verify_keys(_dict: dict = None, _keys: list = None) -> bool:
    """ verify dictionary contains all key(s) specified """
    return all(_k in _dict for _k in _keys if _dict is not {})
