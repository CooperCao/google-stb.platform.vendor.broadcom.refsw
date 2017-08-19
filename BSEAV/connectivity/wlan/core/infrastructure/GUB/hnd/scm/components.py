"""
Print a list of components comprising the named or current directory.

This can also run a command for each component's path if the -c flag
is used. If the command contains "{}" it will be replaced by the path
and invoked from the cwd. Otherwise the command is invoked after a cd
into the component path.

EXAMPLES:

Show the list of components in the current tree:

    %(prog)s scm components

Show the list of components in a different tree:

    %(prog)s scm components xx/yy/zz

Show components and their urls:

    %(prog)s scm components --full

Two ways of upgrading a checkout from an older svn version:

    %(prog)s scm components -c "svn upgrade"    # "cd foo/bar && svn upgrade"
    %(prog)s scm components -c "svn upgrade {}" # "svn upgrade foo/bar"

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
        '-c', '--cmd',
        help="run CMD for each component")
    parser.add_argument(
        '--full', action='store_true',
        help="show the mapping of local path to base url")
    parser.add_argument(
        '--urls', action='store_true',
        help="show the base url instead of the local path")
    parser.add_argument('dirs', nargs=argparse.REMAINDER)
    parser.set_defaults(func=call)


def call(opts, _):
    """Standard subcommand entry point."""
    if not opts.dirs:
        opts.dirs = ['.']

    rc = 0
    for d in opts.dirs:
        try:
            os.stat(d)
        except Exception as e:
            lib.util.error(str(e))
            rc += 1
            continue

        svns, gits = lib.scm.find_components(d, search_up=1)[0:2]

        if opts.cmd:
            for path in svns + gits:
                if '{}' in opts.cmd:
                    rc += lib.util.execute(opts.cmd.replace('{}', path),
                                           interactive=True, vl=2)
                else:
                    rc += lib.util.execute(opts.cmd, cwd=path,
                                           interactive=True, vl=2)
            sys.exit(2 if rc else 0)

        if opts.full:
            fmtlen = max([len(p) for p in svns]) + 1

        for path in svns:
            if opts.urls:
                print(lib.scm.urlof(path))
            elif opts.full:
                print('%-*s %s' % (fmtlen, path + ':', lib.scm.urlof(path)))
            else:
                print(path)

        for path in gits:
            print(path)

    sys.exit(2 if rc else 0)

# vim: ts=8:sw=4:tw=80:et:
