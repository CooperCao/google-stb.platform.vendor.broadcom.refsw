#!/usr/bin/env python2

"""
Process component directories and print makefile data.

EXAMPLES:

"""

# NOTE: this script is written to the Python 2.6 API using only
# core modules. Since it may need to run in external environments,
# we prefer it to require only a standard install of Python (2.6
# or better). This is why it uses json instead of yaml, optparse
# instead of argparse, etc.

from __future__ import print_function

import json
import optparse
import os
import shutil
import sys

CFG = 'cfg.json'
IFLAG = '-I'
INDENT = '    '
LITERAL = 'literal'
PFX = 'WLMAKE_'
PRIVATE = 'incdirs_private'
PUBLIC = 'incdirs_public'
SRCFILES = 'srcfiles'
STDOUT = '__stdout__'
SUBDIRS = 'subdirs'
WRAP = ' \\\n'


def copyfiles(src, dst, flatten=False, ignore=None, links='', preserve=False):
    """Modified version of shutil.copytree.

    This is a variant of shutil.copytree() which allows the destination
    dir to pre-exist and simply tries to add and modify target files
    without removing extra files. It also adds the capacity of making
    links instead of copies and of "flattening" all files into one dir.
    Also provides the choice of whether to preserve mtimes.

    """
    names = os.listdir(src)
    if ignore is not None:
        ignored_names = ignore(src, names)
    else:
        ignored_names = set()

    if not os.path.isdir(dst):
        os.makedirs(dst)

    errors = []
    for name in names:
        if name in ignored_names:
            continue
        srcname = os.path.join(src, name)
        if flatten:
            dstname = dst
        else:
            dstname = os.path.join(dst, name)

        if os.path.exists(dstname) and not os.path.islink(dstname):
            if not os.path.isdir(dstname):
                os.unlink(dstname)

        try:
            if os.path.islink(srcname):
                if not os.path.exists(dstname):
                    linkto = os.readlink(srcname)
                    os.symlink(linkto, dstname)
            elif os.path.isdir(srcname):
                copyfiles(srcname, dstname,
                          flatten=flatten,
                          links=links,
                          ignore=ignore,
                          preserve=preserve)
            elif links:
                if not os.path.exists(dstname):
                    if links == 'r':
                        linkto = os.path.relpath(srcname,
                                                 os.path.dirname(dstname))
                    else:
                        linkto = os.path.abspath(srcname)
                    os.symlink(linkto, dstname)
            else:
                # Will raise a SpecialFileError for unsupported file types
                if preserve:
                    shutil.copy2(srcname, dstname)
                else:
                    shutil.copy(srcname, dstname)
        # catch the shutil.Error from the recursive copyfiles so that we can
        # continue with other files
        except shutil.Error, err:
            errors.extend(err.args[0])
        except EnvironmentError, why:
            errors.append((srcname, dstname, str(why)))

    if preserve:
        try:
            shutil.copystat(src, dst)
        except OSError, why:
            if shutil.WindowsError is not None and \
               isinstance(why, shutil.WindowsError):
                # Copying file access times may fail on Windows
                pass
            else:
                errors.extend((src, dst, str(why)))

    if errors:
        raise shutil.Error(errors)


class Makefile(object):

    """
    Manage a simple Makefile containing only variable assignments.
    """

    def __init__(self, propagate=False):
        self.items = []
        self.add(Variable('CMDLINE', ' '.join(sys.argv)))
        self.add(Variable('CURDIR', os.getcwd()))
        mfdir = '$(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))'
        cmnt = 'The directory containing this makefile'
        self.add(Variable('MFDIR', mfdir, comment=cmnt, style=':'))

        if propagate:
            prop = """
# Ensure that recursive (child) make invocations see this data too.
_self := $(abspath $(lastword $(MAKEFILE_LIST)))
ifeq (,$(findstring $(_self),$(MAKEFILES)))
    export MAKEFILES += $(_self)
endif
"""
        self.add(prop)

    def add(self, item):
        """Add a sequential item to the makefile."""
        self.items.append(item)

    def dump(self, f):
        """Print the entire makefile to the specified file object."""
        f.write('# Generated makefile\n')
        for item in self.items:
            if isinstance(item, Variable):
                f.write('\n')
                item.dump(f)
            else:
                f.write('%s\n' % item)


class Variable(object):

    """
    A class to handle make variables with their various assignment
    types and print them out in a nicely formatted way.
    """

    def __init__(self, name, value=None,
                 comment=None,
                 naked=False,
                 pathbase='',
                 style='',
                 tgt=None):
        self.name = (name if naked else PFX + name)
        self.value = value
        self.comment = comment
        self.pathbase = pathbase
        self.style = style
        self.tgt = tgt

    def append(self, extra):
        """Add the specified text to the variable value."""
        if self.value is None:
            self.value = extra
        elif isinstance(self.value, basestring):
            self.value += extra
        elif isinstance(extra, basestring):
            self.value.append(extra)
        else:
            self.value.extend(extra)

    def __str__(self):
        text = ''
        if self.tgt:
            text += self.tgt + ': '
        text += self.name + ' '
        if self.tgt:
            text += '+'
        else:
            text += self.style
        text += '='
        if isinstance(self.value, basestring):
            text += ' '
            if not os.path.isabs(self.value):
                text += self.pathbase
            text += self.value
        elif self.value:
            if len(self.value) > 1:
                text += WRAP + INDENT
            else:
                text += ' '
            for i, item in enumerate(self.value):
                if not os.path.isabs(item):
                    text += self.pathbase
                text += item
                if i < len(self.value) - 1:
                    text += WRAP + INDENT
        return text

    def dump(self, f, nl=1):
        """Print the final value of this variable to specified file object."""
        if self.comment:
            f.write('# %s\n' % self.comment)
        f.write('%s' % self)
        for _ in range(nl):
            f.write('\n')


def read_cfg(cfgdir):
    """Load the data from a given component's config file."""
    configfile = os.path.join(cfgdir, CFG)
    if os.path.exists(configfile):
        with open(configfile, 'r') as f:
            data = json.load(f)
            for subdir in data.get(SUBDIRS, []):
                data[subdir] = read_cfg(os.path.join(cfgdir, subdir))
    else:
        # Provide a reasonable default if no config file.
        data = {
            PRIVATE: [],
            PUBLIC: [],
            SRCFILES: [],
            SUBDIRS: [],
        }
        for parent, dnames, fnames in os.walk(cfgdir):
            rparent = os.path.relpath(parent, cfgdir)
            dnames[:] = sorted(dnames)
            if os.path.basename(rparent) == 'include':
                data[PUBLIC].append(rparent)
            elif [h for h in fnames if h.endswith('.h')]:
                data[PRIVATE].append(rparent)
            for fname in sorted(fnames):
                if fname.endswith('.c') or fname.endswith('.cpp'):
                    srcfile = os.path.join(rparent, fname)
                    data[SRCFILES].append(srcfile)
    return data


def parse_argv(argv):
    """Parse the command line."""
    parser = optparse.OptionParser(usage=__doc__.strip())
    parser.add_option(
        '-a', '--absolute-paths', action='store_true',
        help="use absolute pathnames")
    parser.add_option(
        '-b', '--base',
        help="directory to prepend to relative paths")
    parser.add_option(
        '-c', '--copy',
        help="copy component files to specified directory")
    parser.add_option(
        '-D', '--debug', action='count', default=0,
        help="enable debugging verbosity")
    parser.add_option(
        '-f', '--flatten', action='store_true',
        help="put all files in the specified directory")
    parser.add_option(
        '-H', '--hacked',
        help=optparse.SUPPRESS_HELP)
    parser.add_option(
        '-i', '--ivar',
        help="append -I flags to this variable")
    parser.add_option(
        '-l', '--link',
        help="link component files to specified directory")
    parser.add_option(
        '-m', '--mf',
        help="path for output makefile")
    parser.add_option(
        '-p', '--preserve', action='store_true',
        help="preserve timestamps of copied files (like cp -p)")
    parser.add_option(
        '-s', '--skip',
        action='append', default=[],
        help="skip specified file pattern(s) during copy")
    parser.add_option(
        '-v', '--vpat',
        action='append', default=[],
        help="generate vpath statements for specified pattern")
    opts, components = parser.parse_args(argv[1:])

    if not components:
        main([sys.argv[0], '-h'])

    if opts.copy and opts.link:
        parser.error('--copy and --link are mutually exclusive')
    if opts.preserve and not opts.copy:
        parser.error('--preserve only makes sense with --copy')

    return opts, components


def main(argv):
    """Start the execution of this module."""
    opts, components = parse_argv(argv)

    # Allow debugging to be requested via the environment too.
    if not opts.debug:
        opts.debug = int(os.environ.get(PFX + 'DEBUG', '0'))

    compdata = []
    for component in components:
        if os.path.isabs(component):
            srcdir = component
        else:
            if opts.base:
                srcdir = os.path.join(opts.base, component)
            elif opts.absolute_paths:
                srcdir = os.path.abspath(component)
            else:
                srcdir = component

        # The combination of --link and --flatten is broken.
        if opts.copy or opts.link:
            dstdir = (opts.copy if opts.copy else opts.link)
            if not opts.flatten:
                dstdir = os.path.join(dstdir, os.path.basename(component))
                if opts.absolute_paths:
                    dstdir = os.path.abspath(dstdir)
            if opts.link:
                if opts.absolute_paths:
                    links = 'a'
                else:
                    links = 'r'
            else:
                links = ''
            ignpats = ['.git*', '.svn*', '.*.swp'] + opts.skip
            # pylint: disable=star-args
            copyfiles(srcdir, dstdir,
                      flatten=opts.flatten,
                      ignore=shutil.ignore_patterns(*ignpats),
                      links=links,
                      preserve=opts.preserve)

        cfg = read_cfg(srcdir)
        cfg[SUBDIRS].append(srcdir)
        cfg['path'] = srcdir
        compdata.append(cfg)

    if opts.mf:
        if opts.mf == '-':
            mf = sys.stdout
            opts.mf = STDOUT
        else:
            mf = open(opts.mf, 'w')
    else:
        sys.exit(0)

    if opts.hacked:
        sys.stderr.write('+ cp %s %s\n' % (opts.hacked, opts.mf))
        shutil.copyfileobj(open(opts.hacked, 'r'), mf)
        sys.exit(0)

    mk = Makefile(propagate=True)

    if opts.debug:
        mk.add('# Debug verbosity')
        wrn = "MAKING '$@' from $(CURDIR) with WLMAKE_MFDIR=$(WLMAKE_MFDIR)"
        mk.add('SHELL=$(warning %s)/bin/sh' % wrn)
        mk.add('$(info + $(%sCMDLINE))' % PFX)
        mk.add('$(info =-=-= MAKEFLAGS=$(MAKEFLAGS))')
        mk.add('$(info =-=-= MAKEFILES=$(MAKEFILES))')
        mk.add('$(info =-=-= MAKEFILE_LIST=$(MAKEFILE_LIST))')

    mk.add(Variable('PATHBASE',
                    (opts.base if opts.base else os.getcwd())))

    mk.add(Variable('COMPONENTS', components))

    srcfiles_var = Variable('SRCFILES')
    ipath_var = Variable('IPATH')
    srcdirs_var = Variable('SRCDIRS')

    mk.add(srcfiles_var)
    mk.add(ipath_var)
    mk.add(srcdirs_var)

    if opts.flatten:
        ipath_var.append([IFLAG + '.'])
        srcdirs_var.append(['.'])
    else:
        for cfg in compdata:
            path = cfg['path']
            srcfiles_var.append(cfg.get(SRCFILES))
            ipfx = IFLAG + path + os.sep
            iflags = [ipfx + i for i in cfg.get(PUBLIC)]
            ipath_var.append(iflags)

            if PRIVATE in cfg and SRCFILES in cfg:
                ofiles = [os.path.splitext(os.path.basename(c))[0] + '.o'
                          for c in cfg[SRCFILES]]
                common = os.path.commonprefix(ofiles)
                tgt = '.%s%%.c.depend %s%%.o' % (common, common)
                tgt += ' .%s.c.depend %s.o' % (common, common)
                tiflags = [ipfx + i for i in cfg.get(PRIVATE)]
                tgt_var = Variable('IPATH', value=tiflags, style='+', tgt=tgt)
                mk.add(tgt_var)

            srcmap = {}
            for src in cfg.get(SRCFILES, []):
                folder, name = os.path.split(src)
                if folder in srcmap:
                    srcmap[folder].append(name)
                else:
                    srcmap[folder] = [name]
            src_var = Variable('IPATH', value=tiflags, style='+', tgt=tgt)
            cwd = os.getcwd()
            for folder in srcmap:
                mk.add('')
                for name in srcmap[folder]:
                    stem, _ = os.path.splitext(name)
                    tgt = '%s/%s.o: %s +=' % (cwd, stem, src_var.name)
                    if opts.debug:
                        tgt += ' %sTTTTTT+' % IFLAG
                    tgt += ' %s%s/%s' % (IFLAG, path, folder)
                    if opts.debug:
                        tgt += ' %sTTTTTT-' % IFLAG
                    mk.add(tgt)

            # Need a vpath for each dir containing source files.
            srcdirs_var.append([path + os.sep + i
                                for i in sorted(srcmap.keys())])

        if LITERAL in cfg:
            mk.add('\n' + cfg[LITERAL])

    if opts.ivar:
        val = '$(%s)' % ipath_var.name
        if opts.debug:
            val = '%s%sDEBUG+ %s %s%sDEBUG-' % (IFLAG, PFX, val, IFLAG, PFX)
        mk.add(Variable(opts.ivar, value=val, naked=True, style='+'))

    for pattern in opts.vpat:
        mk.add('\nvpath %s $(WLMAKE_SRCDIRS)' % pattern)

    if opts.mf != STDOUT:
        mk.add('\n# Regeneration prerequisites')
        mk.add('%s: \\' % opts.mf)
        for i, cfg in enumerate(compdata):
            configfile = os.path.join(cfg['path'], CFG)
            ending = ('' if compdata[i] == compdata[-1] else ' \\')
            mk.add('%s%s%s' % (INDENT, configfile, ending))

    cmnt = 'Suppress dev hack in WLAN_Common.mk'
    mk.add(Variable('PHY_MOD_SRC_DIRS', naked=True, comment=cmnt))
    mk.add(Variable('PHY_MOD_INC_DIRS', naked=True, comment=cmnt))
    mk.add(Variable('PHY_SRC_INC_DIRS', naked=True, comment=cmnt))

    # Write out the makefile.
    mk.dump(mf)

    # Create a flag that the including makefile can check for success.
    mf.write('\n# The makefile was generated and read successfully\n')
    mf.write('%s := 1\n' % os.path.basename(opts.mf))

if __name__ == '__main__':
    main(sys.argv)

# vim: ts=8:sw=4:tw=80:et:
