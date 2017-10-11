"""
Convert an svn revision to the equivalent timestamp or vice versa.

Anything that looks like a revision will be converted to a timestamp.
Anything that looks like a timestamp will be converted to a revision.

This requires making an assumption about which repository the revisions
belongs to. The default is our original wlansvn repo but that can be
overridden with the -u flag.

Due to a subtlety in Subversion, timestamps are incremented by a
microsecond. The timestamp returned by an svn query for a given rev
actually refers to the instance _before_ the commit took place, and
thus passing that timestamp back to svn will result in the original rev
not being selected. Therefore timestamps are kept to the granularity
of usecs and .000001 is added to ensure that the instant _after_ the
commit is returned.

EXAMPLES:

Convert an svn revision to a legal svn timestamp:

    %(prog)s rev2time -r 599004

As above but leave out the brackets (which only svn wants):

    %(prog)s rev2time -s -r 599004

Return the timestamp in posix-time (seconds-past-epoch) format:

    %(prog)s rev2time -p -r 599004

Verify the algorithm by doing a round trip:

    %(prog)s rev2time -r $$(%(prog)s rev2time -r 599004)

"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse

import lib.consts
import lib.scm
import lib.times


def parse_cli(prog, alias, subcmds, _, usage):
    """Standard subcommand command line option definitions."""
    parser = subcmds.add_parser(
        alias,
        epilog=usage,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        prog=prog)
    parser.add_argument(
        '-a', '--all', action='store_true',
        help="print results for all known repos")
    parser.add_argument(
        '-p', '--posix-time', action='store_true',
        help="print posix time (seconds since epoch)")
    parser.add_argument(
        '-r', '--reftime', action='append', default=[],
        help="revision or timestamp")
    parser.add_argument(
        '-s', '--simple-format', action='count', default=0,
        help="print a general-purpose format")
    parser.add_argument(
        '-u', '--user-url', default=lib.consts.WL_REPO_ROOT,
        metavar='URL',
        help="translate revision via specified url")
    parser.add_argument('args', nargs='*')
    parser.set_defaults(func=call)


def call(opts, _):
    """Standard subcommand entry point."""
    if opts.all:
        repos = lib.times.SVNREPOS
    else:
        repos = (opts.user_url,)

    args = opts.reftime + opts.args
    if not args:
        args = [None]

    for arg in args:
        for repo in repos:
            if arg and arg[0] == '@':
                timestamp = lib.times.posix2timestamp(float(arg[1:]))
                arg = '{%s}' % timestamp
            elif arg and arg.startswith('{@') and arg.endswith('}'):
                timestamp = lib.times.posix2timestamp(float(arg[2:-1]))
                arg = '{%s}' % timestamp
            else:
                timestamp = lib.times.svnrev2timestamp(arg, repo=repo)

            if arg and 'T' in arg:
                print(lib.scm.svn_stats(arg, repo)[0])
            elif opts.posix_time:
                print('%f' % lib.times.iso2seconds(timestamp))
            elif opts.simple_format:
                print(timestamp)
            else:
                print('{%s}' % timestamp)

# vim: ts=8:sw=4:tw=80:et:
