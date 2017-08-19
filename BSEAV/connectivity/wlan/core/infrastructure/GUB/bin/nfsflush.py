#!/usr/bin/env python2.7
"""
Flush the NFS filehandle caches of NFS directories.

Newly created files are sometimes unavailable via NFS for a period
of time due to filehandle caching, leading to apparent race problems.
See http://icecap.irssi2.org/nfs-coding-howto.html#fhcache for details.
This script forces a flush using techniques mentioned in the URL. It
can optionally do so recursively.

EXAMPLES:

    %(prog)s /projects/...

"""

from __future__ import print_function
from __future__ import unicode_literals

# Imported first to fix up sys.path etc.
import preface  # pylint: disable=relative-import,unused-import

import argparse
import sys

import lib.opts
import lib.util


def main():
    """Entry point for standalone use."""

    parser = argparse.ArgumentParser(
        epilog=__doc__.strip(),
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument(
        '-r', '--recursive', action='store_true',
        help="flush recursively")
    parser.add_argument(
        '-t', '--touch', action='store_true',
        help="additional flush action - touch and remove a temp file")
    parser.add_argument(
        '-u', '--upflush', action='store_true',
        help="flush parent directories from the root")
    parser.add_argument(
        '-V', '--verbose', action='count', default=2,
        help="increment verbosity level")
    parser.add_argument(
        'path', nargs='+',
        help="directory paths to flush")
    opts = parser.parse_args()

    lib.opts.VERBOSITY = opts.verbose

    for path in opts.path:
        lib.util.nfsflush(path, creat=opts.touch,
                          down=opts.recursive,
                          up=opts.upflush)

if '__main__' == __name__:
    if len(sys.argv) < 2:
        sys.argv.append('--help')
    main()

# vim: ts=8:sw=4:tw=80:et:
