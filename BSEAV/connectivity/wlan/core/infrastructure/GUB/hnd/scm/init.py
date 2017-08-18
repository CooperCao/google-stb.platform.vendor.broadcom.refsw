"""
Generic SCM (Software Change Management) commands (svn/git).

The "%(prog)s scm" tool suite is essentially a replacement for gclient
and sparse though those remain the official tools of choice. It provides
DEPS-wide checkout, checkin, update/sync, status, diff, and cleanup
functionality via svn and (eventually) git. It's a wrapper providing
command-line access to the checkout code used by "%(prog)s build".

IMPORTANT: the %(prog)s scm suite generally operates on the entire
directory tree regardless of where the command is issued. In other words
"%(prog)s scm up" will update all checkouts in the entire tree even
if you run it from deep in a subtree like .../src/wl/sys. To operate
only on the current checkout use the underlying SCM tool (svn, git).

These tools manage the same file metadata (e.g. .gclient files)
as "%(prog)s gclient" meaning there is no lock-in effect and you
can generally switch between them easily. For instance you could use
"%(prog)s scm co" for the initial checkout, then update it with "%(prog)s
gclient up" or vice versa.

INDIVIDUAL TOOLS:

The "%(prog)s scm co" command aka "checkout" handles initial checkouts.
In other words it does an "svn co" of each URL listed in the DEPS file.

The "%(prog)s scm up" command aka "{sync,update}" is the full-tree
analogue of "svn up".

The "%(prog)s scm st" command aka "{stat,status}" is the analogue of
"svn st".

The "%(prog)s scm diff" command is the analogue of "svn diff".

The "%(prog)s scm scrub" command is useful for cleaning up an existing
workspace.  It can remove unversioned files, revert modifications of
versioned files, etc.

The "%(prog)s scm components" command prints a list of components
comprising the tree and can be used to script custom interactions.

The "%(prog)s scm apply" command applies the specified patch file
to the subset of files which exist in the current directory.

The "%(prog)s scm depsdiff" command compares two deps files identified
by url.

See the SCM SUBCOMMANDS list above for the full command set and use
"%(prog)s scm help <tool>" with any tool listed there for details. E.g.:

    %(prog)s scm help            (this message)
    %(prog)s scm help checkout   (aka 'co')
    %(prog)s scm help checkin    (aka 'ci')
    %(prog)s scm help status     (aka 'st')
    %(prog)s scm help sync       (aka 'up')
    %(prog)s scm help components
    %(prog)s scm help diff
    %(prog)s scm help depsdiff
    %(prog)s scm help apply
    %(prog)s scm help scrub

"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse

import lib.consts
import lib.util

SUBCMDS = {
    'apply': {'aliases': ('patch',)},
    'checkout': {'aliases': ('co',)},
    'commit': {'aliases': ('ci',)},
    'components': {'aliases': None},
    'depsdiff': {'aliases': None},
    'diff': {'aliases': None},
    'scrub': {'aliases': ('clean',)},
    'stat': {'aliases': ('st', 'status')},
    'sync': {'aliases': ('up', 'update')},
}


def add_subcmds(base, subcmds, advanced, usage):
    """Standard subcommand command line option definitions."""
    # pylint: disable=unused-argument,unused-variable
    parser = subcmds.add_parser(
        __name__.split('.')[-2],
        epilog=usage,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        prog=lib.consts.PROG)
    subparser = parser.add_subparsers(
        title='SCM SUBCOMMANDS',
        help="use '%(prog)s scm help subcommand' for help"
    )

    setups, cmdnames = lib.init.setup(base=base, subcmds=SUBCMDS)
    try:
        exec setups
    except ImportError:
        # If something this command needs is missing we'll find
        # out soon enough. If it's an import needed by another
        # command we don't care.
        pass
    return cmdnames

# vim: ts=8:sw=4:tw=80:et:
