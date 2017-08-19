"""Operations related to checkouts (aka tree population)."""

from __future__ import print_function
from __future__ import unicode_literals

import os
import re
import string
import sys
import urlparse

import lib.consts
import lib.opts
import lib.scm
import lib.util

# Checkout parallelism shouldn't be a function of cpu count because
# it's not cpu-bound. The limiting factors are (a) the number of URLS
# which make sense to handle in parallel - essentially, the number of
# components involved - and (b) the ability of the repo to handle parallelism.
# When this was first enabled with jobs=24, nightly builds would die because
# the repo simply couldn't handle N simultaneous builds times 24 simultaneous
# connections. This value seems to not overwhelm the server side and in most
# cases the number of "large" components is <= 4 anyway.
DEFAULT_JOBS = 4

QS_TAG = 'tag'
QS_TO = 'to'
SPARSEFILE_URL = '.svn/sparsefile_url.py'
DOT_GCLIENT = '.gclient'
UC_REPO_ROOT_VAR = lib.consts.UC_REPO_ROOT_EV.lower()
WL_REPO_ROOT_VAR = lib.consts.WL_REPO_ROOT_EV.lower()

# These are for gclient/historical compatibility only.
SRC_REPO_ROOT_VAR = 'src_repo_root'
SRC_REPO_ROOT = '${%s}' % SRC_REPO_ROOT_VAR
TDX_REPO_ROOT_VAR = 'threadx_repo_root'
TDX_REPO_ROOT = '${%s}' % TDX_REPO_ROOT_VAR

DOT_GCLIENT_TMPL = string.Template('''
  { "name"        : "deps/${deps_name}",
    "url"         : "${deps_file}",
    "deps_file"   : "DEPS",
    "custom_deps" : {
    },
    "safesync_url": "",
    "custom_vars" : {     '%s': '${%s}',
      '%s': '${%s}',
      '%s': '${%s}'},
  },''' % (SRC_REPO_ROOT_VAR,
           SRC_REPO_ROOT_VAR,
           TDX_REPO_ROOT_VAR,
           TDX_REPO_ROOT_VAR,
           WL_REPO_ROOT_VAR,
           WL_REPO_ROOT_VAR))

SVN_REV = '_SUBVERSION_REVISION'
GIT_REV = '_GIT_COMMIT'

GIT_CO = 'co'

SVN_CO = 'co'
SVN_EXPORT = 'export'
SVN_LS = 'ls'
SVN_MKDIR = 'mkdir'
SVN_SHOW_ = '_show_components_'
SVN_UP = 'up'


def parse_url(url):
    """Parse and clean up an SCM URL."""
    parsed = urlparse.urlparse(url)
    qstrings = urlparse.parse_qs(parsed.query)

    # Hokey code for putting the URL back together and normalizing it.
    base = urlparse.urlunparse(parsed).split('?')[0]
    if '://' in base:
        prefix, upath = base.split('://', 1)
        base_url = '://'.join([prefix, lib.util.mkpath(upath)])
    else:
        base_url = base

    return base_url, qstrings


def break_down_url(url, reftime=None, sparse=True):
    """Split up an SCM URL and return its contents."""
    if lib.util.is_url(url):
        base_url, qstrings = parse_url(url)
        subdir = qstrings.get(QS_TO, [''])[0]

        # Build up the 'svn cat' command used to read the DEPS or sparse.
        cmd = lib.scm.svncmd('cat')
        depsurl = '/deps/' in base_url
        if '@' in base_url:
            if depsurl and 'DEPS' not in base_url:
                cmd.append(base_url.replace('@', '/DEPS@', 1))
            else:
                cmd.append(base_url)
        else:
            if depsurl and 'DEPS' not in base_url:
                cmd.append(base_url + '/DEPS')
            else:
                cmd.append(base_url)

            if reftime:
                cmd[-1] += '@' + reftime

        # To make the WCC transition from sparse to deps easier
        # we implement a fallback mechanism: if the request is for a
        # deps file but no deps exists, try switching to the equivalent
        # sparse file. This allows GUB.yaml to specify the deps file
        # across the board but it will be used only on branches where
        # it actually exists. When sparse is retired this can go away.
        if depsurl and sparse and not lib.scm.urlexists(cmd[-1]):
            spurl = cmd[-1].replace('components/deps', 'proj', 1)
            spurl = spurl.replace('/DEPS', '.sparse', 1)
            if lib.scm.urlexists(spurl):
                cmd[-1] = base_url = spurl

        proc = lib.util.execute(cmd, stdout=lib.util.PIPE, vl=2)
        contents = proc.communicate()[0]
        if proc.returncode:
            raise lib.util.CalledProcessError(proc.returncode, cmd)
    else:
        pfx = 'file://localhost'
        tmpbase, qstrings = parse_url(lib.util.mkpath(pfx, url))
        subdir = qstrings.get(QS_TO, [''])[0]
        base_url = tmpbase[len(pfx):]
        with open(base_url, 'r') as f:
            contents = f.read()

    return base_url, subdir, contents.decode('utf-8')


# This class feels badly designed. I suspect there are too many
# special case attributes (stitch, meta, leaf, etc). It might benefit
# from a clean rethink. On the other hand, if we switch to git then
# much of what's here becomes legacy code.
class Folder(object):

    """Manage the population from SCM of a single directory."""

    def __init__(self, relpath, url,
                 basedir='',
                 depth=None,
                 meta=True,
                 question=None,
                 stitch=None,
                 subdir='',
                 urlrev=None):
        self.relpath = lib.util.mkpath(relpath)
        self.url = url
        self.basedir = basedir
        self.depth = (depth if depth else 'infinity')
        self.meta = meta
        self.question = question
        self.stitch = stitch
        self.gittag = None
        if url:
            self.url, qstrings = parse_url(url)
            self.subdir = qstrings.get(QS_TO, [''])[0]
        else:
            self.url = None
            self.subdir = subdir

        self.is_git = self.url and lib.scm.is_git(self.url)
        self.is_svn = not self.is_git

        if self.url and self.is_git:
            self.gittag = qstrings.get(QS_TAG, [''])[0]

        self.urlrev = urlrev
        self.rootpath = lib.util.mkpath(self.basedir, self.subdir)
        self.path = lib.util.mkpath(self.rootpath, self.relpath)

        self.leaf = True
        self.parent = None
        self.verb = None

    def merge(self, previous):
        """Combine two objects referring to the same local directory."""
        if self.depth != previous.depth and self.relpath != '.':
            if previous.depth == 'infinity' or self.depth != 'infinity':
                self.depth = previous.depth

        self.stitch = (self.stitch if self.stitch else previous.stitch)
        if self.url and previous.url:
            if not (self.is_git or previous.is_git):
                assert self.url == previous.url
            if self.urlrev and previous.urlrev:
                assert self.urlrev == previous.urlrev

        self.verb = (previous.verb if previous.verb else self.verb)
        return self

    def is_co(self):
        """Return True if this represents a checkout."""
        if self.is_git:
            return True
        elif not self.url:
            return False
        elif not self.stitch:
            return True
        elif os.path.basename(self.path) != self.stitch:
            return False
        else:
            return True

    def has_co(self):
        """Return True if a parent checkout exists to be extended via 'up'."""
        if self.is_git:
            return True
        elif self.question is None and \
                os.path.isdir(os.path.join(self.path, '.svn')):
            return True
        elif self.parent:
            return self.parent.has_co()
        else:
            return False

    def svnverb(self):
        """Determine the svn command required."""
        if self.verb:
            verb = self.verb
        elif isinstance(self.question, list):
            verb = SVN_LS
        elif isinstance(self.question, dict):
            verb = SVN_SHOW_
        elif not self.url:
            if self.stitch and self.meta and self.has_co():
                verb = SVN_UP
            else:
                verb = SVN_MKDIR
        elif not self.meta and (not self.stitch or not
                                (self.path.endswith('/src') and
                                 self.path.count('/src') == 1)):
            verb = SVN_EXPORT
        elif self.stitch and self.has_co():
            verb = SVN_UP
        else:
            verb = SVN_CO

        # TODO insert warning/error for sparse trees outside of /proj/src.
        return verb

    @property
    def infinite(self):
        """Query whether this folder includes all subfolders."""
        return self.depth == 'infinity'

    def _fill_svn(self, preserve=False, reftime=None):
        """Populate this directory from Subversion."""
        verb = self.svnverb()

        cmd = lib.scm.svncmd(verb)

        if verb == SVN_LS:
            if self.url:
                cmd.append('--depth=' + self.depth)
                if reftime and not self.urlrev:
                    cmd.append('@'.join([self.url, reftime]))
                else:
                    cmd.append(self.url)
                proc = lib.util.execute(cmd, stdout=lib.util.PIPE, vl=1)
                for line in proc.stdout:
                    path = line.decode('utf-8').rstrip()
                    if not path.endswith('/'):
                        path = os.path.join(self.rootpath, self.relpath, path)
                        self.question.append(os.path.normpath(path))
                if proc.wait():
                    sys.exit(2)
            return
        elif verb == SVN_SHOW_:
            if self.is_co():
                self.question[self.path] = self.url
            return
        elif verb == SVN_MKDIR:
            lib.util.mkdir(self.path, vl=3)
            return

        if not lib.util.verbose_enough(5):
            cmd.append('-q')

        cmd.append('--depth=' + self.depth)

        if verb == SVN_EXPORT:
            cmd.append('--force')

        if verb == SVN_UP:
            cmd.extend(['-r', self.urlrev if self.urlrev else reftime])
        else:
            lib.util.mkdir(os.path.dirname(self.path), vl=3)
            if preserve and verb == SVN_CO:
                # This option can only be used with co because updating to
                # commit times could confuse make's timestamp analysis.
                cmd.extend(['--config-option',
                            'config:miscellany:use-commit-times=yes'])
            if self.url:
                if not reftime or self.urlrev or lib.scm.is_static(self.url):
                    cmd.append(self.url)
                else:
                    cmd.append('@'.join([self.url, reftime]))

        cmd.append(self.path)
        return cmd

    def _fill_git(self, pfx=None, reftime=None):
        """Populate this directory from Git."""
        # TODO this function misses some parallelism opportunities
        # but not worth fixing until we make real use of git.

        reftime = reftime  # suppress warning until this is in use

        if isinstance(self.question, list):
            if os.path.exists(self.url):
                lsclone = self.url
            else:
                lsclone = lib.util.tmpdir(prefix='pop.git.')
                cmd = lib.scm.gitcmd()
                cmd.extend(['clone', '-q', '--depth=1'])
                cmd.extend([self.url, lsclone])
                lib.util.execute(cmd, check=True, pfx=pfx, vl=1)

            cmd = lib.scm.gitcmd() + ['ls-files']
            proc = lib.util.execute(cmd, cwd=lsclone, pfx=pfx,
                                    stdout=lib.util.PIPE, vl=1)
            for line in proc.stdout:
                sys.stdout.write(os.path.join(self.path, line.decode('utf-8')))
            if proc.wait():
                raise lib.util.CalledProcessError(proc.returncode, cmd)
            return
        elif isinstance(self.question, dict):
            self.question[self.path] = self.url
            return

        cmd = lib.scm.gitcmd()
        if self.meta:
            cmd.extend(['clone', '-q'])
            if self.gittag:
                cmd.extend(['-b', self.gittag])
            cmd.extend([self.url, self.path])
            lib.util.execute(cmd, check=True, pfx=pfx, vl=1)
        else:
            if os.path.isdir(os.path.normpath(self.url)):
                cmd.append('--git-dir=' + os.path.normpath(self.url))
                cmd.append('--work-tree=' + self.path)
                cmd.extend([GIT_CO, '-fq'])
                if self.gittag:
                    cmd.append(self.gittag)
                lib.util.mkdir(self.path)
                lib.util.execute(cmd, check=True, pfx=pfx, vl=1)
            else:
                # Doing a clone just to remove the repo immediately
                # is inelegant but it seems the only option.
                cmd.extend(['clone', '-q', '--depth=1'])
                if self.gittag:
                    cmd.extend(['-b', self.gittag])
                cmd.extend([self.url, self.path])
                lib.util.execute(cmd, check=True, pfx=pfx, vl=1)
                lib.util.rm(os.path.join(self.path, '.git'), vl=2)

    def fill(self, preserve=False, reftime=None):
        """Populate this directory appropriately."""
        if self.is_git:
            return self._fill_git(reftime=reftime)
        else:
            return self._fill_svn(preserve=preserve, reftime=reftime)

    def __repr__(self):
        fmt = '%-12s%s\n'
        rep = fmt % ('path:', self.path)
        if self.parent:
            rep += fmt % ('parent:', self.parent.path)
        else:
            rep += fmt % ('parent:', 'None')
        for key in sorted(self.__dict__):
            if key != 'path' and key != 'parent':
                rep += fmt % (key + ':', self.__dict__[key])
        return rep


class Tree(object):

    """
    Manage the population from SCM of an entire tree.

    This class can also run in 'question mode' in which it does not
    do any checkouts but simply parses the input and returns data.
    When the 'question' is a list it will do an 'svn ls' (or git
    equivalent) on each component and return the list containing all
    files found. Thus question=[] produces a file list, similar to
    doing an actual checkout followed by 'find . -type f -print' but
    much more efficient. With question={} it will parse the DEPS file(s)
    and return the resulting component map in the dict.
    """

    def __init__(self, basedir=None, meta=True, question=None):
        self.basedir = basedir
        self.meta = meta
        self.question = question
        self.dirmap = {}
        self.sparsefiles = {}
        self.depsfiles = {}
        self.success = True
        self.prepped = False
        self.notes = []

    def write_metadata(self, reftime=None):
        """Create metadata for compatibility with the gclient/sparse tools."""
        for subdir, sparsefiles in self.sparsefiles.items():
            svndir = os.path.join(self.basedir, subdir, '.svn')
            if os.path.isdir(svndir):
                cfg_file = os.path.join(self.basedir, subdir, SPARSEFILE_URL)
                if len(sparsefiles) == 1:
                    with open(cfg_file, 'w') as f:
                        f.write("'%s'\n" % os.path.basename(sparsefiles[0]))
                else:
                    app = os.path.join(self.basedir, subdir, 'appended.sparse')
                    with open(cfg_file, 'w') as f:
                        f.write("'%s'\n" % os.path.basename(app))
                    with open(app, 'w') as f:
                        f.write('# generated by ' + lib.util.cmdline())
                        f.write('\n[\n')
                        for path in self.folder_list():
                            fldr = self.folder(path)
                            if fldr.depth != 'empty':
                                key = "('%s/'," % fldr.relpath
                                f.write("    %-62s '%s'),\n" %
                                        (key, fldr.depth))
                        f.write(']\n')

        for subdir, depsfiles in self.depsfiles.items():
            gclient = os.path.normpath(
                os.path.join(self.basedir, subdir, DOT_GCLIENT))
            depsdir = os.path.join(os.path.dirname(gclient), 'deps')
            lib.util.mkdir(depsdir)
            for depsfile in depsfiles:
                depsname = os.path.basename(depsfile)
                if os.path.isfile(depsfile):
                    depsto = os.path.join(depsdir, depsname)
                    lib.util.mkdir(depsto)
                    lib.util.cp(depsfile, os.path.join(depsto, 'DEPS'))
                    continue
                cmd = lib.scm.svncmd(SVN_CO)
                if not lib.util.verbose_enough(5):
                    cmd.append('-q')
                cmd.append('--depth=infinity')
                if reftime:
                    cmd.extend(['-r', reftime])
                cmd.append(depsfile)
                cmd.append(os.path.join(depsdir, depsname))
                lib.util.execute(cmd, check=True, vl=1)

            with open(gclient, 'w') as f:
                # f.write('# Created by "%s scm co".\n' % lib.consts.PROG)
                f.write('solutions = [')
                for depsfile in depsfiles:
                    f.write(
                        DOT_GCLIENT_TMPL.substitute(
                            deps_file=depsfile,
                            deps_name=os.path.basename(depsfile),
                            src_repo_root=lib.consts.WL_REPO_ROOT,
                            threadx_repo_root=lib.consts.WL_REPO_ROOT,
                            wl_repo_root=lib.consts.WL_REPO_ROOT,
                        )
                    )
                f.write('\n]\n')

            # Write out the .gclient_info file which is used by epivers.
            with open(gclient + '_info', 'w') as f:
                f.write("DEPS='" + '/DEPS '.join(depsfiles) + "/DEPS'\n")

        # Touch this marker file to indicate the base of the checkout.
        with open(os.path.join(self.basedir, lib.scm.WLBASE), 'w') as f:
            f.write('This marks the base of the build tree.\n')

    def add_url(self, url, depth=None, relpath='.', stitch=None, urlrev=None):
        """Add a URL to the tree."""
        fldr = Folder(relpath, url,
                      basedir=self.basedir,
                      depth=depth,
                      meta=self.meta,
                      question=self.question,
                      stitch=stitch,
                      urlrev=urlrev)
        if fldr.path in self.dirmap:
            self.dirmap[fldr.path] = fldr.merge(self.dirmap[fldr.path])
        else:
            self.dirmap[fldr.path] = fldr

    # pylint: disable=unused-argument
    def add_sparse(self, sparsefile, reftime=None, vl=-1):
        """Add URLs from a sparse file."""
        sparsefile, todir, sparsedata = break_down_url(sparsefile, reftime)
        lib.util.assert_(sparsedata, 'empty sparse file %s' % sparsefile)
        self.notes.append('checkout from sparse file ' + sparsefile)

        if todir in self.sparsefiles:
            self.sparsefiles[todir].append(sparsefile)
        else:
            self.sparsefiles[todir] = [sparsefile]

        for entry in eval(sparsedata):  # pylint: disable=eval-used
            relpath, depth = entry
            url = re.sub(r'/[^/]+$', '/' + relpath, sparsefile)
            qs = '?%s=%s' % (QS_TO, todir)
            self.add_url(url + qs,
                         depth=depth,
                         relpath=relpath,
                         stitch='src')

    def add_deps(self, depsfile, single=True, reftime=None, vl=-1):
        """Add URLs from a DEPS file."""
        # The parsing below is ug-ly. I feel like there's an elegant
        # method hiding around here someplace but haven't found it yet.

        class DepVars(string.Template):

            """Allow deps vars to use leading numerics."""

            idpattern = r'[_a-z0-9]+'

        # Automatically switch to sparse if no deps file found.
        # Transitional while we use deps on some branches and sparse on others.
        depsfile, todir, depsdata = break_down_url(depsfile, reftime)
        lib.util.assert_(depsdata, 'empty DEPS file %s' % depsfile)
        if '.sparse' in depsfile:
            if self.question is None or lib.util.verbose_enough(3):
                lib.util.note('no DEPS file found - using sparse file instead')
            self.add_sparse(depsfile, reftime=reftime, vl=vl)
            return

        vars_dict, deps_dict, _ = lib.scm.parse_deps(depsdata)

        # Always override a repo configured in the DEPS file with
        # one supplied in the environment if present or the preferred
        # local version if not.
        # There are two orthogonal cases where the default wl_repo_root
        # might be overridden: (1) in a non-SJ site where we want to
        # switch to the local repo for performance and (2) when working
        # with a "user branch", in which case we want to stay at the
        # same site while appending "/users/$LOGNAME/<foobar>". Both
        # of these could happen at once, i.e. someone could work on
        # a user branch in BLR.
        for repo_var in (UC_REPO_ROOT_VAR, WL_REPO_ROOT_VAR):
            if repo_var in vars_dict:
                oldroot = vars_dict[repo_var].rstrip('/')
                if repo_var == UC_REPO_ROOT_VAR:
                    newroot = os.environ.get(
                        repo_var.upper(), lib.consts.UC_REPO_ROOT).rstrip('/')
                else:
                    newroot = os.environ.get(
                        repo_var.upper(), lib.consts.WL_REPO_ROOT).rstrip('/')
                oldroot = oldroot.split('/users/', 1)[0]
                # In case a user modified their DEPS file by hand already.
                # We don't want to add the users path twice.
                undone = not [v for v in vars_dict.values() if 'users/' in v]
                if newroot != oldroot and undone:
                    if not newroot.startswith('svn://'):
                        lib.util.note('overriding %s with %s' % (
                            oldroot, newroot), vl=0)
                    vars_dict[repo_var] = newroot
                    newbase = newroot.split('/users/', 1)[0]
                    depsfile = depsfile.replace(oldroot, newbase)

        # More general version of above: allow any "foobar" var to be
        # overridden in the env by exporting "DEPS_VARS_FOOBAR".
        for v in vars_dict:
            ev = 'DEPS_VARS_' + v.upper()
            if ev in os.environ:
                vars_dict[v] = os.environ[ev]

        spec = (depsfile if 'DEPS' in depsfile else depsfile + '/DEPS')
        if reftime and lib.util.is_url(spec):
            spec += '@' + reftime
        self.notes.append('checkout from ' + spec)

        if todir in self.depsfiles:
            self.depsfiles[todir].append(depsfile)
        else:
            self.depsfiles[todir] = [depsfile]

        # Here we walk through the deps dict, doing Var() substitutions
        # on first the key (local directory), then the value (SCM URL).
        qs = '?%s=%s' % (QS_TO, todir)
        for relpath in deps_dict.keys():
            while True:
                repl = DepVars(relpath).substitute(vars_dict)
                if repl == relpath:
                    break
                else:
                    deps_dict[repl] = deps_dict[relpath]
                    del deps_dict[relpath]
                    relpath = repl

            # Support for these is historical only.
            for parm in [SRC_REPO_ROOT, TDX_REPO_ROOT]:
                if parm in deps_dict[relpath]:
                    lib.util.warn('URL not parameterized with %s: %s'
                                  % (WL_REPO_ROOT_VAR, deps_dict[relpath]))
                    deps_dict[relpath] = deps_dict[relpath].replace(
                        parm, '${%s}' % WL_REPO_ROOT_VAR)

            # Require correct site parameterization for svn repos.
            if single and lib.scm.is_svn(deps_dict[relpath]) and (
                    UC_REPO_ROOT_VAR not in deps_dict[relpath] and
                    WL_REPO_ROOT_VAR not in deps_dict[relpath]):
                lib.util.die('URL incorrectly parameterized: %s'
                             % deps_dict[relpath])

            while True:
                repl = DepVars(deps_dict[relpath]).substitute(vars_dict)
                if repl == deps_dict[relpath]:
                    break
                else:
                    deps_dict[relpath] = repl

            if lib.scm.is_git(deps_dict[relpath]):
                self.add_url(deps_dict[relpath] + qs,
                             depth=None,
                             relpath=relpath,
                             stitch=False,
                             urlrev=None)
            else:
                chunks = deps_dict[relpath].split(':')
                url = ':'.join(chunks[:2])
                flags = (chunks[-1] if len(chunks) > 2 else '')
                if 'depth=files' in flags:
                    depth = 'files'
                elif 'depth=file' in flags:
                    # Special case to export a single file. Not in use
                    # but preserved against potential future need.
                    depth = 'file'
                elif 'depth=empty' in flags:
                    depth = 'empty'
                else:
                    depth = None

                match = re.search(r'@(\d+)$', url)
                urlrev = (match.group(1) if match else None)
                stitch = ('src' if 'path=src' in flags else None)
                self.add_url(url + qs,
                             depth=depth,
                             relpath=relpath,
                             stitch=stitch,
                             urlrev=urlrev)

        self.add_url(None)

    def add(self, item, single=True, reftime=None, vl=-1):
        """Add an item (sparse, deps, or URL) to the checkout set."""
        parts = item.split('?')
        if '.sparse' in item:
            self.add_sparse(item, reftime=reftime, vl=vl)
        elif 'DEPS' in item:
            if lib.util.is_url(item):
                parts[0] = os.path.dirname(parts[0])
                url = '?'.join(parts)
                self.add_deps(url, single=single, reftime=reftime, vl=vl)
            else:
                self.add_deps(item, single=single, reftime=reftime, vl=vl)
        elif '/deps/' in item:
            self.add_deps(item, single=single, reftime=reftime, vl=vl)
        elif lib.util.is_url(item):
            fromurl, _ = parse_url(item)
            spec = (fromurl + '@' + reftime if reftime else fromurl)
            self.notes.append('checkout from single url ' + spec)
            match = re.search(r'@(\d+)$', fromurl)
            urlrev = (match.group(1) if match else None)
            self.add_url(item, urlrev=urlrev)
        elif os.path.isdir(parts[0]):
            self.notes.append('checkout from path repo ' + parts[0])
            self.add_url(item)
        elif os.path.isfile(parts[0]):
            if os.path.basename(parts[0]) == '.gclient':
                contents = open(parts[0], 'r').read()
                gcdict = {}
                exec contents in globals(), gcdict
                for sol in gcdict['solutions']:
                    dps = '/'.join([sol['url'], sol['deps_file']])
                    self.add_deps(dps, single=single, reftime=reftime, vl=vl)
            else:
                self.add_deps(item, single=single, reftime=reftime, vl=vl)
        else:
            lib.util.die('unknown item: ' + item)

    def _complete_tree(self, path=None):
        """Complete the tree by inferring implicit directories."""
        if path:
            ppath = os.path.dirname(path)

            if not ppath or path == self.basedir:
                self.dirmap[path].leaf = False
                return

            if ppath in self.dirmap:
                # Only consider parent dir a parent checkout if URLs match.
                url = self.dirmap[path].url
                if url and self.dirmap[ppath].url:
                    purl = self.dirmap[ppath].url.split('@')[0]
                    if os.path.dirname(url).startswith(purl):
                        self.dirmap[path].parent = self.dirmap[ppath]
                    elif not self.dirmap[path].is_git:
                        lib.util.warn('incongruous component: %s' % url)
                        if self.question is None:
                            self.dirmap[path].verb = SVN_EXPORT
                else:
                    self.dirmap[path].parent = self.dirmap[ppath]
            else:
                child = self.dirmap[path]
                relative_to = lib.util.mkpath(self.basedir, child.subdir)
                relpath = os.path.relpath(ppath, relative_to)
                fldr = Folder(relpath, None,
                              basedir=self.basedir,
                              depth='empty',
                              meta=self.meta,
                              question=self.question,
                              stitch=child.stitch,
                              subdir=child.subdir)
                self.dirmap[fldr.path] = fldr
                self.dirmap[path].parent = fldr
                self._complete_tree(path=ppath)

            if not self.dirmap[path].parent:
                return

            # Special case to make epivers.sh work. The parent directory of
            # ./src/include (thus ./src) must be a checkout for svn info to
            # work correctly.
            if path.endswith('/src'):
                if not self.meta and self.question is None:
                    if path.count('/src') == 1 and self.dirmap[path].url:
                        self.dirmap[path].verb = SVN_CO

            # If it has a child it must not be a leaf node.
            self.dirmap[path].parent.leaf = False
        else:
            for path in self.dirmap.keys():
                self._complete_tree(path=path)

    def _elide_redundancies(self):
        """Find and remove entries subsumed by parent entry."""
        for path in self.dirmap.keys():
            fldr = self.dirmap[path]
            # If this is a leaf node and its parent has "depth=infinite"
            # then it must be redundant.
            if fldr.leaf and fldr.parent and fldr.parent.infinite and \
               fldr.parent.path != fldr.parent.rootpath:
                fldr.parent.leaf = True
                del self.dirmap[path]
                if self.question is None:
                    lib.util.warn('redundant entry "%s" elided' % path)

    def folder_list(self):
        """Return a list of folder paths sorted by length."""
        return sorted(self.dirmap.keys(), key=len)

    def folder(self, path):
        """Return the Folder object associated with the specified path."""
        return self.dirmap.get(path)

    def prep(self):
        """Complete the build tree."""
        if self.prepped:
            return

        # Find directories which are implicitly required to bridge
        # between explicit ones. Once this is done:
        # (1) We have a full tree with a node for each dir.
        # (2) Every node has a reference to its parent.
        # (3) Every node knows whether it's a leaf or interior.
        self._complete_tree()

        # Clean out any sloppiness such as:
        #    "xx/yy: infinity"
        #    "xx/yy/zz: infinity"
        self._elide_redundancies()

        self.prepped = True

    def populate(self, jobs=1, preserve=False, reftime=None):
        """Check out files into the build tree."""
        assert self.basedir

        jobs = (jobs if jobs > 0 else DEFAULT_JOBS)

        if self.question is None:
            for note in self.notes:
                if 'single url' in note:
                    lib.util.note(note, vl=0)
                else:
                    lib.util.note('-j%d %s' % (jobs, note), vl=0)

        # Clean up and flesh out the tree representation.
        self.prep()

        # Populate dirs top down (shortest paths first).
        # Interior svn dirs must be made serially.
        # TODO with export --force it may be possible to relax this.
        # Also, all "svn up" operations must be serial since they operate
        # on a common parent database.
        # Keep a list of remaining cmds which can be run in parallel.
        cmds = []
        for path in self.folder_list():
            fldr = self.folder(path)
            cmd = fldr.fill(preserve=preserve, reftime=reftime)
            if cmd:
                if fldr.is_svn and (not fldr.leaf or fldr.svnverb() == SVN_UP):
                    if lib.util.execute(cmd):
                        self.success = False
                else:
                    cmds.append(cmd)

        # Run remaining cmds in parallel no more than <jobs> at a time.
        if lib.util.execute_parallel(cmds, jobs=jobs, vl=1) != 0:
            self.success = False

        return self.success

    def sync(self, jobs=1, reftime='HEAD', svnupopts=None):
        """Update an existing tree."""
        assert self.basedir

        if svnupopts is None:
            svnupopts = []

        # Create a generic svn update command.
        upcmd = lib.scm.svncmd(SVN_UP)

        paths = {}
        if os.path.isfile(os.path.join(self.basedir, SPARSEFILE_URL)):
            sp_upcmd = upcmd + svnupopts + ['-r', reftime]
            spurl = open(os.path.join(self.basedir, SPARSEFILE_URL)).read()
            spfile = eval(spurl)  # pylint: disable=eval-used
            if spfile == 'appended.sparse':
                lib.util.warn('cannot tell if appended.sparse is up to date')
            else:
                catcmd = lib.scm.svncmd('cat')
                catcmd.append('/'.join([lib.scm.urlof(self.basedir), spfile]))
                proc = lib.util.execute(catcmd, stdout=lib.util.PIPE, vl=3)
                latest = proc.stdout.read()
                if proc.wait():
                    self.success = False
                    return self.success
                current = open(os.path.join(self.basedir, spfile)).read()
                if current == latest:
                    paths[self.basedir] = [sp_upcmd + [self.basedir]]
                else:
                    paths[self.basedir] = [['sparse'] + sp_upcmd[1:] +
                                           [self.basedir]]
                    lib.util.note('%s out of date, using "sparse update"' %
                                  spfile)

            svns = lib.scm.find_components(self.basedir)[0]
            for path in svns:
                if path not in paths:
                    paths[path] = [sp_upcmd + [path]]
                    lib.util.note('custom svn checkout: ' + path)
        elif os.path.isfile(os.path.join(self.basedir, DOT_GCLIENT)):
            # First we must update and then parse all DEPS files
            # in case they've moved ahead to a new version.
            dot_gclient = os.path.join(self.basedir, DOT_GCLIENT)
            contents = open(dot_gclient, 'r').read()
            gcdict = {}
            exec contents in {}, gcdict
            for solution in gcdict.get('solutions', []):
                dpsdir = os.path.join(self.basedir, solution['name'])
                dpspath = os.path.join(dpsdir, 'DEPS')
                dp_upcmd = upcmd + ['-r', reftime]
                if lib.opts.VERBOSITY < 1:
                    dp_upcmd.append('-q')
                if os.path.isfile(dpspath):
                    dp_upcmd.extend(svnupopts)
                    dp_upcmd.append(dpsdir)
                else:
                    dp_upcmd.extend([solution['url'], dpsdir])
                    dp_upcmd[dp_upcmd.index(SVN_UP)] = SVN_CO

                lib.util.execute(dp_upcmd, check=True, vl=2)
                self.add_deps(dpspath)

            # Use a list of checkouts derived from DEPS files.
            for path in self.folder_list():
                if self.dirmap[path].urlrev:
                    rest = ['-r', self.dirmap[path].urlrev, path]
                else:
                    rest = ['-r', reftime, path]

                if os.path.isdir(os.path.join(path, '.svn')):
                    deps_url = self.dirmap[path].url
                    curr_url = lib.scm.urlof(path, errmsgs=True)
                    if not curr_url:
                        self.success = False
                        continue
                    deps_path = lib.scm.svn_url_split(deps_url)[1]
                    curr_base, curr_path = lib.scm.svn_url_split(curr_url)
                    if curr_path != deps_path:
                        # The checkout exists but its URL has changed.
                        swcmd = lib.scm.svncmd('switch') + ['-r', reftime]
                        swcmd.append('--ignore-ancestry')
                        swcmd.append('--force')
                        swcmd.append(''.join([curr_base, deps_path]))
                        swcmd.append(path)
                        paths[path] = [swcmd]
                    else:
                        paths[path] = [upcmd + svnupopts + rest]
                elif os.path.isdir(os.path.join(path, '.git')):
                    paths[path] = lib.scm.git_update(path)
                elif self.dirmap[path].url and not os.path.isdir(path):
                    if self.dirmap[path].depth == 'file':
                        # Deal with individual files right away,
                        # don't bother with parallelism.
                        tmppath = '%s.%d' % (path, os.getpid())
                        cmd = lib.scm.svncmd(SVN_EXPORT)
                        if lib.opts.VERBOSITY < 2:
                            cmd.append('-q')
                        cmd += rest
                        cmd.insert(-1, self.dirmap[path].url)
                        if os.path.isfile(path):
                            cmd[-1] = tmppath
                        if lib.util.execute(cmd, vl=2) == 0:
                            if os.path.isfile(tmppath):
                                latest = open(tmppath).read()
                                if open(path).read() == latest:
                                    lib.util.rm(tmppath, vl=2)
                                else:
                                    lib.util.mv(path, tmppath, vl=2)
                                    with open(path, 'w') as f:
                                        f.write(latest)
                        else:
                            self.success = False
                    else:
                        cocmd = upcmd + rest
                        cocmd[cocmd.index(SVN_UP)] = SVN_CO
                        cocmd.insert(-1, self.dirmap[path].url)
                        paths[path] = [cocmd]

            # Warn if any have been subtracted.
            svns, gits = lib.scm.find_components(self.basedir)[0:2]
            for path in svns + gits:
                # DEPS dirs are already taken care of.
                if os.path.basename(os.path.dirname(path)) != 'deps':
                    if path not in paths:
                        lib.util.warn('obsoleted path: ' + path)
        elif os.path.isdir(os.path.join(self.basedir, '.svn')):
            svns, gits = lib.scm.find_components(self.basedir)[0:2]
            for path in svns:
                paths[path] = [upcmd + svnupopts + ['-r', reftime, path]]
            for path in gits:
                paths[path] = lib.scm.git_update(path)
        elif os.path.isdir(os.path.join(self.basedir, '.git')):
            gits = lib.scm.find_components(self.basedir)[1]
            for path in gits:
                paths[path] = lib.scm.git_update(path)
        else:
            lib.util.warn('unsupported tree type: %s' % self.basedir)
            self.success = False

        # Run all update commands using a process pool. Serial work
        # is treated as just another parallel case with processes=1.

        # Make a list of commands to run. These are done in reverse
        # alphabetical order for the simple reason that our biggest/slowest
        # component will probably always be "src" and we want to get it
        # going asap. The others ("components") tend to be lexically earlier.
        cmds = []
        for path in sorted(paths, reverse=True):
            for cmd in paths[path]:
                cmds.append(cmd)

        # The only problem with running svn update output through this
        # function is that if a merge is required it may appear to hang
        # since stdout is captured.
        def print_output(stdout):
            """Handle the stdout of each parallel job."""
            for line in stdout:
                if not (line.startswith('Updating ') or
                        line.startswith('At revision ')):
                    sys.stdout.write(line)

        # Run remaining cmds in parallel no more than <jobs> at a time.
        func = print_output if lib.opts.VERBOSITY < 1 else None
        jobs = (jobs if jobs > 0 else DEFAULT_JOBS)
        if lib.util.execute_parallel(cmds, func=func, jobs=jobs, vl=2) != 0:
            self.success = False

        return self.success

    def __repr__(self):
        return '\n'.join(self.folder_list())

# vim: ts=8:sw=4:tw=80:et:
