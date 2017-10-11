"""
Derive a reasonable amount of make parallelism for the current host.

EXAMPLES:

Placed in .bash_profile or similar:

    export MAKEJOBS=$$(%(prog)s makejobs --gmake)

Setup for that other shell, with custom job multiplier:

    setenv MAKEJOBS `%(prog)s makejobs --jx 1.5`

"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import sys

import lib.consts
import lib.util


def parse_cli(prog, alias, subcmds, _, usage):
    """Standard subcommand command line option definitions."""
    parser = subcmds.add_parser(
        alias,
        epilog=usage,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        prog=prog)
    parser.add_argument(
        '-g', '--gmake', action='store_true',
        help="use '-j <jobs> -l <load>' format for GNU make")
    parser.add_argument(
        '--jx', type=float, default=None,
        help="override the default jobs multiplier")
    parser.add_argument(
        '--lx', type=float, default=None,
        help="override the default load multiplier")
    parser.add_argument(
        '-m', '--maxjobs', type=int, default=8,
        help="maximum number of jobs allowed (default=%(default)s)")
    parser.set_defaults(func=call)


def call(opts, cfgproxy):
    """Standard subcommand entry point."""
    if opts.jx:
        jx = opts.jx
    else:
        cfgroot = cfgproxy.parse()
        jx = float(cfgroot.getvar('LSF_JOBS_MULTIPLIER', 2.0))

    jval, lval = lib.util.makejobs(jx=jx, lx=opts.lx, jobmax=opts.maxjobs)

    if opts.gmake:
        sys.stdout.write('-j %d' % jval)
        if lval:
            sys.stdout.write(' -l %d' % lval)
        sys.stdout.write('\n')
    else:
        print(jval, lval)


# vim: ts=8:sw=4:tw=80:et:
