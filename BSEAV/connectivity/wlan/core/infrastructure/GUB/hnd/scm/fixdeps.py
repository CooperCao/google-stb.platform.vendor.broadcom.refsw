"""
Rewrite the named DEPS files with a standard format, optionally modifying
standard variables while doing so.

Note that this operates on checked-out files (working copies), not
URLs directly.

EXAMPLES:

Simply clean up a set of checked out DEPS file:

    %(prog)s scm fixdeps */DEPS

Also change the base svn URL while editing:

    %(prog)s scm fixdeps -u <new-url> */DEPS

"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import os
import re
import sys

import lib.consts
import lib.scm
import lib.util

VARS_WIDTH = 36
DEPS_WIDTH = 42

DEPS_OS = """

deps_os = {
  'win'  : { },
  'mac'  : { },
  'unix' : { },
}
"""


def parse_cli(prog, alias, subcmds, _, usage):
    """Standard subcommand command line option definitions."""
    parser = subcmds.add_parser(
        alias,
        epilog=usage,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        prog=prog)
    parser.add_argument(
        '--deps-width', type=int, default=DEPS_WIDTH,
        metavar='N',
        help="format 'deps' entry width to N chars (default=%(default)s)")
    parser.add_argument(
        '--no-deps_os', action='store_true',
        help="leave out the 'deps_os' feature which we don't use")
    parser.add_argument(
        '-u', '--base-url',
        metavar='URL',
        help="alternative base url for svn files")
    parser.add_argument(
        '--vars-width', type=int, default=VARS_WIDTH,
        metavar='N',
        help="format 'vars' entry width to N chars (default=%(default)s)")
    parser.add_argument('deps', nargs=argparse.REMAINDER)
    parser.set_defaults(func=call)


def call(opts, _):
    """Standard subcommand entry point."""
    rc = 0

    def fmtline(key, val, width, name=None, quote='"'):
        """Format a dict entry for the deps file."""
        qkey = '%s%s%s' % (quote, key, quote)
        qval = ('%s%s%s' % (quote, val, quote) if name == 'vars' else val)
        return '  %-*s: %s,' % (width, qkey, qval)

    def fmtvars(table, width, quote='"'):
        """Format a dict for the deps file."""
        lines = ['vars = {']  # balance the }
        for key in sorted(table):
            val = table[key]
            if val.endswith(')+'):
                val = val[0:-1]
            line = fmtline(key, val, width, name='vars', quote=quote)
            lines.append(line)
        lines.append('}\n')
        return lines

    for path in sorted(opts.deps):
        if os.path.isdir(path):
            path = os.path.join(path, 'DEPS')

        text = open(path, 'r').read()
        vars_dict, deps_dict, depsdata = lib.scm.parse_deps(text, expand=False)

        if opts.base_url:
            vars_dict['wl_repo_root'] = re.sub(
                r'''http://[^'"]*''',
                opts.base_url,
                vars_dict['wl_repo_root']
            )

        new = []
        state = None
        for line in depsdata.splitlines():
            lstripped = line.lstrip()
            if not lstripped.startswith('#'):
                line = lstripped

            if not line and state != 'deps':
                continue
            elif line.startswith('}'):
                if state != 'vars':
                    new.append('}')
                new.append('')
                state = None
            elif line.startswith('vars'):
                new.append('')
                state = 'vars'
                new += fmtvars(vars_dict, opts.vars_width)
            elif line.startswith('deps_os'):
                break
            elif line.startswith('deps'):
                new.append('deps = {')  # balance the }
                state = 'deps'
            else:
                if state == 'deps':
                    parts = line.split(':', 1)
                    if len(parts) == 2:
                        key = parts[0].strip()
                        vf = '''def Var(name): return 'Var("%s")+' % name\n'''
                        nq = eval(vf + key)  # pylint: disable=eval-used
                        if nq in deps_dict:
                            key = key.strip("'").strip('"')
                            val = parts[1].strip().rstrip(',')
                            new.append(fmtline(key, val, opts.deps_width))
                        else:
                            new.append(line)
                    else:
                        new.append(line)
                elif state != 'vars':
                    new.append(line)

        print('Updating: ' + path)
        with open(path, 'w') as f:
            f.write('\n'.join(new))
            if not opts.no_deps_os:
                f.write(DEPS_OS)

    sys.exit(2 if rc else 0)

# vim: ts=8:sw=4:tw=80:et:
