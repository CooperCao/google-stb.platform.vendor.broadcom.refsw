#!/bin/env python

# ****************************************************************************
#
#  Author:     Sudhir Sangappa
#  Date  :     November 17, 2011
#
#  Purpose:    Wrapper tool for any tool until it succeeds
#
#
#  Optional EnvironmentVariables:
#              BCMRETRYCMD_ITER  (Tool default is 8 iterations)
#              BCMRETRYCMD_DELAY_SECONDS (Tool default is ITER**2)
#
# *****************************************************************************

"""Usage: bcmretrycmd command ...

Wrapper to retry any command until it succeeds or max tries exceeded.

Input:
   Any tool with arguments/options.

Output:
   input tool's stdoutdata and stderrdata.

Optional Environment Variables:
   BCMRETRYCMD_ITER  (Tool default is 8 iterations)
   BCMRETRYCMD_DELAY_SECONDS (Tool default is ITER**2)

"""

import math
import os
import sys
import traceback
import time
import subprocess

__version__ = '$Id$'
VERSION = 'Version: 1.0, %s\n' % __version__.replace('$', '')
LOGFILE = None

DEFAULT_TRIES = 8
DEFAULT_DELAY = 2


def log(msg):
    """Dump a log message."""
    f = sys.stderr
    if LOGFILE:
        f = open(LOGFILE, 'a')
    f.write(str(msg))
    if LOGFILE:
        f.close()


def quote_arg(arg):
    """Protect a word from the shell."""
    if ' ' in arg:
        return '"%s"' % arg
    return arg


def run_cmd(cmd, env=None):
    """Run command, return its standard output."""

    if 'BCMRETRYCMD_VERBOSE' in os.environ:
        print >> sys.stderr, '+ ' + cmd

    p = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE, env=env, shell=True)
    (stdoutdata, stderrdata) = p.communicate()
    return p.returncode, stdoutdata, stderrdata


def main(argv):
    """Entry point."""
    if len(argv) < 2 or argv[1] == '-h' or argv[1] == '--help':
        print __doc__.strip()
        sys.exit(0)

    tries = int(os.environ.get('BCMRETRYCMD_ITER', DEFAULT_TRIES))
    delay = float(os.environ.get('BCMRETRYCMD_DELAY_SECONDS', DEFAULT_DELAY))

    n = 0
    rc = None
    stddata_all = []

    cmd = ' '.join([quote_arg(arg) for arg in sys.argv[1:]])

    try:
        for n in range(tries):
            time.sleep(math.pow(n, delay))

            rc, stdoutdata, stderrdata = run_cmd(cmd)
            if rc == 0:
                log(stdoutdata)
                log(stderrdata)
                break

            # No need to print the same message n times
            thisfail = stdoutdata + stderrdata
            if thisfail not in stddata_all:
                stddata_all.append(thisfail)
    except Exception:
        log('Unhandled Error in Bcmretrycmd Wrapper')
        log(''.join(traceback.format_exception(*sys.exc_info())))
        return 1

    if rc:
        log('\nERROR: bcmretrycmd: %d retries of "%s"\n\n' % (n, cmd))
        log(' '.join(stddata_all))
    elif n > 1:
        log('\nWARN: bcmretrycmd: %d retries of "%s"\n\n' % (n, cmd))

    return rc

if __name__ == '__main__':
    sys.exit(main(sys.argv))
