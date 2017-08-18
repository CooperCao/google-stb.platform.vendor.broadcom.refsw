"""
Apply the named patch(es) to the subset of files which exist locally.

EXAMPLES:

Apply misc/Build.patch to the current directory tree:

    %(prog)s scm apply misc/Build.patch

Just see what would happen (dry run):

    %(prog)s -N scm apply misc/Build.patch
"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import sys

import lib.consts
import lib.patch
import lib.util


def parse_cli(prog, alias, subcmds, _, usage):
    """Standard subcommand command line option definitions."""
    parser = subcmds.add_parser(
        alias,
        epilog=usage,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        prog=prog)
    parser.add_argument(
        '--dig', type=int, default=0,
        help="look this many levels deep for a named file")
    parser.add_argument(
        '-i', '--interactive', action='store_true',
        help="prompt before each patch attempt")
    parser.add_argument('patch', nargs=argparse.REMAINDER)
    parser.set_defaults(func=call)


def call(opts, _):
    """Standard subcommand entry point."""
    rc = 0
    for patch in opts.patch:
        if lib.patch.applypatch(patch, dig=opts.dig, prompt=opts.interactive):
            rc = 2
    if rc:
        sys.exit(rc)

# vim: ts=8:sw=4:tw=80:et:
