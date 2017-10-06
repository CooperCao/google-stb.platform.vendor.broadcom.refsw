"""
Analyze patterns of firmware size growth.

A tag name will be inferred from the directory path if not supplied.

EXAMPLES:

    %(prog)s featuresizing /projects/hnd_swbuild/.../2016.5.31.2

"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import os
import sys

import lib.consts
import lib.fsizing
import lib.opts
import lib.util


def parse_cli(prog, alias, subcmds, _, usage):
    """Standard subcommand command line option definitions."""
    parser = subcmds.add_parser(
        alias,
        epilog=usage,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        prog=prog)
    parser.add_argument(
        '-t', '--tag',
        help="branch/tag/twig to operate on")
    parser.add_argument('path')
    parser.set_defaults(func=call)


def call(opts, _):
    """Standard subcommand entry point."""
    if not opts.tag:
        opts.tag = os.path.basename(os.path.dirname(
            os.path.dirname(opts.path)))
    sys.exit(lib.fsizing.run(opts.path, opts.tag))

# vim: ts=8:sw=4:tw=80:et:
