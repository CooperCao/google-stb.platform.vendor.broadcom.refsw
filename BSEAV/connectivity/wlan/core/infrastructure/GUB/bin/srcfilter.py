#!/usr/bin/env python
"""
Filter out filenames from stdin based on rulesets.

This is a rewrite in Python of the old Perl program srcfilter.pl.
It's generally much faster than the .pl version because by default
it does not use regular expressions but rather compares pathnames
as strings.

It also adds a --expand-dirs flag which allows directories to be expanded
in the output stream. Without this, any attempts to trim its output
with commands like "grep -v foobar" would be unreliable since files
matching that pattern might be implicit under a parent directory.
For the same reason, built-in ignores like /unittest/ may require
--expand-dirs.  Also, a command like pax will interpret a directory
path as meaning "take everything under here", which is another reason
passing directories to the output stream is deprecated. Thus the -d,
--expand-dirs option is preferred.

The ruleset should be a newline-separated list of pathnames. If a
rule is prefaced with '~' the input path is included if it matches,
otherwise it is excluded. If multiple rules match the last one to match
takes precedence. When run with the -v option the inverse happens:
entries with '~' are excluded and others are included.

Wildcard (directory) rules are discouraged because they lead to great
complexity. Imagine the sequence:
    ~directory/
    directory/subdir/
    ~directory/subdir/foobar.c
What does the user intend here? Combining this with the possibility of
combined/preprocessed/mogrified filelists, not to mention the -v flag,
makes the confusion even greater.

Paths containing SCM metadata (svn, git, and cvs) are skipped
automatically, as are "unittest" paths. Therefore it is not necessary
to grep these files out of the pipeline or use e.g. the pax -s flag to
get rid of them, though it may still be preferred to skip them in the
initial find command for the sake of efficiency.

COMPATIBILITY:

A number of bugs or flaws in the .pl version are addressed by the .py:

1. The .pl uses regular expressions which is unnecessary because no
   filelists make use of them. This slows things down a great deal and
   also adds complexity by making it necessary to escape "." chars.
2. Speaking of which, due to a missing 'g' modifier the .pl escapes only
   the first "." in the rule. Subsequent dots become wild-card chars.
3. The .pl behavior is asymmetrical, going against documented behavior:
   rules without a leading ~ are anchored while rules with it are not.
4. It tests all paths against the literal ~ version of each rule by RE.
   This makes no sense because no path will start with a literal ~.

This rewrite uses simple string comparisons by default instead of REs,
resulting in an order of magnitude speedup. This solves #1 above as well
as #2 and $4 since RE matching isn't done. Bug #3 is fixed separately
but a --compat option is added to reintroduce #2 and #3 for cases when
bug-for-bug compatibility is required. There's also a new --regex flag
to force use of REs as the .pl did.

NOTE:

We don't want filelists to be too complicated. Therefore it would be best
to remove unneeded options once the compatibility transition is complete.
Specifically, --compat and --regexp should go away while --expand-dirs
could become the default and the option could go away. But --exact is
needed in a few places and -v is important, while -L and -V are worth
keeping around for future debugging use.

EXAMPLES:

Copy a subset of source files to the ./build area:

    find src components | srcfilter.py -v xxx-filelist.txt | pax -rw build
"""

# $Copyright Broadcom Corporation$
#
# <<Broadcom-WL-IPTag/Proprietary:>>

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import os
import re
import sys

IGNORE = (
    '.svn',
    '.git',
    'CVS',
    '.cvsignore',
    'unittest',
    '_SUBVERSION_REVISION',
)


class Rule(object):
    """Represent a rule line from a filelist."""

    def __init__(self, line, compat=False, exact=False, regex=False):
        self.compat = compat
        self.exact = exact
        self.flip = (line[0] == '~')
        if self.flip:
            line = line[1:]
            assert line[0] != '~'
        if regex or '*' in line or \
           '[' in line or ']' in line or \
           line[0] == '^' or line[-1] == '$' or \
           (self.compat and line.count('.') > 1):
            # Avoid accidental substring matches by escaping periods.
            # The .pl version has a bug: it escapes only the first one.
            splits = 1 if self.compat else -1
            self.patt = '[.]'.join(line.split('.', splits)).replace('[.]*',
                                                                    '.*')
            if exact:
                if self.patt[0] != '^':
                    self.patt = '^' + self.patt
                if self.patt[-1] != '$':
                    self.patt += '$'
            elif self.compat and not self.flip:
                # Another srcfilter.pl bug: unflipped matches are anchored,
                # flipped are not.
                self.patt = '^' + self.patt
            self.regex = re.compile(self.patt)
        else:
            self.patt = self.regex = None
        self.line = line

    def __repr__(self):
        if self.patt:
            return '%s /%s/ (%s)' % (self.line, self.patt, self.flip)
        else:
            return '%s (%s)' % (self.line, self.flip)

    def matches(self, path):
        """True iff this path matches this rule."""
        if self.regex:
            match = re.search(self.regex, path) is not None
        elif self.exact:
            match = self.line == path
        else:
            if self.compat and not self.flip:
                match = path.startswith(self.line)
            else:
                match = self.line in path
        return match


def main():
    """Main routine."""
    parser = argparse.ArgumentParser(
        epilog=__doc__.strip(),
        formatter_class=argparse.RawDescriptionHelpFormatter)
    # TODO - ideally this option will go away once filelists are fixed.
    parser.add_argument(
        '--compat', action='store_true',
        help="Stay bug-for-bug compatible with srcfilter.pl")
    parser.add_argument(
        '-d', '--expand-dirs', action='store_true',
        help="Expand directory paths recursively on output")
    parser.add_argument(
        '-e', '--exact', action='store_true',
        help="Accept exact (anchored) matches only")
    parser.add_argument(
        '-f', '--pathlist',
        help="Read path list from PATHLIST (default=stdin)")
    parser.add_argument(
        '-L', '--list-files',
        help="save the list of selected files here")
    parser.add_argument(
        '-r', '--regex', action='store_true',
        help="Treat rules as regular expressions instead of paths")
    parser.add_argument(
        '-V', '--verbose', action='store_true',
        help="Send verbose output to stderr")
    parser.add_argument(
        '-v', '--invert', action='store_true',
        help="Use inverse logic")
    parser.add_argument(
        'rulefiles', nargs='*',
        help="filelist containing copy rules")
    opts = parser.parse_args()

    # Read in the list of rules and turn them into Rule objects.
    rules = []
    for rulefile in opts.rulefiles:
        with open(rulefile) as f:
            for line in f:
                line = line.strip()
                if not line or line[0] == '#':
                    continue
                rules.append(Rule(line,
                                  compat=opts.compat,
                                  exact=opts.exact,
                                  regex=opts.regex))

    # The output stream may optionally be saved to a file besides stdout.
    streams = [sys.stdout]
    if opts.list_files:
        ldir = os.path.dirname(opts.list_files)
        if ldir and not os.path.exists(ldir):
            os.mkdir(ldir)
        streams.append(open(opts.list_files, 'a'))

    def dump(path):
        """Deliver a pathname to the output stream."""
        for stream in streams:
            stream.write(path + '\n')

    def allow(path, rules, opts):
        """Apply the rule set to the specified path."""
        if any([w in path for w in IGNORE]) or path.endswith('.pyc'):
            if opts.verbose:
                sys.stderr.write('ignored "%s"\n' % path)
            return False
        # Use the last rule which matches this path. Do not 'break' early.
        accept, matched = True, None
        for rule in rules:
            if rule.matches(path):
                accept, matched = rule.flip, rule.line
        allowed = (accept != opts.invert)
        if allowed and opts.verbose:
            sys.stderr.write('ALLOWED: %s by "%s"\n' % (path, matched))
        return allowed

    # Apply the rule set to the input path list.
    pathlist = open(opts.pathlist) if opts.pathlist else sys.stdin
    for path in pathlist:
        path = path.strip()

        # Check this path against the rule set.
        if not allow(path, rules, opts):
            continue

        # It's generally not a good idea to print a directory
        # path to the output stream since tools like pax will
        # interpret it as "take everything under this dir
        # recursively", so all our filtering could be undone
        # by a parent directory which gets into the output.
        # So we expand directories, taking care to apply filtering
        # rules to discovered contents too.
        # But we still preserve empty dirs in case they're wanted.
        if opts.expand_dirs and \
           os.path.isdir(path) and not os.path.islink(path):
            for parent, dnames, fnames in os.walk(path):
                if not dnames and not fnames:
                    if allow(parent, rules, opts):
                        dump(parent)
                for fn in fnames:
                    subpath = os.path.join(parent, fn)
                    if allow(subpath, rules, opts):
                        dump(subpath)
        else:
            dump(path)

if __name__ == '__main__':
    main()

# vim: ts=4:sw=4:tw=80:et:
