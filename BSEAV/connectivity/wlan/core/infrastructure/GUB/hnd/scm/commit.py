"""
Commit all modified files in the specified tree.

Operates on the entire tree, not just the current checkout.

Note that 'ci' is supported as a synonym for 'checkin'.

EXAMPLES:

Commit the current tree with the specified message:

    %(prog)s scm ci -m "your message here"

Same thing but query for message:

    %(prog)s scm ci

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
        '-F', '--file-message',
        metavar='ARG',
        help="read log message from file ARG")
    parser.add_argument(
        '-m', '--message',
        help="specify log message")
    parser.add_argument('dirs', nargs=argparse.REMAINDER)
    parser.set_defaults(func=call)


def call(opts, _):
    """Standard subcommand entry point."""
    if not opts.dirs:
        lib.util.chdir(lib.scm.find_tree_base(), vl=2)
        opts.dirs = ['.']

    rc = 0
    for d in opts.dirs:
        try:
            os.stat(d)
        except Exception as e:
            lib.util.error(str(e))
            rc += 1
            continue

        for parent, dnames, _ in os.walk(d):
            if '.svn' in dnames or '.git' in dnames:
                if '.svn' in dnames:
                    cmd = lib.scm.svncmd('commit', parent)
                    cwd = None
                else:
                    cmd = lib.scm.gitcmd('commit')
                    cwd = parent

                if opts.message:
                    cmd.extend(['-m', opts.message])
                    interactive = False
                elif opts.file_message:
                    cmd.extend(['-F', opts.file_message])
                    interactive = False
                else:
                    interactive = True
                    if '--non-interactive' in cmd:
                        cmd.remove('--non-interactive')

                if lib.util.execute(cmd,
                                    cwd=cwd,
                                    interactive=interactive,
                                    vl=2):
                    rc += 1

                # Go no deeper.
                dnames[:] = []

    sys.exit(2 if rc else 0)

# vim: ts=8:sw=4:tw=80:et:
