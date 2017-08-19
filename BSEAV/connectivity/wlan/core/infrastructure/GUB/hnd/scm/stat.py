"""
Do an SCM status check within the specified tree.

Operates on the entire tree, not just the current checkout.

Note that 'st' and 'stat' are supported as synonyms for 'status'.

EXAMPLES:

From the root of the tree:

    %(prog)s scm stat

"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import os
import sys

import lib.consts
import lib.scm
import lib.util


def parse_cli(prog, alias, subcmds, _, usage):
    """Standard subcommand command line option definitions."""
    parser = subcmds.add_parser(
        alias,
        epilog=usage,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        prog=prog)
    parser.add_argument(
        '-a', '--all', '--ignored', '--no-ignore', action='store_true',
        help="show 'ignored' paths as well")
    parser.add_argument(
        '-j', '--jobs', type=int, default=3,
        metavar='N',
        help="allow up to N parallel update jobs (default=%(default)s)")
    parser.add_argument(
        '-p', '--privates', action='count', default=0,
        help="don't print versioned items")
    parser.add_argument(
        '-q', '--quiet', action='store_true',
        help="don't print unversioned items")
    parser.add_argument(
        '-s', '--simple-format', action='count', default=0,
        help="print a list of files, for use by tools")
    parser.add_argument(
        '-u', '--show-updates', action='store_true',
        help="display update information")
    parser.add_argument(
        '-v', '--verbose_status', action='store_true',
        help="print extra information")
    parser.add_argument(
        '--depth',
        metavar='ARG',
        help="limit operation by depth ARG")
    parser.add_argument('dirs', nargs=argparse.REMAINDER)
    parser.set_defaults(func=call)


def report(dirs, opt_all=False, opt_depth=0, opt_jobs=1,
           opt_privates=False, opt_quiet=False, opt_show_updates=False,
           opt_simple_format=False, opt_verbose_status=False):
    """Traverse component list showing status."""
    rc = 0

    svncmd = lib.scm.svncmd('stat')
    gitcmd = lib.scm.gitcmd('stat')

    if opt_all:
        svncmd.append('--no-ignore')
        gitcmd.append('--ignored')

    if opt_depth:
        svncmd.extend(['--depth', opt_depth])

    if opt_quiet:
        svncmd.append('-q')

    if opt_show_updates:
        svncmd.append('-u')

    if opt_verbose_status:
        svncmd.append('-v')

    def print_results(stdout):
        """Process svn status output into a readable report."""
        def printline(pfx, path):
            """Print a processed version of status output."""
            path = lib.util.shortpath(path)
            if os.path.isdir(path) and not os.path.islink(path):
                sys.stdout.write(''.join([pfx, path, os.sep, '\n']))
            else:
                sys.stdout.write(''.join([pfx, path, '\n']))

        for line in stdout:
            line = line.decode('utf-8').rstrip()
            if opt_show_updates:
                if line.startswith('Status against revision: '):
                    sys.stdout.write(line + '\n')
                elif line:
                    ix = line.rindex(' ') + 1
                    printline(line[:ix], line[ix:])
                continue

            if not line:
                continue

            if line.startswith('--- Changelist '):
                continue

            pfx, path = line[0:7], line[8:]
            if opt_privates and pfx[0] not in '?I~':
                continue

            # The > is a continuation line which contains no pathname.
            if path[0:2] == '> ':
                if opt_simple_format:
                    continue

            if opt_simple_format:
                if pfx[0] not in 'D!':
                    sys.stdout.write(lib.util.shortpath(path) + '\n')
            else:
                printline(pfx, path)

    for d in dirs:
        try:
            os.stat(d)
        except Exception as e:
            lib.util.error(str(e))
            rc += 1
            continue

        svns, gits, others = lib.scm.find_components(d, search_up=1)

        if gits:
            lib.util.warn('git support is TBD %s' % gits)

        cmds = [svncmd + [svn] for svn in svns]
        if lib.util.execute_parallel(cmds, func=print_results,
                                     jobs=opt_jobs, vl=2) != 0:
            rc = 2

        for other in others:
            path = lib.util.shortpath(other)
            if not opt_simple_format and (os.path.isdir(path) and
                                          not os.path.islink(path)):
                sys.stdout.write(''.join([path, os.sep, '\n']))
            else:
                sys.stdout.write(''.join([path, '\n']))

    return rc


def call(opts, _):
    """Standard subcommand entry point."""
    if not opts.dirs:
        base = lib.scm.find_tree_base()
        if base:
            opts.dirs = [base]
        else:
            lib.util.die('unable to find checkout tree')

    rc = report(opts.dirs, opt_all=opts.all, opt_depth=opts.depth,
                opt_jobs=opts.jobs,
                opt_privates=opts.privates, opt_quiet=opts.quiet,
                opt_show_updates=opts.show_updates,
                opt_simple_format=opts.simple_format,
                opt_verbose_status=opts.verbose_status)

    sys.exit(2 if rc else 0)

# vim: ts=8:sw=4:tw=80:et:
