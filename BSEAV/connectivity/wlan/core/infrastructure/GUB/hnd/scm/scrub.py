"""
A collection of ways to clean up an existing SCM checkout tree.

This is a grab bag of methods for cleaning up an existing checkout.
There's a flag to remove unversioned files, a flag to restore deleted
versioned files, and a flag to revert (undo) modifications to versioned
files. Dangerous operations are prompted for unless the --yes flag
is used. The default is "--remove --restore".

Operates on the entire workspace by default but can operate on a named
subdir (see examples).

Use with caution because this has the capacity to lose work!

EXAMPLES:

To remove non-versioned files:

    %(prog)s scm scrub --remove

As above but removing svn-ignored files too:

    %(prog)s scm scrub --remove --all

As above, without prompting:

    %(prog)s scm scrub --remove --all --yes

Clean up only within the current directory:

    %(prog)s scm scrub --remove .

Restore versioned files which have been removed:

    %(prog)s scm scrub --restore

Revert (undo) all modifications to versioned files:

    %(prog)s scm scrub --revert

As above, without prompting (USE WITH CARE):

    %(prog)s scm scrub --revert --yes

Do all of the above:

    %(prog)s scm scrub --remove --all --restore --revert

"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import os
import sys

import lib.consts
import lib.opts
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
        '-a', '--all', '--no-ignore', action='store_true',
        help="show 'ignored' paths as well")
    parser.add_argument(
        '--delete-if-clean', action='store_true',
        help="remove the ENTIRE CHECKOUT if it contains no mods")
    parser.add_argument(
        '--yes', '--noprompt', action='count', default=0,
        help="suppress prompting")
    parser.add_argument(
        '--mergeinfo', action='count', default=0,
        help="remove svn mergeinfo on files without affecting their diffs")
    parser.add_argument(
        '--remove', action='store_true',
        help="remove non-versioned files")
    parser.add_argument(
        '--restore', action='store_true',
        help="restore removed versioned files")
    parser.add_argument(
        '--revert', action='store_true',
        help="revert (undo) modifications to versioned files")
    parser.add_argument('dirs', nargs=argparse.REMAINDER)
    parser.set_defaults(func=call)


def call(opts, _):
    """Standard subcommand entry point."""
    rc = 0

    # By default, do removes and restores but not reverts.
    if not (opts.remove or opts.restore or opts.revert):
        opts.remove = opts.restore = True

    if not opts.dirs:
        opts.dirs = [lib.scm.find_tree_base()]

    svn_st_cmd = lib.scm.svncmd('stat', '--no-ignore')
    git_st_cmd = lib.scm.gitcmd('stat')

    if opts.all:
        git_st_cmd.append('--ignored')

    for d in opts.dirs:
        svn_removes, svn_restores, svn_reverts = set(), set(), set()
        svn_mergeinfos = {}

        try:
            os.stat(d)
        except Exception as e:
            lib.util.error(str(e))
            rc += 1
            continue

        svns, gits, others = lib.scm.find_components(d, search_up=1)

        if gits:
            lib.util.die('git support is TBD %s' % gits)

        for svn in svns:
            proc = lib.util.execute(svn_st_cmd + [svn],
                                    stdout=lib.util.PIPE, vl=2)
            stdout = proc.communicate()[0].decode('utf-8')
            if proc.returncode:
                rc += 1
                continue

            for line in stdout.splitlines():
                pfx, path = line[0:7], line[8:].rstrip()
                path = lib.util.shortpath(path)
                if pfx[0] in '?~':
                    svn_removes.add(path)
                elif pfx[0] in 'I' and opts.all:
                    svn_removes.add(path)
                elif pfx[0] in '!':
                    svn_restores.add(path)
                elif pfx[0] in 'ADM':
                    svn_reverts.add(path)

                if pfx[1] == 'M' and (opts.mergeinfo > 1 or
                                      os.path.isfile(path)):
                    svn_mergeinfos[path] = '%s.%d' % (path, os.getpid())

        for other in others:
            svn_removes.add(lib.util.shortpath(other))

        if opts.mergeinfo and svn_mergeinfos:
            micmd = lib.scm.svncmd('revert', '-q')
            for path in sorted(svn_mergeinfos):
                if not os.path.isdir(path):
                    lib.util.mv(path, svn_mergeinfos[path], vl=2)
                micmd.append(path)
            lib.util.execute(micmd, check=True, vl=2)
            for path in sorted(svn_mergeinfos):
                if not os.path.isdir(path):
                    lib.util.mv(svn_mergeinfos[path], path, vl=2)

        if opts.remove and svn_removes:
            goners = []
            for path in sorted(svn_removes):
                if opts.yes:
                    response = 'y'
                elif os.path.isdir(path):
                    prompt = 'Remove directory "%s"? ' % path
                    response = raw_input(prompt).lower()
                else:
                    prompt = 'Remove regular file "%s"? ' % path
                    response = raw_input(prompt).lower()

                if response.startswith('y'):
                    goners.append(path)
                    svn_removes.remove(path)

            if goners:
                lib.util.rm(goners)

        revcmd = lib.scm.svncmd('revert')

        if opts.revert:
            for path in sorted(svn_reverts):
                if opts.yes:
                    response = 'y'
                else:
                    prompt = 'Revert modified file "%s"? ' % path
                    response = raw_input(prompt)

                if response and response.lower()[0] == 'y':
                    revcmd.append(path)
                    svn_reverts.remove(path)

        if opts.restore and svn_restores:
            revcmd.extend(sorted(svn_restores))

        if len(revcmd) > len(lib.scm.svncmd('revert')):
            lib.util.execute(revcmd, check=True)

        if opts.delete_if_clean:
            if svn_removes or svn_reverts:
                continue

            path = os.path.normpath(d)

            if os.getcwd().startswith(os.path.realpath(path)):
                lib.util.warn('will not delete current directory')
                continue

            if opts.yes:
                response = 'y'
            else:
                prompt = 'Delete entire (clean) checkout tree "%s"? ' % path
                response = raw_input(prompt)

            if response and response.lower()[0] == 'y':
                lib.util.rm(path)

    sys.exit(rc)

# vim: ts=8:sw=4:tw=80:et:
