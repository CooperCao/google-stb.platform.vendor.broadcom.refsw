"""
Generate a document summarizing the commits contributing to a build.

One of the goals here is to produce a stable, human-readable text
format which will diff and delta easily for version control purposes.
The canonical output format is JSON, with HTML as an optional secondary
format.

A checkout directory DIR must be provided. The report will only mention
commits affecting at least one file found in that directory tree.
This is required because some commits span multiple branches and some
builds may use sparse svn trees. A commit to any file present in the
build tree is considered to be involved in the build, though no attempt
is made to verify whether the file was actually opened and used.

A report covering a given date or revision range may be generated from
scratch.  Alternatively, a persistent JSON database may be maintained
with each invocation extending it by the latest commits (this is how
automated builds do it).

The algorithm of this tool and its file format are explained in depth
in the docstring of the lib/contents.py module.

The script requires values corresponding to the "-t TAG", "-b BUILDABLE",
and "-n BUILD_NUMBER" options. However, if given a standard build
directory such as the following it can infer them from the path:

    /projects/hnd_swbuild/build_linux/<TAG>/<BUILDABLE>/<BUILD_NUMBER>

Many examples below assume DIR to be a standard build path. If not,
the -b, -n, and -t flags must be used explicitly.

EXAMPLES:

To get the entire content history of a buildable on a branch:

    %(prog)s contents -t IGUANA_BRANCH_13_10 -e 0 DIR

To stop at a particular revision

    %(prog)s contents -t IGUANA_BRANCH_13_10 -e 433912 DIR

Or to stop at July 4, 2013:

    %(prog)s contents -t IGUANA_BRANCH_13_10 -e 2013-7-4 DIR

Save output in both JSON and HTML formats:

    %(prog)s contents -t 12_10 -T xyz.json -w xyz.html DIR

As above, but build upon the pre-existing database:

    %(prog)s contents -t 12_10 -J xyz.json -w xyz.html DIR

Print the last good build (LGB) of a buildable on a branch:

    %(prog)s contents -t 13_10 -b hndrte --lgb

Show addresses of relevant committers since the LGB:

    %(prog)s contents -F <URL>/${CLNAME} --blames LGB

Print a map of relevant commits since specified date:

    %(prog)s contents -F <URL>/${CLNAME} --blames {ISO8601}

Get the history of linux-external-wl on IGUANA_BRANCH_13_10 showing all flags:

    %(prog)s -V contents -b linux-external-wl -n {BUILD_NUMBER} -t 12_10 \\
        -e 0 -l {ISO8601} \\
        -F ${CLBASE}/trunk/linux-external-wl/${CLNAME} \\
        -T /tmp/to.json -w /tmp/to.html DIR

"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import os
import sys
import time

import lib.consts
import lib.contents
import lib.scm
import lib.times


def parse_cli(prog, alias, subcmds, advanced, usage):
    """Standard subcommand command line option definitions."""
    parser = subcmds.add_parser(
        alias,
        epilog=usage.format(
            BUILD_NUMBER=time.strftime('%Y.%-m.%-d.0'),
            ISO8601='{%s}' % time.strftime('%Y.%m.%dT00:00:00')),
        formatter_class=argparse.RawDescriptionHelpFormatter,
        prog=prog)
    parser.add_argument(
        '-B', '--blames',
        help="print a blame map for date range earliest[,latest]"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-b', '--buildable',
        metavar='ITEM',
        help="specify the buildable item")
    parser.add_argument(
        '-e', '--time-earliest',
        help="starting time or revision")
    parser.add_argument(
        '-l', '--time-latest',
        help="ending time or revision")
    parser.add_argument(
        '-m', '--message',
        help="commit message for DB update")
    parser.add_argument(
        '-F', '--json-from',
        help="read from the specified file or SCM URL")
    parser.add_argument(
        '-T', '--json-to',
        help="write results to the specified file or SCM URL")
    parser.add_argument(
        '-J', '--json-update',
        metavar='JSON',
        help="shorthand for '--json-from=FOO --json-to=FOO'")
    parser.add_argument(
        '-n', '--build-number',
        help="the number (YYYY.mm.dd.n) of the build tree")
    parser.add_argument(
        '-r', '--reftime', default='HEAD',
        help="timestamp or revision for JSON DB file")
    parser.add_argument(
        '--lgb', action='store_true',
        help="print the name of the last good build")
    parser.add_argument(
        '--set-failed', action='store_true',
        help="mark build status as 'failed'")
    parser.add_argument(
        '--set-succeeded', action='store_true',
        help="mark build status as 'succeeded'")
    parser.add_argument(
        '--simple-format', action='count', default=0,
        help="print a simplified format for use by tools")
    parser.add_argument(
        '-t', '--tag',
        help="basename of branch/tag/twig/trunk")
    parser.add_argument(
        '-w', '--html-to',
        metavar='FILE',
        help="write data to FILE as an HTML table")
    parser.add_argument('tree', nargs='?')
    parser.set_defaults(func=call)


def call(opts, cfgproxy):
    """Standard subcommand entry point."""
    cfgroot = cfgproxy.parse()

    # Process the starting time cutoff through the standard filter.
    if opts.time_earliest:
        opts.time_earliest = lib.times.timeref(opts.time_earliest)

    # Process the ending time cutoff through the standard filter.
    opts.time_latest = lib.times.timeref(opts.time_latest)

    # Allow for the usual numeric tail matching of tags.
    if opts.tag:
        opts.tag = cfgroot.expand_tag(opts.tag)

    if opts.json_update:
        # Convenience option.
        if opts.json_update != '-':
            opts.json_from = opts.json_update
        opts.json_to = opts.json_update
    elif not opts.json_from:
        # Provide a default svn url path for both reading and writing.
        cdir = '/'.join([lib.consts.CLBASE,
                         lib.scm.svn_tag_path(opts.tag),
                         opts.buildable])
        opts.json_from = '/'.join([cdir, lib.consts.CLNAME])
        if not opts.json_to:
            opts.json_to = opts.json_from

    # Must be done prior to chdir.
    if opts.json_from and not lib.util.is_url(opts.json_from):
        opts.json_from = os.path.abspath(opts.json_from)
    if opts.json_to and opts.json_to != '-' and \
       not lib.util.is_url(opts.json_to):
        opts.json_to = os.path.abspath(opts.json_to)
    if opts.html_to and opts.html_to != '-' and \
       not lib.util.is_url(opts.html_to):
        opts.html_to = os.path.abspath(opts.html_to)

    blditem = opts.buildable
    bldnum = opts.build_number

    if opts.tree:
        opts.tree = os.path.abspath(opts.tree)
        # Try inferring these from a standard build path if not provided.
        if not opts.buildable:
            blditem = os.path.basename(os.path.dirname(opts.tree))
        if not opts.build_number:
            bldnum = os.path.basename(opts.tree)
        if not opts.tag:
            opts.tag = os.path.basename(
                os.path.dirname(os.path.dirname(opts.tree)))

    # Specialty operations on the history DB.
    if opts.blames or opts.set_failed or opts.set_succeeded or opts.lgb:
        # Optimization: load the CL DB once for both read (blames) and write
        # (status update) operations. This creates a possibility of over-
        # blaming because someone may have committed after build start and
        # started a new build which updated the DB before we got here, but
        # the odds are low and the damage of a spurious cc is minor.
        # The status update should be done on the latest (HEAD) version.
        hist = lib.contents.History(path=opts.json_from, reftime=opts.reftime)

        if opts.set_failed or opts.set_succeeded:
            hist.set_status(bldnum, opts.set_succeeded)
            hist.store_db(opts.json_to, message=opts.message)

        if opts.blames:
            if opts.blames.upper() == 'LGB':  # "last good build"
                blamelist = hist.blames_by_lgb()
            else:
                blamelist = hist.blames_by_date(opts.blames)

            if opts.simple_format == 0:
                # This format is a human-readable string representation.
                for cmt in blamelist:
                    sys.stdout.write(str(cmt))
            elif opts.simple_format == 1:
                # This format is a simple list of email addresses.
                # Try to filter out ex-employees if possible.
                uset = None
                if not sys.platform.startswith('cyg'):
                    proc = lib.util.execute(['ypcat', 'passwd'],
                                            stdout=lib.util.PIPE,
                                            stderr=os.devnull,
                                            vl=3)
                    out = proc.communicate()[0].decode('utf-8')
                    if proc.returncode == 0:
                        uset = set([l.split(':')[0] for l in out.splitlines()])
                addrs = set()
                for cmt in blamelist:
                    user = cmt.author
                    if (uset is None or user in uset) and user[0:2] != 'XX':
                        addrs.add(user)
                for addr in sorted(addrs):
                    print(addr)
            else:
                lib.util.error('unsupported format')
        elif opts.lgb:
            last_good_build = hist.lgb()
            if last_good_build:
                print(last_good_build)
            elif hist.newcl:
                # If no previous DB exists, return error.
                sys.exit(2)

        return

    # Generate an updated content list history.
    lib.contents.derive(blditem,
                        bldnum,
                        opts.tag,
                        opts.tree,
                        cfgroot=cfgroot,
                        db_from=opts.json_from,
                        db_to=opts.json_to,
                        html_to=opts.html_to,
                        opts=opts,
                        message=opts.message,
                        time_earliest=opts.time_earliest,
                        time_latest=opts.time_latest)

# vim: ts=8:sw=4:tw=80:et:
