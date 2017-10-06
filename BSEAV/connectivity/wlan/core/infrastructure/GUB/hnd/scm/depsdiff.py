"""
Compare two DEPS files by URL.

EXAMPLES:

Show all textual diffs between two URLS:

    %(prog)s scm depsdiff \\
            $$svn/components/deps/branches/JAGUAR_BRANCH_14_10/dhd \\
            $$svn/components/deps/trunk/dhd

Show just a summary of diffs:

    %(prog)s scm depsdiff --summarize <left-url> <right-url>

Pass random flags through to svn diff:

    %(prog)s scm depsdiff \\
            --svn-option=--no-diff-added \\
            --svn-option=--no-diff-deleted \\
            --svn-option=--show-copies-as-adds \\
            <left-url> <right-url>

Use tag names instead of URLs:

    %(prog)s scm depsdiff -t IGUANA_REL_13_10_98 -t IGUANA_REL_13_10_99 wl-src

"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse

import lib.consts
import lib.pop
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
        '--summarize', action='store_true',
        help="print a summary of differences")
    parser.add_argument(
        '--svn-option', default=[], action='append',
        metavar='FLAG',
        help="pass this option along to svn")
    parser.add_argument(
        '-t', '--tag', default=[], action='append',
        help="basename of branch/tag/twig/trunk")
    parser.add_argument('urls', nargs=argparse.REMAINDER)
    parser.set_defaults(func=call)


def call(opts, _):
    """Standard subcommand entry point."""

    def deps_url(tag, deps):
        """Given a deps file and tag name, return its full URL."""
        return '/'.join([
            lib.consts.WL_REPO_ROOT,
            'components/deps',
            lib.scm.svn_tag_path(tag),
            deps
        ])

    if len(opts.urls) == 2:
        lpath, rpath = opts.lpath, opts.rpath
    else:
        assert len(opts.urls) == 1
        assert len(opts.tag) == 2
        lpath, rpath = (deps_url(opts.tag[0], opts.urls[0]),
                        deps_url(opts.tag[1], opts.urls[0]))

    reftime = lib.times.timeref(None)

    ltree = lib.pop.Tree(basedir='.', question={})
    ltree.add(lpath, reftime=reftime)
    ltree.populate(reftime=reftime)

    rtree = lib.pop.Tree(basedir='.', question={})
    rtree.add(rpath, reftime=reftime)
    rtree.populate(reftime=reftime)

    both = set(ltree.question.keys()) & set(rtree.question.keys())

    for side in ((lpath, ltree), (rpath, rtree)):
        for key in sorted(side[1].question, key=len):
            if key not in both:
                print('Only in %s: %s' % (lib.scm.svn_tag_name(side[0]), key))

    for path in sorted(both, key=len):
        cmd = lib.scm.svncmd('diff', '--ignore-properties') + opts.svn_option
        if opts.summarize:
            cmd.append('--summarize')
        cmd.append(ltree.question[path])
        cmd.append(rtree.question[path])
        lib.util.execute(cmd, vl=2)

# vim: ts=8:sw=4:tw=80:et:
