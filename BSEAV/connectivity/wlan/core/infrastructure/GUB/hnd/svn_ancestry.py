"""
Trace the parent history of an SVN branch/tag/twig (aka copy).

EXAMPLES:

Find the parent branch of a tag:

  %(prog)s svn_ancestry -t BISON05T_REL_7_35_39

Go back 4 generations, ignoring intermediate static tags:

  %(prog)s svn_ancestry -t DIN155RC27_REL_9_83_5 -l4 -b

Trace a tag back to trunk with full paths and revisions:

  %(prog)s svn_ancestry -t BISON05T_REL_7_35_39 --all --details

"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import os

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
        '-a', '--all', '--trunk', action='store_true',
        help="trace ancestry all the way back to trunk")
    parser.add_argument(
        '-b', '--no-rel-tags', action='store_true',
        help="ignore static tags, show only branches and twigs")
    parser.add_argument(
        '-d', '--details', action='store_true',
        help="show full svn urls and revisions")
    parser.add_argument(
        '-l', '--limit', type=int, default=1,
        metavar='N',
        help="go back N generations")
    parser.add_argument(
        '-t', '--tag', default=[], action='append',
        help="name of (or path to) branch/tag/twig/trunk")
    parser.set_defaults(func=call)


def call(opts, cfgproxy):
    """Standard subcommand entry point."""
    cfgroot = cfgproxy.parse()

    tags = [lib.scm.svn_expand_tag(t, cfgroot=cfgroot)
            for t in lib.util.tolist(opts.tag, 'trunk')]
    for tag in tags:
        path, rev = (tag, None)
        while path and not path.endswith('/trunk'):
            path, rev, _, by, date = lib.scm.svn_tag_parent(path, rev)
            if path:
                name = os.path.basename(path)
                if opts.no_rel_tags and lib.scm.is_static(name):
                    continue
                if opts.details:
                    print('%s@%s %s %s' % (path, rev, by, date))
                else:
                    print(name)

            if not opts.all:
                opts.limit -= 1
                if opts.limit == 0:
                    break

# vim: ts=8:sw=4:tw=80:et:
