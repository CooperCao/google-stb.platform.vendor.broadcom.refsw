"""
Produce a patch from the specified tree.

EXAMPLES:

Print a diff from the current working directory.

    %(prog)s scm diff

Print a diff from the specified subdirectories:

    %(prog)s scm diff dir1 dir2

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
        '--svn-option', default=[], action='append',
        help="pass this option along to svn")
    parser.add_argument('dirs', nargs=argparse.REMAINDER)
    parser.set_defaults(func=call)


def call(opts, _):
    """Standard subcommand entry point."""
    if not opts.dirs:
        base = lib.scm.find_tree_base()
        if base:
            lib.util.chdir(base, vl=2)
        opts.dirs = ['.']

    # The idea here is to normalize all patches to be against
    # the base of the checkout tree.
    rc = 0
    for d in opts.dirs:
        d = os.path.normpath(d)
        try:
            os.stat(d)
        except Exception as e:
            lib.util.error(str(e))
            rc += 1
            continue

        svns, gits = lib.scm.find_components(d, search_up=1)[0:2]

        for svn in svns:
            svn = svn if d == '.' else svn[len(d) + 1:]
            cmd = lib.scm.svncmd('diff', svn)
            for svn_option in reversed(opts.svn_option):
                cmd.insert(1, svn_option)
            if lib.util.execute(cmd, cwd=d, vl=2):
                rc += 1

        for git in gits:
            cmd = lib.scm.gitcmd('diff')
            if lib.util.execute(cmd, cwd=git, vl=2):
                rc += 1

    sys.exit(2 if rc else 0)

# vim: ts=8:sw=4:tw=80:et:
