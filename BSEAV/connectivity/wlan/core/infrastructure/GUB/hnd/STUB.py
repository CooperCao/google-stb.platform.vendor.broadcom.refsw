"""
An example stub for adding a new %(prog)s subcommand.

EXAMPLES:

    %(prog)s STUB ...

"""

##############################################################################
# README: this is a starter stub for new GUB commands. To make a new command:
# 1. Copy this file to <command>.py.
# 2. Change/update/extend the doc string above.
# 3. Add desired logic in the parse_cli() and call() functions below. The
#    cfgproxy variable gives access to GUB.yaml data and opts is the parsed
#    command line.
# 4. Edit bin/gub and add a new SUBCMDS entry copying existing examples.
# 5. Test that 'bin/gub help' lists the new command and 'bin/gub help <command>'
#    documents it.
# 6. Delete this comment block.
# 7. Make sure the pattern /STUB/i does not occur in the new module.
# 8. Run "pystyle" to make sure you've gotten it right before committing.
##############################################################################

from __future__ import print_function
from __future__ import unicode_literals

import argparse

import lib.consts
import lib.util


def parse_cli(prog, alias, subcmds, advanced, usage):
    """Standard subcommand command line option definitions."""
    parser = subcmds.add_parser(
        alias,
        epilog=usage,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        prog=prog)
    parser.add_argument(
        '--yes', action='store_true',
        help="force an action which would otherwise be prompted for"
        if advanced else argparse.SUPPRESS)
    parser.add_argument('args', nargs='*')
    parser.set_defaults(func=call)


def call(opts, cfgproxy):
    """Standard subcommand entry point."""
    # If a handle to the config file is needed.
    # cfgroot = cfgproxy.parse()

    # Replace the following with your code
    if opts != cfgproxy:
        lib.util.note('stub command')


# vim: ts=8:sw=4:tw=80:et:
