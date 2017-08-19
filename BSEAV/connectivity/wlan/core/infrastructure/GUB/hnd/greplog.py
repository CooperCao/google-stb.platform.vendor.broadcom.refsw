"""
Look for likely error patterns in specified logfile(s).

Nearby error messages may be "bridged" by including intervening lines
of context.  The maximum number of bridging lines is controlled by the
-b flag. For example, if an error is found on line 330 and another on
line 334, and if the bridging value is at least 4, then lines 330-334
will be shown in a single block.

A change in directory (as indicated by GNU make 'Entering' and 'Leaving'
lines) will always end the bridging. In other words an error context
never spans multiple directories or make processes.

In the great majority of cases the last error message shown is the only
real, or fatal, one and previous messages are spurious. Therefore the
'--last 1' option is recommended. In addition, since fatal errors have
a strong tendency to cluster at the end of a logfile, the --tail option
will show the last N lines whether they match or not.

The way this works is by applying a wide sieve RE to catch anything
resembling a potential error message. That's the "blacklist" which is
used to round up the usual suspects. Then we apply another "whitelist" RE
to throw away lines which are not really errors.  Anything caught by the
blacklist and not by the whitelist is treated as an error message. The
philosophy is to over- rather than under-report; developers can figure
out which are not errors and ideally help add them to the whitelist.

The script attempts to track the current working directory by counting
the GNU Make "Entering" and "Leaving" messages. This derived CWD is
shown above each set of errors found.

EXAMPLES:

To look through a specific logfile:

    %(prog)s greplog /projects/hnd_swbuild/.../,release.log

To limit bridging to 2 lines:

    %(prog)s greplog -b 2 /projects/hnd_swbuild/.../,release.log
"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import collections
import gzip
import os
import re
import sys

import lib.consts
import lib.opts
import lib.util

# For CI purposes we try to distingish between infrastructure and coding
# errors in the exit status.
NO_ERROR = 0
INFRA_ERROR = 3
BUILD_ERROR = 4


def parse_cli(prog, alias, subcmds, advanced, usage):
    """Standard subcommand command line option definitions."""
    parser = subcmds.add_parser(
        alias,
        epilog=usage,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        prog=prog)
    parser.add_argument(
        '-b', '--bridge-len', type=int, default=8,
        metavar='LINES',
        help="number of lines bridging two errors for context (%(default)s)")
    parser.add_argument(
        '-c', '--chunk-max', type=int, default=30,
        help="maximum number of lines printed per chunk (%(default)s)")
    parser.add_argument(
        '-l', '--last', type=int, default=0,
        metavar='N',
        help="show only the last N error messages")
    parser.add_argument(
        '--no-linenums', action='store_true',
        help="suppress showing of line numbers on each logfile line"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '--no-logtimes', action='store_true',
        help="suppress showing of timestamps on each logfile line"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-o', '--outfile',
        metavar='FILE',
        help="save output to this file (stdout)")
    parser.add_argument(
        '-s', '--simple-format', action='count', default=0,
        help="print a simplified format for use by tools")
    parser.add_argument(
        '-t', '--tail', type=int, default=0,
        metavar='N',
        help="show the last N lines of each logfile")
    parser.add_argument('logfiles', nargs=argparse.REMAINDER)
    parser.set_defaults(func=call)

BAR_ = '================================================='
BAR = BAR_ + '--' + BAR_
RE_FLAGS = re.IGNORECASE

# One feature of find_build_errors not yet implemented here is the
# ability to identify a large set of similar errors and report only a
# representative version. Not sure that's really a feature. Maybe it's
# simpler and clearer to just report all the errors we find; if that's
# a lot, oh well, try not to break it so badly next time.

# Assume these are all hard errors until asserted otherwise.
# Patterns in the blacklist should be as *GENERAL* as possible,
# i.e. "No such file or directory" not "foo.h: No such file or directory".
# The idea is to catch anything that looks like an error here and
# throw it back via the whitelist if it isn't really.
# *ALSO* - an important principle is to avoid enumeration of individual
# messages as much as possible, which entails formatting messages under
# our own control in a standard format. For instance, every single error
# thrown by GUB itself matches the pattern "/: Error: /", so a single RE
# will match any error message, present or future. The list below should
# generally be extended only for messages NOT under our control. Our own
# messages should be made to fit one of the existing patterns.
RE_BLACK_LIST = [
    # The most common explicit error message.
    r"""Error: """,

    # And another very common one.
    r""": No such file or directory""",

    # The GNU make convention for all hard errors is " *** " but
    # we shouldn't handle recipe errors here, only DAG and other
    # make-internal errors, because if a recipe fails the
    # real message comes from it while subsequent make messages
    # contribute only context.
    r""": [*]{3} No targets specified and no makefile found.  Stop.""",
    r""": [*]{3} No rule to make target .*""",
    r""": [*]{3} missing separator.  Stop.""",
    r""": [*]{3} .* does not exist in .*.  Stop.""",
    r""": [*]{3} unterminated call to function.*\.  Stop.""",

    # Microsoft style message (MSVC, etc).
    r""": (?:fatal )?error [A-Z]+\d+ *:""",

    # Perl syntax error.
    r""" syntax error at """,

    # Bash: unable to find specified child command via PATH.
    r""": command not found$""",

    # Bash syntax error.
    r""": syntax error near [`']""",

    # From GNU tar.
    r""": file changed as we read it""",

    # Generic Unix.
    r"""core dumped""",

    # Perl missing module error.
    r"""Can't locate \S+ in @INC""",

    # Individual path component limit (a risk in FW builds).
    r"""File name too long""",

    # GNU linker error.
    r""": undefined reference to [`']\S+'""",

    # Another GNU linker error.
    r""": defined in discarded section""",

    # Apparently from gcc (seen in android builds).
    r"""error, forbidden warning: """,

    # Seen in FreeBSD cross builds on Linux.
    r"""cc1?: warnings being treated as errors""",

    # Seen from make invoking /bin/sh. Probably a limit of bash, not ARG_MAX.
    r"""/bin/sh: Argument list too long""",

    # From either cmd.exe or nmake. Remarkably, this is all it says.
    r"""The command line is too long""",

    # Seen once in a trunk linux-external-dongle-pcie build.
    r"""/lib64/libc.so.\d+: version `GLIBC_[\d.]+' not found""",

    # Nonstandard error message from GNU linker.
    r"""ld: error in """,
]

RE_BLACK_LIST_INFRA = [
    # Cygwin error, rarely seen since Cygwin added auto-rebase.
    r"""fork: Resource temporarily unavailable""",

    # Seen on Windows sometimes from unidentified program (link.exe?).
    r"""Access is denied\.$""",

    # LSF errors.
    r"""Failed in an LSF library call: """,
    r"""<<Terminated while pending>>""",
    r"""Job not submitted\.""",

    # Android specific - adding to this list until race is fixed.
    r"""get_hash.h: No such file or directory""",

    # Flag signtool errors as infra errors.
    r"""SignTool Error:""",
    r"""bcmretrycmd: (\d*) retries of""",

    # Flag connection errors as infra errors.
    r"""Connection timed out""",

    # Flag certificate errors as infra errors.
    r"""No certificates were found that met all the given criteria""",

    # All svn client errors should match this pattern.
    r"""svn: E\d+:""",

    # AFAIK this occurs when a windows build is waiting
    # for an HDW build to finish. I will keep a close eye on this error
    # and if this is due to a coding error,
    # will revert. Currently, it points to an INFRA error.
    r"""ERROR: ALL_DNGL_IMAGES list empty. Can't continue""",

    # HDW build dependency is reporting these failures in CI
    # We need to fix the underlying logic for HDW.
    # most likely not a coding error and error will be monitored.
    r"""Empty RELEASE_DNGL_LIST file found""",

    # HDW build dependency is reporting these failures in CI
    # We need to fix the underlying logic for HDW.
    # most likely not a coding error and error will be monitored.
    r"""dongle_image_temp_build_spawned: No such file or directory""",

    # HDW build dependency is reporting these failures in CI
    # We need to fix the underlying logic for HDW.
    # most likely not a coding error and error will be monitored.
    r"""replay-Brand_Makefile-.*Segmentation fault.*(core dumped)""",

    # We see this error when a disk partition is at 100%.
    r"""No space left on device""",

    # This occurs when there's a physical issue with a disk drive.
    # Sometimes a disk-full situation triggers this too.
    r"""Input/output Error""",

    # Generally a transient infra issue.
    r"""Device or resource busy""",

    # This occurs on windows boxes due to a cywin daemon issue.
    r"""Not enough server storage is available to process this command""",

    # Python exception. Not necessarily infra but more likely than not.
    # The second line is to extend context through the traceback.
    r"""Traceback \(most recent call last\)""",
    r"""File ".*", line \d+, in \w+""",

    # Killed by outside agent.
    r"""terminated by signal """,

    # Android hiccup.
    r"""fatal: Could not read from remote repository""",

    # This is a problem caused in 8/2016 by IT installing a bin2c
    # program in /tools/bin which is incompatible with ours. Once
    # the incompatibility is ironed out this will probably never be
    # needed again and can be removed. TODO.
    r"""bin2c: No output files are named""",

    # Connection failure message from ssh.
    r"""Permission denied \(publickey,""",  # \) to keep editor happy
]

# Patterns in the whitelist should be as *SPECIFIC* as possible.
# If we know that "No such file or directory" isn't an error
# in a particular situation, try to capture that exact situation
# via an RE here such as "FooBar: No such file or directory".
RE_WHITE_LIST = [
    # This is what gmake prints when not initially finding
    # and then generating a makefile/include file.
    # It's not an error message but looks a lot like one.
    r"""^\S+:\d+: \S+: No such file or directory$""",

    # Some of our makefile recipes have explicit error messages
    # like 'if [[ whatever]]; then echo "ERROR: something"; fi'
    # which are not protected by @, so the recipe itself can
    # end up in the build log and be mistaken for an error
    # even if the echo was never actually executed.
    r"""echo .?ERROR:""",

    # This file is generated so is not always present.
    # If it's missing when really needed we'll get a hard error.
    r"""Warning: No such file or directory: \S+/wlc_clm_data.c""",

    # This seems to be a spurious dongle build error.
    # Maybe it would be better to suppress it at makefile level?
    r"""cat: roml.qt: No such file or directory""",

    # A typical spurious error message in Windows builds.
    # It would have been better to hide this but the author didn't.
    r"""grep: build_solution_bcm.log: No such file or directory""",

    # Something in the dongle FW building infrastructure emits these
    # messages which are not actually errors. It should be fixed there.
    r"""ls: cannot access """
    r"""\d+\S+/(?:bcm|rtecdc)[^/]*\.(?:bin|dis|exe|h|map|nvm|trx): """
    """No such file or directory""",

    # Related to the problem directly above. A spurious message from
    # dongle builds which ought to be suppressed at the source.
    r"""/bin/sh: line 1: cd: /build/dongle: No such file or directory"""
]


class Frame(object):

    """Hold a sliding window containing one or more error messages."""

    RESULTS = []

    def __init__(self, cwd, linenums=True):
        self.cwd = cwd
        self.linenums = linenums
        self.infra = False
        self.lines = []
        self.bridge = []

    def __repr__(self):
        msg = '  * [%s]\n\n' % self.cwd
        for line in self.lines + self.bridge:
            if self.linenums:
                msg += '%d: ' % line[0]
            msg += line[1] + '\n'
        return msg

    def add(self, lineno, line):
        """Add a new line to the error output."""
        if self.bridge:
            self.lines.extend(self.bridge)
            self.bridge = []
        self.lines.append((lineno, line))

    def context(self, lineno, line):
        """Add a line of bridging context."""
        if line.endswith(' Error 1 (ignored)'):
            # If make is explicitly ignoring this error, we can too.
            self.erase()
            return False
        else:
            self.bridge.append((lineno, line))
            return True

    def dump(self):
        """Print and flush the current state."""
        if self.lines:
            msg = repr(self)
            Frame.RESULTS.append((msg, self.infra))
            self.lines = []
            self.bridge = []

    def erase(self):
        """Flush the current state without printing."""
        self.lines = []
        self.bridge = []

    def healthy(self):
        """Return True if not in error state."""
        return len(self.lines) == 0

    @property
    def lnum(self):
        """Return the current line number in this frame."""
        return self.lines[-1][0] if self.lines else 0

    def __del__(self):
        self.dump()


def call(opts, _):
    """Standard subcommand entry point."""
    rc = NO_ERROR
    outfile = open(opts.outfile, 'w') if opts.outfile else sys.stdout

    black_re_infra = '|'.join(RE_BLACK_LIST_INFRA)
    black_re = '|'.join(RE_BLACK_LIST)
    black_re += '|' + black_re_infra
    white_re = '|'.join(RE_WHITE_LIST)

    # Since these logfiles can be big, we try to process them a line at a
    # time without reading the whole thing into memory. This is why we
    # avoid readlines(), enumerate(), etc.

    if not opts.logfiles:
        opts.logfiles = ['-']

    for log in opts.logfiles:
        lnum = 0

        pubwd = bldwd = None

        # Optionally show the last N lines of the file since that's
        # where the error usually is.
        tail = collections.deque()

        # The only way to know the CWD within the build is if it tells us.
        # Generally this is with GNU make "Entering" and "Leaving" lines.
        # Since the make "Leaving" message doesn't say what directory it's
        # returning to, we need to keep a directory stack.

        if log == '-':
            log = '/dev/stdin'
            dirstack = ['<NONE>']
            logf = sys.stdin
        else:
            if os.path.isdir(log):
                log = os.path.join(log, lib.util.RELEASE_LOG_FILE)
            dirstack = [os.path.dirname(os.path.abspath(log))]
            if log.endswith('.gz'):
                logf = gzip.open(log, 'r')
            else:
                logf = open(log, 'r')

        # Note that every time we jump to a new frame the previous one
        # will automatically dump its error state, if any.
        frame = Frame(dirstack[0], linenums=not opts.no_linenums)

        for line in logf:
            line = line.decode('utf-8', 'replace').rstrip()
            if line == BAR:
                break

            lnum += 1

            if opts.tail > 0:
                tail.append((lnum, line))
                if len(tail) > opts.tail:
                    if line.startswith('STARTED:'):
                        opts.tail = -opts.tail
                        tail.pop()
                    else:
                        tail.popleft()

            if not opts.no_logtimes:
                line = re.sub(r'^[\d:\s]*', '', line)

            if line.startswith('BUILD_DIR:'):
                pubwd = line.split()[1]
                continue
            elif line.startswith('LOCAL_DIR:'):
                bldwd = line.split()[1]
                continue

            # Track log cwd via GNU make 'Entering' and 'Leaving' lines.
            if 'Entering directory ' in line or 'Leaving directory ' in line:
                ndir = line.split('ing directory ', 1)[-1]
                if 'Entering directory ' in line:
                    # Some platforms (android) seem to use `GNU quoting'.
                    ndir = ndir.replace("`", "'").split("'")[1]
                    if pubwd and bldwd:
                        ndir = ndir.replace(bldwd, pubwd, 1)
                    dirstack.append(ndir)
                else:
                    dirstack.pop()
                frame = Frame(dirstack[-1], linenums=not opts.no_linenums)

            # If this line doesn't match the blacklist we can dispose
            # of it right away. Exception: if in error state already
            # we may want to collect context.
            if not re.search(black_re, line, RE_FLAGS):
                if not frame.healthy():
                    if lnum - frame.lnum < opts.bridge_len:
                        if pubwd and bldwd:
                            line = line.replace(bldwd, pubwd)
                        frame.context(lnum, line)
                    else:
                        frame = Frame(frame.cwd, linenums=not opts.no_linenums)
                continue

            # Limit the number of error messages printed. If a build
            # process tries to ls or tar 32,000 nonexistent files we
            # don't want 32,000 messages in the summary.
            if len(frame.lines) >= opts.chunk_max:
                break

            # If current state is "healthy" then a suspect line needs to
            # get past both RE's to put us in "error" state. But once
            # in error state, ignore the whitelist to collect all
            # suspicious data till we get back to a healthy stretch.
            if not frame.healthy():
                if pubwd and bldwd:
                    line = line.replace(bldwd, pubwd)
                frame.add(lnum, line)
            elif not re.search(white_re, line, RE_FLAGS):
                if pubwd and bldwd:
                    line = line.replace(bldwd, pubwd)
                frame.add(lnum, line)
                if re.search(black_re_infra, line, RE_FLAGS):
                    frame.infra = True

        if logf != sys.stdin:
            logf.close()

        # Clear the top-level frame.
        del frame

        # We always derive the return code from the first error
        # found, even if it's not the first error reported.
        # The rationale is that we can't trust the result if the
        # infrastructure broke down. So if a coding error was
        # preceded by an infra error we ignore the coding error
        # and treat it as infra, whereas if a coding error
        # precedes an infra error the coding error remains valid.
        if rc == NO_ERROR:
            if Frame.RESULTS and Frame.RESULTS[0][1]:
                rc = INFRA_ERROR
            else:
                rc = BUILD_ERROR

        if lib.opts.VERBOSITY > 0:
            if opts.last:
                del Frame.RESULTS[:len(Frame.RESULTS) - opts.last]

            if not opts.simple_format:
                # Print header with name of logfile.
                outfile.write('%s\n%s %s\n%s\n\n' % (BAR, BAR[:3], log, BAR))

            for i, result in enumerate(Frame.RESULTS):
                errnum = '' if opts.last == 1 else ' %d' % (i + 1)
                outfile.write('\n  * ---- ERROR%s ---- *\n' % errnum)
                outfile.write(result[0])

            if opts.tail:
                outfile.write('\n  * ---- LAST %d LINES ---- *\n\n' %
                              abs(opts.tail))
                for lineno, line in tail:
                    outfile.write('%d: %s\n' % (lineno, line))

    sys.exit(rc)

# vim: ts=8:sw=4:tw=80:et:
