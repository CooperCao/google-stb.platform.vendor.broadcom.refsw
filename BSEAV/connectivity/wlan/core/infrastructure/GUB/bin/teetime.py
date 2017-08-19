#!/usr/bin/env python
"""
A tee-like program with optional decorated output and error search.

This is more or less compatible with GNU tee while adding
some enhancements which include:

- Optional prefixing of each line with a timestamp or line number.
- Optional printing of 'Started:' and 'Elapsed:' summary lines.
- The ability to search for error patterns in the output stream
  and report hits via its exit status.

EXAMPLES:

To use as a "tee" replacement:

  someprogram | %(prog)s logfile1 logfile2

To add timestamps and line numbers:

  someprogram | %(prog)s -d -n logfile1 logfile2

"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import datetime
import os
import re
import sys
import time

START_TIME = time.time()
STATUS = '_PARENT_STATUS_'


def main():
    """Entry point for standalone use."""
    parser = argparse.ArgumentParser(
        epilog=__doc__.strip(),
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument(
        '-a', '--append', action='store_true',
        help="append to the given files, do not overwrite")
    parser.add_argument(
        '-d', '--time-stamp', action='store_true',
        help="add a time stamp to each line")
    parser.add_argument(
        '-e', '--error-search', action='store_true',
        help="search lines for errors via RE")
    parser.add_argument(
        '-n', '--line-number', action='store_true',
        help="print line numbers with output lines")
    parser.add_argument(
        '-q', '--quiet', action='store_true',
        help="don't copy data to stdout")
    parser.add_argument(
        '-R', '--error-re',
        help="replace the default RE for finding errors")
    parser.add_argument(
        '-r', '--extra-error-re',
        help="extend the default RE for finding errors")
    parser.add_argument(
        '-s', '--summary',
        help="print start and elapsed time summaries")
    parser.add_argument(
        '-t', '--ref-time', type=float, default=START_TIME,
        help="make timestamp relative to this time")
    parser.add_argument('logfiles', nargs=argparse.REMAINDER)
    opts, _ = parser.parse_known_args()

    if not opts.error_re:
        opts.error_re = \
            r'''(?:
                \bError:\s|
                \bError 2\b|
                \sfatal\s
                )$'''

    if opts.extra_error_re:
        opts.error_re = '(?:%s|%s)' % (opts.error_re, opts.extra_error_re)

    error_re = re.compile(opts.error_re)

    ok = None
    rc = 0

    outputs = {}
    outputs['-stdout'] = sys.stdout
    mode = ('ab' if opts.append else 'wb')
    for fn in opts.logfiles:
        if fn.startswith('+'):
            outputs[fn] = open(fn[1:], 'ab')
        else:
            outputs[fn] = open(fn, mode)

    if opts.summary:
        ssecs = int(START_TIME - opts.ref_time + 0.5)
        sstr = '%s' % datetime.timedelta(seconds=ssecs)
        for fn in outputs:
            outputs[fn].write('%s: Started: +%s\n' % (opts.summary, sstr))

    # Set stdout to universal-newline mode to strip ^M from Windows outputs.
    sys.stdin = os.fdopen(sys.stdin.fileno(), 'rU', 0)

    lineno = 0
    timestamp = None
    total = 0.0
    exc = None
    continuation = False
    while True:
        line = sys.stdin.readline().decode('utf-8', 'replace')
        if not line:
            break

        if STATUS in line:
            if opts.summary:
                match = re.match(STATUS + r'=(\d+)', line)
                if match:
                    ok = (False if int(match.group(1)) else True)
                else:
                    sys.stderr.write('Error: bad status line: %s' % line)
            break

        if opts.error_search:
            if re.search(error_re, line):
                rc = 1

        lineno += 1

        if opts.time_stamp:
            total = time.time() - opts.ref_time
            timestamp = '%02d:%02d: ' % (total // 60, total % 60)

        for fn in outputs:
            if opts.quiet and fn.startswith('-'):
                continue

            # If errors are encountered on output, show the first one
            # but continue reading input and trying to write output.
            try:
                if opts.line_number:
                    outputs[fn].write('%d: ' % lineno)
                if timestamp:
                    if continuation:
                        if line != '\n':
                            outputs[fn].write(' ' * len(timestamp))
                    else:
                        outputs[fn].write(timestamp)
                outputs[fn].write(line.encode('utf-8', 'replace'))
                outputs[fn].flush()
            except EnvironmentError as e:
                if not exc:
                    exc = e
                    sys.stderr.write('%s\n' % e)

        continuation = line.endswith('\\\n')

    if opts.summary:
        esecs = int(time.time() - START_TIME + 0.5)
        elapsed = '%s' % datetime.timedelta(seconds=esecs)
        now = time.ctime()
        if ok:
            pf = ' [PASS]'
        elif ok is False:
            pf = ' [FAIL]'
        else:
            pf = ''
        estr = '%s: Elapsed: %s at %s%s' % (opts.summary, elapsed, now, pf)
        for fn in outputs:
            outputs[fn].write(estr + '\n')

    return rc

if __name__ == '__main__':
    sys.exit(main())

# vim: ts=8:sw=4:tw=80:et:
