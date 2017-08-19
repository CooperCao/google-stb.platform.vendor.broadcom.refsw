"""Operations related to time."""

from __future__ import print_function
from __future__ import unicode_literals

import datetime
import os
import subprocess
import sys
import time

# In order to avoid import loops, this module should
# not import non-core modules.
PROG = os.path.basename(sys.argv[0])

# All timestamps are normalized to "Broadcom Standard Time".
BRCM_TZ = 'America/Los_Angeles'

SVNREPOS = (
    'http://svn.sj.broadcom.com/svn/wlansvn',
    'http://svn.sj.broadcom.com/svn/ucodesvn',
)


def ctimez(seconds=None):
    """Same as time.ctime() but with timezone name appended."""
    return ' '.join([
        time.ctime(seconds),
        time.strftime('%Z', time.localtime(seconds))
    ])


def tzoff(ref):
    """Determine the numerical UTC timezone offset to Sunnyvale."""
    return 7 if (time.localtime(ref).tm_isdst > 0 and time.daylight) else 8


def is_offset(spec):
    """Interpret spec as an offset from most recent midnight?"""
    if spec and spec[0] in '+-':
        try:
            int(spec)
        except ValueError:
            pass
        else:
            return True
    return False


# Conversion of revision to timestamp is a bit controversial since
# it requires an assumption about which repo the rev refers to.
# The intent is to support conversion only for a transition period.
# We use --xml format to get microsecond precision. The result is
# incremented by one usec in order to select the original revision.
def svnrev2timestamp(rev, repo=SVNREPOS[0]):
    """Convert a revision in the supplied repo to an ISO8601 timestamp."""
    cmd = ['svn', 'info', '--xml']
    cmd.append('-r%s' % rev if rev else '-rHEAD')
    cmd.append(repo)
    stdout = subprocess.check_output(cmd)
    for line in stdout.splitlines():
        if line.startswith('<date>'):
            # Yes, this should really do actual XML parsing.
            date = line.split('<date>', 1)[1].split('</date>', 1)[0]
            dt = datetime.datetime.strptime(date, '%Y-%m-%dT%H:%M:%S.%fZ')
            dt += datetime.timedelta(microseconds=1)
            return dt.isoformat() + 'Z'


def posix2timestamp(posixtime):
    """Convert seconds-past-epoch to ISO8601."""
    dt = datetime.datetime.utcfromtimestamp(posixtime)
    return dt.strftime('%Y-%m-%dT%H:%M:%S.%fZ')


def timeref(spec, bracket=True, ref=None):
    """Interpret %-specifiers in time specs relative to previous midnight."""
    if ref is None:
        ref = time.time()
    else:
        ref = iso2seconds(ref)

    if spec and not spec.isdigit():
        if is_offset(spec):
            # Offset from midnight in integral hours. It may be negative
            # and extend over multiple days, thus +8 means 8AM today,
            # -12 means noon yesterday, and -72 is midnight 3+ days ago.
            delta = int(spec)
            ref += (delta // 24) * (60 * 60 * 24)
            rev = '%%Y-%%m-%%dT%02d:00:00-0%d00' % (delta % 24, tzoff(ref))
            if bracket:
                rev = '{%s}' % rev
        elif spec == 'HEAD':
            rev = spec
        elif spec[0] == '{' and spec[-1] == '}':
            if spec[1] == '@':
                rev = '{%s}' % posix2timestamp(float(spec[2:-1]))
            else:
                rev = spec
        elif bracket:
            rev = '{%s}' % spec
        else:
            rev = spec

        r = time.localtime(ref)
        since = (r.tm_year, r.tm_mon, r.tm_mday, 0, 0, 0,
                 r.tm_wday, r.tm_yday, r.tm_isdst)
    else:
        r = time.localtime(ref)
        if spec:
            rev = svnrev2timestamp(int(spec))
        else:
            # No spec was given. Set a reference point of "now".
            rev = '%%Y-%%m-%%dT%%H:%%M:%%S-0%d00' % tzoff(ref)
        since = [r.tm_year, r.tm_mon, r.tm_mday,
                 r.tm_hour, r.tm_min, r.tm_sec,
                 r.tm_wday, r.tm_yday, r.tm_isdst]
        if bracket:
            rev = '{%s}' % rev

    timestamp = time.strftime(rev, since)
    if spec and spec.isdigit():
        sys.stderr.write('%s: Warning: revisions are deprecated: r%s ' %
                         (os.path.basename(sys.argv[0]), spec))
        sys.stderr.write('converted to "%s"\n' % timestamp)
    return timestamp


def iso2seconds(date):
    """Convert some common date formats to seconds-since-epoch."""
    if date:
        if isinstance(date, float):
            result = date
        elif isinstance(date, int):
            result = float(date)
        elif date.count('.') == 3:
            # Convert a traditional yyyy.mm.dd.N build name to posix time.
            # This will be truncated to midnight.
            date = '.'.join(date.split('.')[0:3])
            result = time.mktime(time.strptime(date, '%Y.%m.%d'))
        else:
            date = date.lstrip('{').rstrip('}')
            if ' ' in date:
                fmt = '%Y-%m-%d %H:%M:%S'
                parts = date.split(' ')
                if len(parts) == 3:
                    fmt += ' ' + parts[-1]
                result = time.mktime(time.strptime(date, fmt))
            elif '-' in date:
                if 'T' in date:
                    if date[-1] == 'Z' and '.' in date:
                        dt = datetime.datetime.strptime(
                            date, '%Y-%m-%dT%H:%M:%S.%fZ')
                        delta = dt - datetime.datetime(1970, 1, 1)
                        result = delta.total_seconds()
                    else:
                        fmt = '%Y-%m-%dT%H:%M:%S'
                        parts = date.split('-')
                        if len(parts) == 4:
                            fmt += '-' + parts[-1]
                        result = time.mktime(time.strptime(date, fmt))
                else:
                    fmt = '%Y-%m-%d'
                    result = time.mktime(time.strptime(date, fmt))
            else:
                fmt = '%Y%m%dT%H%M%S'
                result = time.mktime(time.strptime(date, fmt))
    else:
        result = 0.0

    return result


def elapsed(since):
    """Print time elapsed since 'since'."""
    invoked = time.time()
    seconds = int(invoked - since + 0.5)
    delta = '%s' % datetime.timedelta(seconds=seconds)
    now = time.ctime(invoked)
    sys.stderr.write('%s: Elapsed: %s at %s\n' % (PROG, delta, now))

# vim: ts=8:sw=4:tw=80:et:
