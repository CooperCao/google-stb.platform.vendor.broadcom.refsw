"""
Compare firmware binaries from two different builds.

The set of images to be checked is derived by inspecting the left
hand path (lpath).  Therefore, order matters and the lpath should
be considered the control path.

The name of an image is considered to be its two parent directories,
e.g. '4355b3-roml/config_pcie_release'.

EXAMPLES:

From within a tag build area:

    %(prog)s fwcmp linux-olympic-dongle-src/*.0/package_check/build/dongle \
hndrte-dongle-wl/*.0/build/dongle

"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import sys

import lib.consts
import lib.fw
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
        '--forgive', default=[], action='append',
        help="ignore images containing this substring (may be repeated)")
    parser.add_argument('lpath')
    parser.add_argument('rpath')
    parser.set_defaults(func=call)


def call(opts, _):
    """Standard subcommand entry point."""
    sys.exit(lib.fw.compare(opts.lpath, opts.rpath, forgive=opts.forgive))

# vim: ts=8:sw=4:tw=80:et:
