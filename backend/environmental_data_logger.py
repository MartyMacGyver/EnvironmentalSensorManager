#!/usr/bin/env python2

#   Copyright (c) 2016 Martin F. Falatic
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

"""
Reading data from a Particle-based environmental monitoring project

"""

from __future__ import print_function

import re
import sys
from contextlib import contextmanager
import os
import time
import json
import signal
from datetime import datetime
# import base64
import argparse
import requests
import dateutil.parser
import dateutil.zoneinfo
import dateutil.tz
from sseclient import SSEClient


PROGNAME = os.path.basename(sys.argv[0])
SCREEN_WIDTH = 78
TIMEZONE = dateutil.tz.tzlocal()  # dateutil.zoneinfo.gettz('America/Los_Angeles')

CO2_BASELINE_PPM = 400
RETRY_WAIT_PERIOD = 5

SIGNUMS_TO_NAMES = dict((getattr(signal, n), n) \
    for n in dir(signal) if n.startswith('SIG') and '_' not in n)
SIGNAMES_TO_NUMS = dict((v, k) for k, v in SIGNUMS_TO_NAMES.items())

API_ROOT = "https://api.particle.io/v1/devices/events/"

#            ("2016-05-16T01:32:09.712000-07:00  76.46  47.30 1240   0x01D900F7D1 0x2104D8FD")
DATA_HEADER = "# Date(ISO8601)                   T(F)   RH(%) CO2ppm RHT_raw      CO2_raw"


@contextmanager
def _flexout_open(filename, mode='Ur'):
    if filename == None or filename == '':
        fhandle = sys.stdout
    else:
        fhandle = open(filename, mode)
    try:
        yield fhandle
    finally:
        if not (filename is None or filename == ''):
            fhandle.close()


def signals_init():
    '''Initialize signal handlers'''
    initted_signals = []
    for sig in ['SIGINT', 'SIGTERM', 'SIGBREAK']:
        if sig in SIGNAMES_TO_NUMS:
            signal.signal(SIGNAMES_TO_NUMS[sig], signals_handler)
            initted_signals.append(sig)
    print("-- Initialized signal handlers: {}".format(' '.join(initted_signals)))
    # print('Press Ctrl+C')
    # signal.pause()


def signals_handler(sig, frame):
    '''Signals handler'''
    print('\n-- Received signal {} ({}) - exiting\n'.format(sig, SIGNUMS_TO_NAMES[sig]))
    sys.exit(0)


def process_message(msg, msg_num, co2prev):
    '''Process an individual message'''
    parsed = {}
    try:
        parsed = json.loads(msg.data)
    except ValueError:
        parsed['data'] = ''
        return co2prev
    # print(msg_num, type(msg.data), msg.data, parsed['data'])
    date = str(parsed['published_at'])
    iso_date = dateutil.parser.parse(date).astimezone(TIMEZONE).isoformat()
    marker = ''
    data_regex = r'^T=(\d+\.\d+)\s*F\sRH=(\d+\.\d+)\s*\%\sCO2=(\d+)\s*ppm RHTRAW=(.*) CO2RAW=(.*)$'
    match = re.match(data_regex, parsed['data'])
    if match:
        temp = float(match.group(1))
        rhum = float(match.group(2))
        co2ppm = int(match.group(3))
        rhtraw = match.group(4)
        co2raw = match.group(5)  # base64.b16decode(match.group(5))
        if co2ppm == 0:
            co2ppm = co2prev
        else:
            co2prev = co2ppm
        if rhum > 100.0:
            marker = '! '
        data_field = "{:6.2f} {:6.2f} {:6d} 0x{:s} 0x{:s}".\
                     format(temp, rhum, co2ppm, rhtraw, co2raw)
    else:
        marker = '! '
        data_field = parsed['data']
    with _flexout_open(output_file, 'a') as fstream:
        print("{:s}{:s} {:s}".format(marker, iso_date, data_field), file=fstream)
        fstream.flush()
    return co2prev


def run_message_loop(api_url):
    '''The main processing loop '''
    co2prev = CO2_BASELINE_PPM
    msg_num = 1
    error_str = ''
    try:
        messages = SSEClient(api_url, timeout=5)
        for msg in messages:
            msg_num += 1
            returned_data = process_message(msg=msg, msg_num=msg_num, co2prev=co2prev)
            if returned_data is not None:
                (co2prev) = returned_data
    except requests.exceptions.Timeout as err:
        error_str = repr(err)
    except requests.exceptions.TooManyRedirects as err:
        error_str = repr(err)
    except requests.exceptions.RequestException as err:
        error_str = repr(err)
    else:
        error_str = "Unexpected exit of main loop"
    return error_str


if __name__ == "__main__":
    print('-'*SCREEN_WIDTH)
    print("-- Using Python {}".format(sys.version))
    signals_init()
    print()

    argparser = argparse.ArgumentParser(prog=PROGNAME,
                                        usage='%(prog)s [options]')
    argparser.add_argument('--datasource', action='store', required=True,
                           dest='data_source', metavar='<name>',
                           help='API data source name')
    argparser.add_argument('--token', action='store', required=True,
                           dest='access_token', metavar='<token>',
                           help='API access token')
    argparser.add_argument('--outputfile', action='store', required=False,
                           dest='output_file', metavar='<file>',
                           help='output file name (else stdout)')
    args = argparser.parse_args(sys.argv[1:])

    data_source = args.data_source
    access_token = args.access_token
    output_file = args.output_file

    print('-- Output_file = {}'.format(output_file))
    print()

    api_url = API_ROOT+data_source+'?access_token='+access_token

    with _flexout_open(output_file, 'a') as fstream:
        print(DATA_HEADER, file=fstream)

    while True:
        errstr = run_message_loop(api_url=api_url)
        raw_date = str(datetime.now(dateutil.tz.tzlocal()))
        iso_date = dateutil.parser.parse(raw_date).astimezone(dateutil.tz.tzlocal()).isoformat()
        with _flexout_open(output_file, 'a') as fstream:
            marker = '! '
            print("{:s}{:s} Error: {:s}".format(marker, iso_date, errstr), file=fstream)
            fstream.flush()
        print('-'*SCREEN_WIDTH)
        print(errstr)
        print('-'*SCREEN_WIDTH)
        time.sleep(RETRY_WAIT_PERIOD)
