"""
Update a tree of SCM (svn or git) checkouts.

Operates on the entire tree, not just the current checkout.

Note that 'up' and 'update' are supported as synonyms for 'sync'.

EXAMPLES:

To update the checkout tree in DIR:

    %(prog)s scm sync DIR

To update the current checkout tree:

    %(prog)s scm up

To do the same using up to 8 parallel jobs:

    %(prog)s scm up -j8

To update quietly and show resulting status in one command:

    %(prog)s scm up -qs

"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import os
import sys

import lib.consts
import lib.pop
import lib.scm
import lib.times
import lib.util

import hnd.scm.stat

NO_UPDATE = '_NO_UPDATE'


def parse_cli(prog, alias, subcmds, _, usage):
    """Standard subcommand command line option definitions."""
    parser = subcmds.add_parser(
        alias,
        epilog=usage,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        prog=prog)
    parser.add_argument(
        '--accept',
        metavar='ARG',
        help="pass this option to 'svn update'")
    parser.add_argument(
        '-a', '--all', action='store_true',
        help="pass this flag to a --status check")
    parser.add_argument(
        '-j', '--jobs', type=int, default=4,
        metavar='N',
        help="allow up to N parallel update jobs (default=%(default)s)")
    parser.add_argument(
        '-q', '--quiet', action='store_true',
        help="suppress unnecessary output")
    parser.add_argument(
        '-r', '--reftime',
        metavar='TIME',
        help="peg floating URLs to this time (or revision)")
    parser.add_argument(
        '-s', '--status', action='store_true',
        help="do a 'status' check after the update")
    parser.add_argument(
        '--svn-up-option', default=[], action='append',
        help="pass this option along to svn update")
    parser.add_argument('basedirs', nargs=argparse.REMAINDER)
    parser.set_defaults(func=call)


def call(opts, _):
    """Standard subcommand entry point."""
    rc = 0

    basedirs = (opts.basedirs if opts.basedirs else ['.'])
    for i, basedir in enumerate(basedirs):
        if os.path.exists(basedir):
            basedirs[i] = lib.scm.find_tree_base(basedir)
            if not basedirs[i]:
                lib.util.error('unable to find basedir of ' + basedir)
                rc += 1
        else:
            lib.util.error('no such directory: %s' % basedir)
            rc += 1

    if rc != 0:
        sys.exit(2)

    reftime = lib.times.timeref(opts.reftime)

    svnupopts = opts.svn_up_option
    if opts.accept:
        svnupopts.append('--accept=%s' % opts.accept)

    for basedir in basedirs:
        if os.path.exists(os.path.join(basedir, NO_UPDATE)):
            lib.util.warn('updates disallowed: %s' % basedir)
            rc += 1
            continue
        tree = lib.pop.Tree(basedir=basedir)
        tree.sync(jobs=opts.jobs, reftime=reftime, svnupopts=svnupopts)
        if not tree.success:
            rc += 1

    if opts.status and rc == 0:
        lib.util.note('status check ...', vl=0)
        hnd.scm.stat.report(basedirs, opt_all=opts.all,
                            opt_jobs=opts.jobs, opt_quiet=opts.quiet)

    sys.exit(2 if rc else 0)

# vim: ts=8:sw=4:tw=80:et:
