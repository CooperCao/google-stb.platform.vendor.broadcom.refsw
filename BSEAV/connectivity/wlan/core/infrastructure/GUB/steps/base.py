"""The base class for all Steps."""

from __future__ import print_function
from __future__ import unicode_literals

import itertools
import os
import shutil
import stat
import sys
import time

import lib.opts
import lib.scm
import lib.log
import lib.util


# This is a thread-safe (but not fork-safe) way of counting.
REPLAY_COUNTER = itertools.count(1).next
STEP_COUNTER = itertools.count(1).next
XMODE = stat.S_IRWXU | stat.S_IRGRP | stat.S_IROTH


class Step(object):

    """Base class for all Steps."""

    # TODO what is the difference between self.cfg and self.build.cfg
    # and why do they produce different results (in DHDAP router builds
    # at least)? Simplify.
    def __init__(self, cfg, name=None, build=None, incr=True):
        self.cfg = cfg
        self.name = (name if name else self.__class__.__name__[:-4])
        self.build = build
        self.passed = True
        self.duration = -1
        self.watcher = None
        if incr:
            self.seqno = STEP_COUNTER()

    def __getattr__(self, attr):
        """Pass unrecognized queries to the build object, if any."""
        if self.build:
            return getattr(self.build, attr)
        else:
            return None

    @staticmethod
    def lookup(cfg, name, build, **kwargs):
        """Find and instantiate a Step class by name."""
        # Map the step name to a conventional module path such
        # that "Mixed_Case_Name" becomes "steps.custom.lower.case.name".
        modname = cfg.get('use', name)
        tail = modname.replace('_', '.').lower()
        modpath = 'steps.' + tail

        # If not already present, try importing the step module
        # dynamically, falling back to DefaultStep.
        if modpath not in sys.modules:
            try:
                __import__(modpath)
            except ImportError:
                modpath = 'steps.custom.' + tail
                try:
                    __import__(modpath)
                except ImportError:
                    modpath = 'steps.default'
                    modname = 'Default'
        this = sys.modules[modpath]
        clsname = modname.replace('_', '') + 'Step'
        cls = getattr(this, clsname, None)
        lib.util.assert_(cls, 'no class "%s" found' % clsname)
        obj = cls(cfg, name=name, build=build, **kwargs)
        return obj

    def sub(self, data, tidy=True, final=False):
        """Replace Config variables in provided data."""
        return self.cfg.sub(data, tidy=tidy, final=final)

    def getvar(self, name, default=None, required=False):
        """Look up a variable at this node and return the value."""
        return self.build.cfg.getvar(name, default=default, required=required)

    def getbool(self, ref, default=None):
        """Look up a variable and return its value as a boolean."""
        return self.build.cfg.getbool(ref, default=default)

    def invoke(self, logs=None):
        """Run the step."""
        if logs:
            self.watcher = lib.log.Watcher(summary=self.name,
                                           logs=logs,
                                           timestamp=True,
                                           full_stdout=self.build.full_stdout)

        started_at = time.time()

        self.run()

        self.duration = int(time.time() - started_at + 0.5)

        return self.passed

    def execute(self, cmd, cwd=None, dosubs=False, env=None, extra_env=None,
                pfx='+', replay=False, stdout=None, vl=1):
        """Run a step command."""
        if not stdout:
            sys.stdout.write('\n')

        nenv = (env.copy() if env else os.environ.copy())
        if self.build:
            # These could possibly have a value of None which is why we don't
            # use .update(). "None" is illegal in an actual environ array.
            # In particular this can happen with "-d /dev/null".
            for ev in self.build.env:
                if self.build.env[ev] is not None:
                    nenv[ev] = '%s' % self.build.env[ev]
        nenv.update(self.cfg.exports(final=True))
        if extra_env:
            nenv.update(extra_env)

        timeout = int(self.cfg.getvar('STEP_TIMEOUT', 0))

        if dosubs:
            cmd = self.sub(cmd)

        if replay:
            script = 'replay-%s-%d.%d.%d.sh' % (self.name, self.seqno,
                                                os.getpid(), REPLAY_COUNTER())
            f = open(script, 'w')
            f.write('#!/bin/bash\n\n')
            f.write('# Run ./%s to retry\n' % script)
            f.write('(\n')
            f.write('exec </dev/null\n')
            f.write('[[ $0 != */* ]] || cd "${0%/*}" || exit 2\n')
            refdir = self.bldwd
            if cwd:
                scriptwd = os.path.relpath(cwd, refdir)
            else:
                scriptwd = os.path.relpath(os.getcwd(), refdir)
            if scriptwd != '.':
                f.write('cd "%s" || exit 2\n' % scriptwd)

            cmdline = lib.util.cmdline(cmd)
            if stdout and isinstance(stdout, basestring):
                cmdline += ' > ' + stdout

            for e in sorted(nenv.keys()):
                if (e not in os.environ or
                        nenv[e] != os.environ[e] or e == 'PATH'):
                    if e == 'SHELLOPTS' or e == 'BASHOPTS':
                        cmdline = "env %s='%s' %s" % (e, nenv[e], cmdline)
                    else:
                        f.write("export %s='%s'\n" % (e, nenv[e]))

            f.write(cmdline + '\n')
            f.write(')\n')
            lib.util.xtrace(cmdline, cwd=cwd, pfx=pfx, vl=vl)
            f.close()

            os.chmod(script, XMODE)
            rc = lib.util.execute(['/bin/bash', script], cwd=self.bldwd,
                                  timeout=timeout, vl=-1)
            if self.replays:
                # This should be synchronized in case of parallel jobs.
                shutil.copyfileobj(open(script, 'rb'),
                                   open(self.replays, 'ab'))
                os.chmod(self.replays, XMODE)
            if rc == 0:
                lib.util.rm(script, vl=2)
        else:
            if stdout and isinstance(stdout, basestring):
                if cwd:
                    stdout = open(os.path.join(cwd, stdout), 'w')
                else:
                    stdout = open(stdout, 'w')
            rc = lib.util.execute(cmd, cwd=cwd, env=nenv, stdout=stdout,
                                  timeout=timeout, vl=vl, pfx=pfx)

        if rc:
            if stdout == lib.util.PIPE:
                return rc
            else:
                self.passed = False
                return False
        else:
            return True

    def __del__(self):
        if self.watcher:
            self.watcher.status = (0 if self.passed else 1)
            del self.watcher


def _mark(stepname, logfiles, event, sep=False):
    """Insert a MARK-START or MARK-END line into the logfile(s)."""
    if logfiles:
        fobjs = [open(lf, 'a') for lf in logfiles]
    else:
        fobjs = [sys.stdout]

    t = time.strftime('[%m/%d/%y %H:%M:%S]', time.localtime())
    for f in fobjs:
        f.write('%s MARK-%s: %s\n' % (t, event, stepname))
        if sep:
            f.write('\n')


def mark_start(stepname, logfiles):
    """Insert a MARK-START line into the logfile(s)."""
    _mark(stepname, logfiles, 'START', sep=False)


def mark_end(stepname, logfiles):
    """Insert a MARK-END line into the logfile(s)."""
    _mark(stepname, logfiles, 'END', sep=True)

# vim: ts=8:sw=4:tw=80:et:
