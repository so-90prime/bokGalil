#!/usr/bin/env python3
# -*- coding: utf-8 -*-


# +
# import(s)
# -
from typing import Optional

import argparse
import json
import urllib.request


# +
# __doc__
# -
__doc__ = """ python3 Galil_DMC_22x0_Read_Telemetry.py --help """


# +
# constant(s)
# -
JSON_KEYS = ['dome', 'mirror_cell', 'tube', 'telemetry', 'upper_dome', 'wind', 'weather']
JSON_TIMEOUT = 5.0
JSON_URL = "http://10.30.1.5:42080/latest"


# +
# class: TelemetryClient()
# -
class TelemetryClient(object):
    """
        from Galil_DMC_22x0_Read_Telemetry import *
        _c = TelemetryClient(url='http://10.30.1.5:42080/latest', timeout=5.0)
        _c.get_json()
        _dome = _c.parse_json(_key='dome')
        print(f"dome={_dome}, type={type(_dome)}")
        _mc = _c.parse_json(_key='mirror_cell')
        print(f"mirror_cell={_mc}, type={type(_mc)}")
        _tlm = _c.parse_json(_key='telemetry')
        print(f"telemetry={_tlm}, type={type(_tlm)}")
        _tube = _c.parse_json(_key='tube')
        print(f"tube={_tube}, type={type(_tube)}")
        _ud = _c.parse_json(_key='upper_dome')
        print(f"upper_dome={_ud}, type={type(_ud)}")
        _wind = _c.parse_json(_key='wind')
        print(f"wind={_wind}, type={type(_wind)}")
        _weather = _c.parse_json(_key='weather')
        print(f"weather={_weather}, type={type(_weather)}")
    """

    # +
    # method: __init__()
    # -
    def __init__(self, url: str = JSON_URL, timeout: float = JSON_TIMEOUT) -> None:

        # get input(s)
        self.url = url
        self.timeout = timeout

        # set variable(s)
        self.__error = None
        self.__jdata = None
        self.__rdata = None
        self.__udata = None

    # +
    # property(s)
    # -
    @property
    def url(self):
        return self.__url

    @url.setter
    def url(self, url: str = JSON_URL) -> None:
        self.__url = url if url.strip() != '' else JSON_URL

    @property
    def timeout(self):
        return self.__timeout

    @timeout.setter
    def timeout(self, timeout: float = JSON_TIMEOUT) -> None:
        self.__timeout = timeout if timeout > 0.0 else JSON_TIMEOUT

    # +
    # getter(s)
    # -
    @property
    def error(self):
        return self.__error

    @property
    def jdata(self):
        return self.__jdata

    @property
    def rdata(self):
        return self.__rdata

    @property
    def udata(self):
        return self.__udata

    # +
    # method: get_json()
    # -
    def get_json(self) -> None:
        try:
            self.__udata = urllib.request.urlopen(url=self.__url, timeout=self.__timeout)
            if self.__udata.getcode() == 200:
                self.__rdata = self.__udata.read()
                self.__jdata = json.loads(self.__rdata)
        except Exception as _:
            self.__error = f'{_}'
            self.__jdata = None
            self.__rdata = None
            self.__udata = None

    # +
    # method: parse_json()
    # -
    def parse_json(self, _key: str = '') -> dict:
        if self.__jdata is not None and _key.lower() in JSON_KEYS:

            if _key.lower() == 'dome':
                _dome = {
                    'external_temperature': 'unknown',
                    'external_humidity': 'unknown',
                    'external_dewpoint': 'unknown',
                    'internal_temperature': 'unknown',
                    'internal_humidity': 'unknown',
                    'internal_dewpoint': 'unknown',
                    'control_room_temperature': 'unknown',
                    'utc': 'unknown',
                    'error': ''
                }
                try:
                    _base = self.__jdata['weather']['dome_outside']['data']
                    _dome = {**_dome, **{'external_temperature': _base['outside_temperature']}}
                    _dome = {**_dome, **{'external_humidity': _base['outside_humidity']}}
                    _dome = {**_dome, **{'external_dewpoint': _base['outside_dew_point']}}
                    _dome = {**_dome, **{'internal_temperature': _base['dome_temperature']}}
                    _dome = {**_dome, **{'internal_humidity': _base['dome_humidity']}}
                    _dome = {**_dome, **{'internal_dewpoint': _base['dome_dew_point']}}
                    _dome = {**_dome, **{'control_room_temperature': _base['control_room_temperature']}}
                    _dome = {**_dome, **{'utc': self.__jdata['weather']['dome_outside']['timestamp']}}
                except Exception as _e1:
                    _dome = {**_dome, **{'error': f'{_e1}'}}
                return _dome

            elif _key.lower() == 'mirror_cell':
                _mcell = {
                    'temperature': 'unknown',
                    'humidity': 'unknown',
                    'dewpoint': 'unknown',
                    'utc': 'unknown',
                    'error': ''
                }
                try:
                    _base = self.__jdata['weather']['mirror_cell']['data']
                    _mcell = {**_mcell, **{'temperature': _base['mirror_cell']['mirror_cell_temperature']}}
                    _mcell = {**_mcell, **{'humidity': _base['mirror_cell']['mirror_cell_humidity']}}
                    _mcell = {**_mcell, **{'dewpoint': _base['mirror_cell']['mirror_cell_dew_point']}}
                    _mcell = {**_mcell, **{'utc': self.__jdata['weather']['mirror_cell']['timestamp']}}
                except Exception as _e2:
                    _mcell = {**_mcell, **{'error': f'{_e2}'}}
                return _mcell

            elif _key.lower() == 'tube':
                _tube = {'utc': 'unknown', 'error': ''}
                try:
                    _tube = {**_tube, **{'utc': self.__jdata['weather']['tube']['timestamp']}}
                except Exception as _e3:
                    _tube = {**_tube, **{'error': f'{_e3}'}}
                return _tube

            elif _key.lower() == 'telemetry':
                _tel = {
                    'motion': 'unknown',
                    'ra': 'unknown',
                    'dec': 'unknown',
                    'ha': 'unknown',
                    'lst': 'unknown',
                    'el': 'unknown',
                    'az': 'unknown',
                    'airmass': 'unknown',
                    'motion_ra': 'unknown',
                    'motion_dec': 'unknown',
                    'error': ''
                }
                try:
                    _base = self.__jdata['telemetry']['legacy_tcs']['data']
                    _tel = {**_tel, **{'motion': _base['motion']}}
                    _tel = {**_tel, **{'ra': _base['ra']}}
                    _tel = {**_tel, **{'dec': _base['dec']}}
                    _tel = {**_tel, **{'ha': _base['ha']}}
                    _tel = {**_tel, **{'lst': _base['lst']}}
                    _tel = {**_tel, **{'el': _base['el']}}
                    _tel = {**_tel, **{'az': _base['az']}}
                    _tel = {**_tel, **{'airmass': _base['airmass']}}
                    _tel = {**_tel, **{'motion_ra': _base['motion_ra']}}
                    _tel = {**_tel, **{'motion_dec': _base['motion_dec']}}
                    _tel = {**_tel, **{'utc': self.__jdata['telemetry']['legacy_tcs']['timestamp']}}
                except Exception as _e4:
                    _tel = {**_tel, **{'error': f'{_e4}'}}
                return _tel

            elif _key.lower() == 'upper_dome':
                _udome = {
                    'temperature': 'unknown',
                    'humidity': 'unknown',
                    'dewpoint': 'unknown',
                    'utc': 'unknown',
                    'error': ''
                }
                try:
                    _base = self.__jdata['weather']['upper_dome']['data']
                    _udome = {**_udome, **{'temperature': _base['upper_dome']['upper_dome_temperature']}}
                    _udome = {**_udome, **{'humidity': _base['upper_dome']['upper_dome_humidity']}}
                    _udome = {**_udome, **{'dewpoint': _base['upper_dome']['upper_dome_dew_point']}}
                    _udome = {**_udome, **{'utc': self.__jdata['weather']['upper_dome']['timestamp']}}
                except Exception as _e5:
                    _udome = {**_udome, **{'error': f'{_e5}'}}
                return _udome

            elif _key.lower() == 'wind':
                _wind = {
                    'speed': 'unknown',
                    'direction': 'unknown',
                    'gust_speed': 'unknown',
                    'gust_direction': 'unknown',
                    'utc': 'unknown',
                    'error': ''
                }
                try:
                    _base = self.__jdata['weather']['wind']['data']
                    _wind = {**_wind, **{'speed': _base['wind']['wind_speed']}}
                    _wind = {**_wind, **{'direction': _base['wind']['wind_direction']}}
                    _wind = {**_wind, **{'gust_speed': _base['wind']['gust_speed']}}
                    _wind = {**_wind, **{'gust_direction': _base['wind']['gust_direction']}}
                    _wind = {**_wind, **{'utc': self.__jdata['weather']['wind']['timestamp']}}
                except Exception as _e6:
                    _wind = {**_wind, **{'error': f'{_e6}'}}
                return _wind

            elif _key.lower() == 'weather':
                _d = self.parse_json(_key='dome')
                _m = self.parse_json(_key='mirror_cell')
                _t = self.parse_json(_key='tube')
                _u = self.parse_json(_key='upper_dome')
                _w = self.parse_json(_key='wind')
                return {'dome': _d, 'mirror_cell': _m, 'tube': _t, 'upper_dome': _u, 'wind': _w}
        return {}


# +
# function: get_weather_json()
# -
def get_weather_json(_url: str = JSON_URL, _timeout: float = JSON_TIMEOUT, _key: str = JSON_KEYS[0]) -> Optional[dict]:

    # exercise command(s) and request(s)
    try:
        _client = TelemetryClient(url=_url, timeout=_timeout)
        if _client is not None and hasattr(_client, 'get_json') and \
                hasattr(_client, 'parse_json') and _key.lower() in JSON_KEYS:
            _client.get_json()
            return _client.parse_json(_key=_key)
    except Exception as _e:
        print(f"failed to instantiate TelemetryClient(url={_url}, timeout={_timeout}), error='{_e}'")


# +
# main()
# -
if __name__ == '__main__':

    # get command line argument(s)
    _p = argparse.ArgumentParser(description='Galil_DMC_22x0_Read_Telemetry',
                                 formatter_class=argparse.RawTextHelpFormatter)
    _p.add_argument('--url', default=f"{JSON_URL}", help="""Host, default = %(default)s""")
    _p.add_argument('--timeout', default=JSON_TIMEOUT, help="""Timeout, (s) default = %(default)s""")
    _p.add_argument('--key', default=JSON_KEYS[-1], help=f"""Key, default = %(default)s, choice of {JSON_KEYS}""")
    _args = _p.parse_args()

    # noinspection PyBroadException
    try:
        print(f"{get_weather_json(_url=_args.url, _timeout=float(_args.timeout), _key=_args.key.lower())}")
    except Exception as _:
        print(f"{_}\nUse: {__doc__}")
