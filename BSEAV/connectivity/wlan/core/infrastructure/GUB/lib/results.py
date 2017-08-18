"""Look up results of past builds."""

from __future__ import print_function
from __future__ import unicode_literals

import datetime
import glob
import os
import pprint
import re
import sys
import time

import lib.bld
import lib.consts
import lib.opts
import lib.scm
import lib.times
import lib.util
import lib.wip

ERR_RE = r'Error \d+\s*$|xiting with failure status'

# If a build says it's still running but its output logfile
# is older than this many seconds, assume it's dead.
# This is primarily for old-style (BB) builds which do not
# have the heartbeat feature.
LOG_TIMEOUT = 60 * 60 * 8


class Commit(object):

    """Record and report data about a given commit."""

    def __init__(self, line):
        self.line = line.strip()
        self.user, revstr = self.line.split()[0:2]
        self.rev = int(revstr[1:])

    def __cmp__(self, other):
        if self.line < other.line:
            return -1
        elif self.line > other.line:
            return 1
        else:
            return 0

    def __hash__(self):
        return hash(self.line)

    def __repr__(self):
        return self.line


class Result(object):

    """Record and report data about a given build."""

    def __init__(self, bdir, name):
        self.bdir = bdir
        self.name = name

        self.bldable = os.path.basename(bdir)
        self.path = os.path.join(bdir, name)
        self.incr = int(self.name.split('.', 3)[-1])
        self.log = os.path.join(self.path, lib.util.RELEASE_LOG_FILE)
        self.error_log = os.path.join(self.path, lib.bld.BUILD_ERRORS_FILE)
        self.contents = os.path.join(self.path,
                                     lib.opts.METADIR,
                                     lib.consts.CONTENTS_HTML)
        self.dead = self.live = False
        try:
            self.bdata = lib.bld.BuildData(restore=self.path)
        except Exception:
            # TODO This is ugly compatibility code for reporting on old-style
            # (BB) builds which is intended to be removed eventually.
            # However, keep in mind some subset of this logic may be needed
            # for the case where a modern build dies so hard (power hit, etc)
            # that it can't write the post-mortem file.
            self.bdata = None
            wip = os.path.join(self.path, lib.wip.WIP_FILE)
            atmatch = glob.glob(
                os.path.join(self.path, lib.opts.METADIR, 'AT_*'))
            # Some brand makefiles implement this instead of lib.wip.WIP_FILE.
            partial = os.path.join(self.path,
                                   '_WARNING_PARTIAL_CONTENTS_DO_NOT_USE')
            placeholder = os.path.join(self.path, ',placeholder_release.log')
            self.start_time = sys.maxsize
            if os.path.isfile(os.path.join(self.path, lib.bld.SUCCEEDED_FILE)):
                self.succeeded = True
            elif os.path.isfile(self.error_log):
                self.succeeded = False
            elif os.path.isfile(wip) or \
                    os.path.isfile(partial) or \
                    os.path.isfile(placeholder):
                try:
                    # A modern build will have the WIP file updated every
                    # minute while running. Old-style builds require a
                    # different/longer timeout which is less reliable.
                    modern = os.path.exists(os.path.join(
                        self.path,
                        lib.opts.METADIR,
                        lib.bld.SUMMARY_FILE))
                    if modern and lib.wip.died(wip):
                        self.dead = True
                    elif time.time() - os.path.getmtime(self.log) > LOG_TIMEOUT:
                        self.dead = True
                    else:
                        self.live = True
                except OSError:
                    self.dead = True

                if atmatch:
                    # Modern builds should always have this flag file.
                    self.start_time = os.path.getmtime(atmatch[0])
                elif os.path.isfile(wip):
                    # BB compatibility.
                    self.start_time = os.path.getmtime(wip)
                elif os.path.isfile(placeholder):
                    # Other BB compatibility, maybe overkill.
                    self.start_time = os.path.getmtime(placeholder)
                else:
                    # Yet more BB compatibility, maybe overkill.
                    self.start_time = os.path.getmtime(partial)
            else:
                self.dead = True

            # Use sys.maxint to ensure bogus values never sort above good ones.
            try:
                self.start_time = os.path.getmtime(
                    os.path.join(self.path, lib.opts.METADIR, ',env.log'))
            except OSError:
                # The &@#$ dslcpe build is nonstandard.
                try:
                    self.start_time = os.path.getmtime(
                        os.path.join(self.path, ',env.log'))
                except OSError:
                    pass
            try:
                self.end_time = os.path.getmtime(self.log)
            except OSError:
                self.end_time = -sys.maxsize - 1
            if self.end_time > self.start_time:
                duration = int(self.end_time - self.start_time)
                self.elapsed = '%s' % datetime.timedelta(seconds=duration)
            else:
                self.elapsed = 'UNKNOWN'
            self.hostname = 'UNKNOWN'
            if os.path.isfile(self.log):
                for line in open(self.log, 'r'):
                    if line == '\n':
                        break
                    elif line.startswith('HOSTNAME'):
                        self.hostname = line.split()[-1].lower()

        self.blames = set()
        blamefile = os.path.join(self.path, lib.opts.METADIR, 'Blames.txt')
        if os.path.exists(blamefile):
            try:
                with open(blamefile) as f:
                    self.blames = set(f.read().splitlines())
            except OSError as e:
                lib.util.error(str(e))

    @property
    def passed(self):
        """True iff this build was successful."""
        if self.bdata:
            return self.bdata.succeeded
        elif self.dead or self.live:
            return False
        else:
            return not os.path.isfile(self.error_log)

    @property
    def failed(self):
        """True iff this build completed and was not successful."""
        return not (self.passed or self.dead or self.live)

    @property
    def state(self):
        """Return a string describing the state of this build."""
        if self.passed:
            return 'PASS'
        elif self.live:
            return 'LIVE'
        elif self.failed:
            return 'FAIL'
        elif self.dead:
            return 'DEAD'

    def strtime(self, timeval):  # pylint: disable=no-self-use
        """Stringify the provided time value."""
        try:
            return time.ctime(timeval)
        except Exception:
            return str(None)

    @property
    def started(self):
        """Return the starting time of this build."""
        return self.strtime(self.start_time)

    @property
    def ended(self):
        """Return the ending time of this build."""
        return self.strtime(self.end_time)

    def matches(self, atpath):
        """Return True iff the build is from the request rev-set."""
        return os.path.isfile(os.path.join(self.path, atpath))

    @property
    def errors(self):
        """Extract and return error messages from the log file."""
        errors = warnings = 0
        try:
            with open(self.log, 'r') as f:
                logdata = f.read().decode('utf-8')
        except IOError as e:
            lib.util.error('%s %s' % (self.log, e))
            errors += 1
        else:
            for line in logdata.splitlines():
                if re.search(ERR_RE, line):
                    errors += 1
                elif 'warning:' in line or 'Warning:' in line:
                    # Clock skew warnings aren't interesting since
                    # all builds are from scratch.
                    if 'has modification time ' in line:
                        continue
                    elif 'clock skew detected' in line:
                        continue
                    warnings += 1

        return errors, warnings

    @property
    def lsflog(self):
        """Return a path to the LSF log, if present."""
        if self.bdata:
            return self.bdata.lsflog
        else:
            path = os.path.join(self.path, lib.opts.METADIR, 'LSF.log')
            return path if os.path.isfile(path) else None

    def __getattr__(self, attr):
        """Pass unrecognized queries to the BuildData object."""
        if self.bdata:
            return getattr(self.bdata, attr)
        else:
            return None

    def __repr__(self):
        if self.bdata:
            merged = self.bdata.__dict__.get('restored', {}).copy()
        else:
            merged = {}
        merged.update(self.__dict__)
        merged.pop('bdata', None)
        return pprint.pformat(merged)


def by_incr(build):
    """Sort builds by their increment."""
    return int(build.split('.')[-1])


def find_results(cfgroot, tag, bldday=None, bldname=None, findall=False,
                 reftime=None):
    """Search the build area for build results to summarize."""
    if lib.scm.is_static(tag):
        findall = True
        bldday = r'\d+.\d+.\d+'
    elif not bldday:
        now = datetime.datetime.now()
        bldday = '%s.%s.%s' % (now.year, now.month, now.day)
    bldday_re = re.compile(bldday.replace('.', r'\.') + r'\.\d+$')

    if lib.times.is_offset(reftime):
        atpath = lib.bld.atpath(lib.times.timeref(reftime))
    else:
        atpath = None

    found = set()
    enabled = cfgroot.enabled_buildables(tag)
    if not enabled and not findall:
        lib.util.warn('no enabled buildable items for ' + tag)
        return found, []
    if bldname:
        enabled_set = set(lib.util.tolist(bldname))
    else:
        enabled_set = set(enabled if enabled else [])
    valid_re = re.compile(r'^[a-z]')
    latest = {}
    build_base = lib.consts.STATE.base
    for name in os.listdir(build_base):
        if not name.startswith('build_'):
            continue
        if name == 'build_window':
            continue
        plat_dir = os.path.join(build_base, name)
        tag_dir = os.path.join(plat_dir, tag)
        if not os.path.isdir(tag_dir):
            continue
        for bldname in os.listdir(tag_dir):
            if bldname not in enabled_set and not findall:
                continue
            if not re.match(valid_re, bldname):
                continue
            bldbase = os.path.join(tag_dir, bldname)
            if not os.path.isdir(bldbase):
                continue
            blds = [d for d in os.listdir(bldbase) if re.match(bldday_re, d)]
            for bld in sorted(blds, key=by_incr):
                # If given an atpath, use it as a filter.
                if atpath:
                    res = Result(bldbase, bld)
                    if not res.matches(atpath):
                        continue

                # Keep the latest remaining build of each buildable item.
                latest[bldbase] = bld

    for k, v in latest.items():
        res = Result(k, v)
        found.add(res)
        enabled_set.discard(res.bldable)

    if not enabled_set or lib.scm.is_static(tag):
        missing = enabled_set
    else:
        # Don't complain about missing builds that weren't scheduled.
        missing = set()
        today = time.strftime('%a', time.strptime(bldday, '%Y.%m.%d'))
        for name in enabled_set:
            bcfg = cfgroot.buildables().get(name, {})
            blddays = bcfg.get(lib.consts.NIGHTLY_DAYS)
            if not blddays or today in blddays.split(','):
                missing.add(name)

    return found, missing

# vim: ts=8:sw=4:tw=80:et:
