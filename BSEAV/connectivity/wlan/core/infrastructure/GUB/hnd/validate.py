"""
Check the specified directory or archive for IPTAG validity.

In order to ensure that intellectual property (IP) doesn't leak to the
wrong place via source packages, we use this tool to check embedded tags
(see http://confluence.broadcom.com/display/WLAN/BuildIPValidation).
The same logic is used during automated builds, generally on the
./release subdirectory of a finished build.

Allowed tags are configured as a property of each buildable in GUB.yaml.

EXAMPLES:

Validate a linux-external-dhd build using the iptags configured for it:

    %(prog)s validate -b linux-external-dhd \
/projects/hnd_swbuild/build_linux/trunk/linux-external-dhd/2016.6.16.0/release

Validate a specific <release-dir> using a custom iptag set:

    %(prog)s validate --iptags Open,Proprietary <release-dir>

"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import os

import lib.consts
import lib.opts
import lib.util
import lib.val


def parse_cli(prog, alias, subcmds, _, usage):
    """Standard subcommand command line option definitions."""
    parser = subcmds.add_parser(
        alias,
        epilog=usage,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        prog=prog)
    parser_tagset = parser.add_mutually_exclusive_group()
    parser_tagset.add_argument(
        '-b', '--buildable',
        metavar='ITEM',
        help="the buildable item from which to get allowed iptags")
    parser_tagset.add_argument(
        '-i', '--iptags',
        help="a comma-separated list of custom IP tags")
    parser.add_argument('paths', nargs='*')
    parser.set_defaults(func=call)


def call(opts, cfgproxy):
    """Standard subcommand entry point."""
    cfgroot = cfgproxy.parse()
    rc = 0
    for path in opts.paths:
        path = os.path.normpath(os.path.abspath(path))
        iptags = opts.iptags
        if opts.buildable:
            bldable = opts.buildable
            cfg = cfgroot.buildables().get(bldable)
        else:
            bldable = os.path.basename(os.path.dirname(os.path.dirname(path)))
            cfg = cfgroot.buildables().get(bldable)
            if not cfg:
                bldable = os.path.basename(os.path.dirname(path))
                cfg = cfgroot.buildables().get(bldable)

        if not cfg:
            lib.util.error('no such buildable: %s' % bldable)
            rc = 2
            continue

        ipstr = cfg.getvar(lib.val.IPTAG_MAP)
        if not ipstr:
            lib.util.error('no iptag data for buildable: %s' % bldable)
            rc = 2
            continue

        ipmap = dict([kv.split(':') for kv in ipstr.split()])
        keyset = set(ipmap.keys())
        if os.path.basename(path) in keyset or not os.path.isdir(path):
            ipaths = [path]
        else:
            ipaths = [os.path.join(path, d)
                      for d in sorted(set(os.listdir(path)) & keyset)]

        if not ipaths:
            lib.util.error('no release IP found under: %s' % path)
            rc = 2
            continue

        for ipath in ipaths:
            name = os.path.basename(ipath)
            if os.path.isdir(ipath):
                iptags = ipmap.get(name)
            else:
                iptags = ipmap.get(os.path.basename(os.path.dirname(ipath)))
            if not iptags:
                lib.util.error('no iptag data for %s' % name)
                rc = 2
                continue

            lib.util.note('validating IP in "%s" against iptags "%s"',
                          name, iptags)

            if os.path.isdir(ipath):
                if not lib.val.checkdir(ipath, iptags):
                    rc = 2
            else:
                if not lib.val.checkfile(ipath, lib.util.shortpath(ipath),
                                         iptags=iptags):
                    rc = 2

    if rc:
        lib.util.note('See %s', lib.val.WIKI)

    return rc

# vim: ts=8:sw=4:tw=80:et:
