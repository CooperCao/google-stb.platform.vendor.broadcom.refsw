#!/usr/bin/env python

"""
Populate the build area for a WDK build.

This program uses makedepend or gcc -MM to derive the list of
required header files and copies them into the build area. The
question of whether to use makedepend or gcc is complicated. All
things being equal we'd prefer gcc since makedepend is
obsolete, nonstandard, and barely maintained whereas gcc is the
opposite. Unfortunately it turns out that gcc is much slower for
some reason. Also, gcc wasn't installed on all of our precommit
systems so a gcc-based version can't pass precommit till that's
fixed. On the other hand, makedepend as released has a static limit
of 64 -I flags which on some builds/branches can be exceeded but
we've got a locally built binary which raises the limit to 512
and we've submitted that patch.

So, for the time being at least this script prefers our patched
makedepend over the network but it could easily be modified to
use /usr/bin/makedepend (if our patch is accepted) or even gcc.

"""

# Must stay compatible with Python 2.5 until C:\Tools\win32 is obsoleted.

import optparse
import os
import shutil
import subprocess
import sys

# See comments above.
MAKEDEPEND_MAX_IFLAGS = 512


# Adapted from 2.7 os.path.relpath() for 2.5 compatibility.
def relpath(path, start=None):
    """Return a relative version of a path"""

    if not start:
        start = os.getcwd()

    start_list = [x for x in os.path.abspath(start).split(os.path.sep) if x]
    path_list = [x for x in os.path.abspath(path).split(os.path.sep) if x]

    # Work out how much of the filepath is shared by start and path.
    i = len(os.path.commonprefix([start_list, path_list]))

    rel_list = ['..'] * (len(start_list) - i) + path_list[i:]
    if not rel_list:
        return os.getcwd()
    return os.path.join(*rel_list)  # pylint: disable=star-args


def main(argv):
    """
    Populate the build area for a Cygwin/Windows WDK build.
    """

    parser = optparse.OptionParser(usage=__doc__.strip())
    parser.allow_interspersed_args = False
    parser.add_option(
        '-b', '--tree-base',
        help="base of tree")
    parser.add_option(
        '--makedepend',
        help="path to makedepend binary to use")
    parser.add_option(
        '--use-gcc', action='store_true',
        help="prefer gcc over makedepend")
    parser.add_option(
        '--use-makedepend', action='store_true',
        help="prefer makedepend over gcc")
    parser.add_option(
        '--skip',
        default=[], action='append',
        help="skip named file")
    parser.add_option(
        '--copy-tree',
        default=[], action='append', nargs=2,
        help="copy entire tree specified in <arg1> to <arg2>")
    parser.add_option(
        '-t', '--to-dir',
        help="base directory of build/copy tree")
    parser.add_option(
        '--verbose', action='count', default=0,
        help="increment verbosity level")
    opts, args = parser.parse_args(argv[1:])

    if not args:
        main([sys.argv[0], '-h'])

    treebase = os.path.abspath(opts.tree_base)

    flags = []
    srcs = []
    dirs = []
    seen = set()
    for word in args:
        if word[0] == '-':
            flags.append(word)
            continue

        if os.path.exists(word):
            rpath = relpath(word)
        elif os.path.exists(os.path.join(treebase, word)):
            rpath = relpath(os.path.join(treebase, word))
        else:
            print >>sys.stderr, 'Warning: No such file or directory: ' + word
            continue

        if os.path.isdir(rpath):
            if rpath not in seen:
                dirs.append(rpath)
                seen.add(rpath)
        else:
            if rpath not in seen:
                srcs.append(rpath)
                seen.add(rpath)

    print 'Preprocessing %d source files to get required headers' % len(srcs)

    nhdrs = len([i for i in flags if i.startswith('-I')])
    prefer_mkdep = opts.use_makedepend or \
        not os.path.exists('/usr/bin/gcc') or \
        nhdrs < MAKEDEPEND_MAX_IFLAGS

    # See comments above for makedepend-vs-gcc logic.
    if prefer_mkdep and not opts.use_gcc:
        if opts.makedepend:
            makedepend = opts.makedepend
        else:
            import re
            from distutils.version import StrictVersion
            cygver = re.sub(r'[^\d.].*', '', os.uname()[2])
            if StrictVersion(cygver) < StrictVersion('1.6.0'):
                # This binary raises the limit on number of -I flags from
                # 64 to 512 and was built for and requires Cygwin 1.5.x/32.
                makedepend = 'Z:/projects/hnd_tools/win/bin/makedepend.exe'
            elif False and StrictVersion(cygver) >= StrictVersion('1.7.27'):
                # Someday, if our patch to raise the number of -I flags is
                # accepted and/or we improve the makefile model to trim
                # unneeded -I flags, we can just install the makedepend
                # package and use the local copy. Disabled for now.
                makedepend = '/usr/bin/makedepend'
            elif '64' in os.uname()[4]:
                # This binary raises the limit on number of -I flags from
                # 64 to 512 and was built for and requires Cygwin 1.7.x/64.
                makedepend = 'Z:/projects/hnd_tools/win/bin/64/makedepend.exe'
            else:
                # This binary raises the limit on number of -I flags from
                # 64 to 512 and was built for and requires Cygwin 1.7.x/32.
                makedepend = 'Z:/projects/hnd_tools/win/bin/32/makedepend.exe'
        cmd = [makedepend, '-f', '-', '-w7000', '-Y']
    else:
        cmd = ['/usr/bin/gcc', '-MM', '-MG', '-nostdinc']

    cmd += flags
    cmd += srcs
    print >>sys.stderr, '+', ' '.join(cmd)
    # print >>open(os.path.join(opts.to_dir, '.MKD'), 'wb'), ' '.join(cmd)
    proc = subprocess.Popen(cmd,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    out, err = proc.communicate()
    includes = set()
    for line in out.splitlines():
        if line.startswith('#'):
            continue
        line = line.strip(' \\')
        for inc in line.split():
            if inc.endswith('.h'):
                includes.add(inc)
    if proc.returncode:
        sys.stderr.write(err)

    print >>sys.stderr, 'Copying %d header files into %s' % (len(includes),
                                                             opts.to_dir)
    for src in sorted(includes):
        if [s for s in opts.skip if src.endswith(s)]:
            continue
        rsrc = relpath(os.path.abspath(src), treebase)
        dst = os.path.join(opts.to_dir, rsrc)
        if opts.verbose:
            print >>sys.stderr, '+ cp -p %s %s' % (src, dst)
        dstdir = os.path.dirname(dst)
        if not os.path.exists(dstdir):
            os.makedirs(dstdir)
        if os.path.exists(src):
            shutil.copy2(src, dst)
        else:
            print >>sys.stderr, 'Warning: No such file:', src

    for src in dirs:
        dstdir = os.path.join(opts.to_dir, os.path.basename(src))
        print >>sys.stderr, '+ cp -pr %s %s' % (src, dstdir)
        shutil.copytree(src, dstdir)

    for cptree in opts.copy_tree:
        src, dstdir = cptree
        print >>sys.stderr, '+ cp -pr %s %s' % (src, dstdir)
        shutil.copytree(src, dstdir)

    sys.exit(2 if proc.returncode else 0)

if '__main__' == __name__:
    main(sys.argv)

# vim: ts=8:sw=4:tw=80:et:
