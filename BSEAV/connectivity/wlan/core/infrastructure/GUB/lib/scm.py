"""Abstract away as many SCM-tool variations as possible."""

# TODO: ATM this is a horrible hodgepodge of a module but at least it
# concentrates much of the scm-related horribleness in one place.
# It should get a complete overhaul once stable.

from __future__ import print_function
from __future__ import unicode_literals

import distutils.version
import os
import re
import sys
import time

import lib.consts
import lib.times
import lib.util

PROJ = 'proj'
SVN_WC_DB = os.path.join('.svn', 'wc.db')
WLBASE = '.wlbase'

if 'linux' in sys.platform:
    # This uses <GUB>/bin/svn which is a wrapper that
    # hands off to /tools/bin/svn (if available) after requesting
    # an svn version matching the current working copy (if any).
    # The svn version of a fresh checkout will be determined by
    # SUBVERSIONVER if /tools/bin is available, or whatever is
    # offered by /usr/bin/svn otherwise.
    SVNCMD = ['svn']
else:
    # On non-linux platforms only the native version is available so
    # the wrapper is wasted energy.
    SVNCMD = ['/usr/bin/svn']

GITCMD = ['git', '--no-pager']


def as_builder():
    """
    Return True if we should do svn queries as the privileged builder.

    Since remote users may not keep their SJ svn credentials up to date
    we prefer to do SJ svn queries as builder which is guaranteed up
    to date.  However, if we're currently running at a remote site the
    situation is reversed since builder's credentials are maintained
    only in SJ.
    """

    return lib.consts.SITE == 'sj'


def svncmd(*args, **kwargs):
    """Return an appropriate svn command."""
    # This must be a copy because it may be modified by the caller.
    cmd = SVNCMD[:]

    # The benefit of doing svn queries as the privileged builder is not
    # obvious. We have multiple sites and multiple svn mirrors and most
    # users work primarily with one mirror, which means their cached svn
    # credentials may not be up to date at other sites which would cause
    # queries by them *as themselves* to fail. If we do the same query as
    # builder we can safely assume the credentials are fine. This isn't
    # a security hole since (a) hwnbuild doesn't have write access to svn
    # anyway so it can only query and (b) anyone who can access this setuid
    # wrapper already has svn write privileges. Thus it provides read-only
    # access to people who already have write privileges under their own
    # account; it's just a safer way to do automated queries.
    if kwargs.get('as_builder'):
        if lib.util.path_whence(lib.consts.AS_BUILDER):
            cmd.insert(0, lib.consts.AS_BUILDER)

    # Within automated builds svn should never read from stdin but for uses like
    # "hnd gclient" we should allow first-time svn users to cache passwords etc.
    if not os.isatty(sys.stdin.fileno()):
        cmd.append('--non-interactive')

    cmd.extend(list(args))

    return cmd


def gitcmd(*args):
    """Return an appropriate git command."""
    # This must be a copy because it may be modified by the caller.
    cmd = GITCMD[:]

    cmd.extend(list(args))

    return cmd


def is_static(tag):
    """Determine whether the provided string looks like a static tag."""
    return '_REL_' in os.path.basename(tag)


def is_git(path):
    """Determine whether the provided URL or directory looks like git."""
    if lib.util.is_url(path):
        if '.git' in path or 'https://' in path:
            return True
    elif os.path.isdir(path):
        if os.path.isdir(os.path.join(path, '.git')):
            return True

    return False


def is_svn(path):
    """Determine whether the provided URL or directory looks like svn."""
    if lib.util.is_url(path):
        if '/svn' in path:
            return True
    elif os.path.isdir(path):
        if os.path.isdir(os.path.join(path, '.svn')):
            return True

    return False


def svn_tag_split(url):
    """Break an SVN path apart by the tag path."""
    parts = url.split('/')
    if 'tags' in parts:
        start = parts.index('tags')
        end = start + 3
        tagpath = '/'.join(parts[start:end])
    elif 'branches' in parts:
        start = parts.index('branches')
        end = start + 2
        tagpath = '/'.join(parts[start:end])
    elif 'trunk' in parts:
        tagpath = 'trunk'
    else:
        tagpath = None

    if tagpath:
        pre, post = url.split('/' + tagpath, 1)
        post = post.lstrip('/')
    else:
        pre, post = None, url

    return pre, tagpath, post


def svn_tag_name(url, default=None):
    """Extract the branch/twig/tag name from an SVN url."""
    _, tagpath, _ = svn_tag_split(url)
    name = (os.path.basename(tagpath) if tagpath else default)
    return name


def svn_tag_path(tag):
    """Given a subversion tag name, return a standard tag path."""
    if tag is None:
        result = 'trunk'
    elif is_static(tag):
        result = 'tags/%s/%s' % (tag.split('_')[0], tag)
    elif '_BRANCH_' in tag or '_TWIG_' in tag:
        result = 'branches/%s' % tag
    else:
        result = tag

    return result


def svn_tag_parent(tag, frev):
    """Use svn queries to work out where this tag came from."""
    if not tag or os.path.basename(tag) == 'trunk':
        return None, None, None, None, None

    cmd = svncmd('log', '--stop-on-copy', '-v', as_builder=as_builder())
    if '/' in tag:
        cmd.append(tag)
    else:
        url = '/'.join([lib.consts.WL_REPO_ROOT, PROJ, svn_tag_path(tag)])
        cmd.append(url)

    if frev:
        cmd[-1] += '@' + frev

    proc = lib.util.execute(cmd,
                            stdout=lib.util.PIPE,
                            stderr=lib.util.PIPE,
                            vl=2)
    std_out, std_err = proc.communicate()

    # Ignore a warning that we've gone earlier than the requested revision.
    for line in std_err.decode('utf-8', 'replace').splitlines():
        if 'E160013' not in line:
            sys.stderr.write(line + '\n')

    fromline = revline = None
    for line in std_out.decode('utf-8', 'ignore').splitlines():
        if not line:
            pass
        elif line[0] == 'r' and line.count(' | ') == 3:
            revline = line
        elif line.startswith('   A ') and 'from /' in line:
            fromline = line
        elif line == 'Changed paths:':
            fromline = None

    if proc.returncode == 0 and fromline:
        frm = fromline.split()[-1].split()
        path, frev = frm[-1].split(':')[0:2]
        path = lib.consts.WL_REPO_ROOT + path
        frev = frev.rstrip(')')
        parts = revline.split(' | ')
        trev = parts[0][1:]
        by = parts[1]
        date = ' '.join(parts[2].split()[0:3])
    else:
        path, frev, trev, by, date = None, None, None, None, None

    return path, frev, trev, by, date


def svn_tag_history(tag):
    """Given a tag, derive its branching history back to trunk."""
    brhist = {}
    tpath, rev = tag, None
    if lib.util.is_url(tpath):
        tpath = '/'.join(svn_tag_split(tpath)[0:2])

    while tpath and os.path.basename(tpath) != 'trunk':
        ppath, rev, _, _, date = svn_tag_parent(tpath, rev)
        brhist[os.path.basename(tpath)] = lib.times.iso2seconds(date)
        tpath = ppath

    return brhist


def svn_tag_date(tag):
    """Use svn queries to work out when this tag was made."""
    cmd = svncmd('log', '-l1', as_builder=as_builder())
    if lib.util.is_url(tag):
        cmd += [tag]
    else:
        cmd += ['/'.join([lib.consts.WL_REPO_ROOT, PROJ, svn_tag_path(tag)])]

    proc = lib.util.execute(cmd, stdout=lib.util.PIPE, vl=3)
    revline = None
    for line in proc.stdout:
        if line[0] == 'r':
            revline = line.decode('utf-8', 'ignore')
            break
    if proc.wait() == 0 and revline:
        when = revline.split(' | ')[2].strip()
        return ' '.join(when.split()[0:2])
    else:
        return None


# TODO there should be a Tag() or Branch() class with this comparator.
def svn_tag_cmp(left, right):
    """Compare two branches/tags numerically."""
    vers = []
    for tag in (left, right):
        # Make sure "trunk", "master", etc. sort above all else.
        match = re.search(r'_([\d_]*)$', tag)
        vers.append(match.group(1) if match else '10000.0')
    return cmp(distutils.version.LooseVersion(vers[0]),
               distutils.version.LooseVersion(vers[1]))


def svn_stats(arg, repo):
    """Use an svn query to translate the arg into rev and time."""
    rev, when = -1, -1
    cmd = svncmd('log', '-r%s' % arg, '-q', repo, as_builder=as_builder())
    proc = lib.util.execute(cmd, stdout=lib.util.PIPE, vl=2)
    for line in proc.stdout:
        if line[0] == 'r':
            parts = line.strip().split(' | ', 2)
            rev = int(parts[0][1:])
            # If we have no permission for this rev the result will look like:
            # 'r606465 | (no author) | (no date)'
            try:
                whenstr = '%sT%s%s' % tuple(parts[2].split()[0:3])
                when = lib.times.iso2seconds(whenstr)
            except TypeError as e:
                lib.util.error('"%s": %s' % (parts[2], e))
            break
    return (-1, -1) if proc.wait() else (rev, when)


def svn_find_parent(tag, pattern):
    """Find the most recent parent of a tag which contains the pattern."""
    tpath = svn_tag_parent(tag, None)[0]
    while tpath and not re.search(pattern, tpath):
        tpath = svn_tag_parent(tpath, None)[0]
    return tpath


def svn_expand_tag(tag, cfgroot=None, reftime='HEAD', stderr=None):
    """
    Find a full tag name by tail-matching the numeric part.

    This is done by first looking for an active tag in the config file,
    because that's fast. If not listed there we search in svn.
    """
    tag = tag.strip('/').replace('.', '_')
    if '_BRANCH_' in tag or '_TWIG_' in tag or '_REL_' in tag:
        candidates = [tag]
    elif cfgroot:
        candidates = cfgroot.expand_tag(tag, unique=False)
    else:
        candidates = []
    if not candidates:
        url = '/'.join([lib.consts.WL_REPO_ROOT_RO, PROJ, 'branches'])
        for branch in svn_ls(url, reftime=reftime, stderr=stderr):
            if branch.endswith(tag):
                candidates.append(branch)
    if len(candidates) == 1:
        return candidates[0]
    elif not candidates:
        return tag
    else:
        lib.util.die('ambiguous tag match: (%s)' % ', '.join(candidates))


def svn_ls(path, reftime='HEAD', stderr=None):
    """Invoke an 'svn ls' command and return its output."""
    cmd = svncmd('ls', '-r', reftime, path)
    proc = lib.util.execute(cmd, stdout=lib.util.PIPE, stderr=stderr, vl=3)
    output = proc.communicate()[0].decode('utf-8', 'strict')
    return sorted([os.path.normpath(p) for p in output.splitlines()])


def svn_cat(*args):
    """Invoke an 'svn cat' command and return its output."""
    cmd = svncmd('cat', *args)
    proc = lib.util.execute(cmd, stdout=lib.util.PIPE, vl=3)
    output = proc.communicate()[0].decode('utf-8', 'strict')
    return output


def svn_ci(path, *args):
    """Invoke an 'svn checkin' command and return its exit status."""
    cmd = svncmd('ci', *args) + [path]
    rc = lib.util.execute(cmd, vl=3)
    return rc


def svn_co(url, path, *args):
    """Invoke an 'svn checkout' command and return its output."""
    cmd = svncmd('co', url, path, *args)
    proc = lib.util.execute(cmd, stdout=lib.util.PIPE, vl=3)
    output = proc.communicate()[0].decode('utf-8', 'strict')
    return output


def svn_diff(filename):
    """Invoke an 'svn diff' command and return its exit status."""
    cmd = svncmd('diff', filename)
    rc = lib.util.execute(cmd, vl=-1)
    return rc


# This uses a simple recursive algorithm to create parents
# (if we could assume svn 1.7+ we may be able to use --parents instead).
def svn_updir(path, reftime='HEAD'):
    """Create a sparse directory in a workspace via svn update."""
    if not os.path.exists(path):
        parent = os.path.dirname(path)
        if not os.path.exists(parent):
            svn_updir(parent, reftime=reftime)
        upcmd = svncmd('up', '-q', '--depth=empty', '-r', reftime)
        lib.util.execute(upcmd + [path], check=True, vl=2)


def svn_mkdir(path):
    """Create a directory in a workspace via svn mkdir."""
    if not os.path.isdir(path):
        mdcmd = svncmd('mkdir', '-q', '--parents', path)
        lib.util.execute(mdcmd, check=True, vl=2)


def svn_revision(url):
    """Return the canonical revision of the provided URL."""
    cmd = svncmd('info', url)
    proc = lib.util.execute(cmd, stdout=lib.util.PIPE, vl=3)
    output = proc.communicate()[0]
    rev = 0
    if proc.returncode == 0:
        for line in output.splitlines():
            if line.startswith('Revision: '):
                rev = int(line.rstrip().split(' ', 1)[1])
                break
    return rev


def svn_wait(master=lib.consts.WL_REPO_ROOT,
             mirror=lib.consts.WL_REPO_ROOT_RO):
    """Block until the mirror repo has caught up with the master."""
    if mirror != master:
        ref = svn_revision(master)
        if ref:
            for delay in range(0, 32, 2):
                rev = svn_revision(mirror)
                if rev >= ref:
                    return
                if delay:
                    lib.util.note('sleeping %d for %s' % (delay, mirror))
                    time.sleep(delay)
        lib.util.die('not in sync with master: %s' % mirror)


def svn_url_split(url):
    """Break an svn URL into repo and path sections."""
    marker = '/svn-sj1/' if '/svn-sj1/' in url else '/svn/'
    parts = url.split(marker, 1)
    return parts[0] + marker, parts[1]


def git_update(path):
    """Return the commands necessary to update specified git repo."""
    # First we do a "git co ." to fix any missing files. This has to happen
    # first because "git pull --rebase" refuses to work if there are any
    # local mods. Then we do the pull/rebase, then we do _another_ "git co"
    # to make sure any changes pulled are reflected in the work tree.
    gitbase = gitcmd('--git-dir=%s/.git' % path, '--work-tree=%s' % path)
    cmds = [
        gitbase + ['checkout', '.'],
        gitbase + ['pull', '--rebase'],
        gitbase + ['checkout', '.'],
    ]

    return cmds


def find_tree_base(path='.'):
    """Find the base of the current checkout."""
    curdir = os.path.realpath(path)
    checkout = tree = None
    while True:
        parent = os.path.dirname(curdir)
        if os.stat(parent).st_ino == os.stat(curdir).st_ino:
            break
        elif os.path.exists(os.path.join(curdir, WLBASE)):
            tree = curdir
            break
        elif os.path.exists(os.path.join(curdir, '.gclient')):
            tree = curdir
            break
        elif os.path.exists(os.path.join(curdir, '.svn/sparsefile_url.py')):
            tree = curdir
            break
        elif not checkout:
            if os.path.exists(os.path.join(curdir, '.svn')):
                if os.path.exists(os.path.join(curdir, '.svn/wc.db')):
                    checkout = curdir
                elif not os.path.exists(os.path.join(curdir, '../.svn')):
                    # Backward compatibility for svn 1.6.
                    checkout = curdir
            elif os.path.exists(os.path.join(curdir, '.git')):
                checkout = curdir

        curdir = parent

    if tree:
        return tree
    else:
        return checkout


def cd_to_tree_base(exc=False):
    """Chdir to the root of the SCM tree if possible."""
    treebase = find_tree_base()
    if treebase:
        lib.util.chdir(treebase)
    else:
        lib.util.die('no SCM basedir found', exc=exc)
    return treebase


def urlexists(url, tag=None):
    """Return True if the SCM URL exists."""
    if not lib.util.is_url(url):
        repo = lib.consts.WL_REPO_ROOT
        if tag:
            url = '/'.join([repo, PROJ, svn_tag_path(tag), url])
        else:
            url = '/'.join([repo, url])
    cmd = svncmd('ls', url)
    return lib.util.execute(cmd,
                            stdout=open(os.devnull, 'wb'),
                            stderr=open(os.devnull, 'wb'),
                            vl=3) == 0


def urlof(path, errmsgs=False):
    """Return the SCM URL from which the path came."""
    url = None
    cmd = svncmd('info', path)
    stderr = None if errmsgs else open(os.devnull, 'w')
    proc = lib.util.execute(cmd, stdout=lib.util.PIPE,
                            stderr=stderr, vl=3)
    output = proc.communicate()[0]
    if proc.returncode == 0:
        for line in output.splitlines():
            if line.startswith('URL: '):
                url = line.rstrip().split(' ', 1)[1]
                break

    return url


def find_components(basedir, search_up=0):
    """Return sets of checkouts found (svn and git)."""
    svns = set()
    gits = set()
    others = set()
    for path, dnames, _ in os.walk(basedir):
        npath = os.path.normpath(path)
        if '.svn' in dnames or '.git' in dnames:
            if '.svn' in dnames:
                svns.add(npath)
            else:
                gits.add(npath)

            # Now we know this directory is versioned, remove its
            # parents retroactively from the unversioned set.
            while npath.startswith(basedir):
                npath = os.path.dirname(npath)
                others.discard(npath)

            # Go no deeper.
            dnames[:] = []
        else:
            if npath != basedir:
                if not [p for p in others if npath.startswith(p)]:
                    others.add(npath)

    if not svns and not gits and search_up:
        curdir = basedir
        while True:
            parent = os.path.normpath(os.path.join(curdir, '..'))
            if os.path.isdir(os.path.join(parent, '.svn')):
                svns.add(parent if search_up > 1 else basedir)
                others = set()
                break
            elif os.path.isdir(os.path.join(parent, '.git')):
                gits.add(parent if search_up > 1 else basedir)
                others = set()
                break
            elif os.stat(parent).st_ino == os.stat(curdir).st_ino:
                break
            curdir = parent

    return sorted(svns), sorted(gits), sorted(others)


def get(paths, cwd=None, reftime=None, tag=None, umask=None, vl=2):
    """Extract a list of pathnames from SCM."""
    if tag:
        if lib.util.is_url(tag):
            base = tag
        else:
            base = '/'.join([lib.consts.WL_REPO_ROOT, PROJ, svn_tag_path(tag)])
    elif tag is None:
        base = None
        url = urlof('.')
        if url:
            parts = url.split('/')
            if 'tags' in parts:
                base = '/'.join(parts[:parts.index('tags') + 3])
            elif 'branches' in parts:
                base = '/'.join(parts[:parts.index('branches') + 2])
            elif 'trunk' in parts:
                base = '/'.join(parts[:parts.index('trunk') + 1])
    else:
        base = lib.consts.WL_REPO_ROOT

    if not base:
        return 2

    rc = 0
    for path in paths:
        path = os.path.normpath(path)
        url = '/'.join([base, path])
        if reftime:
            url += '@' + reftime
        if not urlexists(url):
            url = '/'.join([lib.consts.WL_REPO_ROOT, PROJ, 'trunk', path])
        cmd = svncmd('export', url)
        if umask:
            oldmask = os.umask(umask)
        rc += lib.util.execute(cmd, check=True, cwd=cwd,
                               stdout=open(os.devnull, 'wb'), vl=vl)
        if umask:
            os.umask(oldmask)

    return 2 if rc else 0


def parse_deps(contents, expand=True):
    """Parse the contents of a DEPS file and return relevant results."""
    # Because 'vars' is (unfortunately) a reserved word we need to play games.
    xdata = contents.replace('vars = ', 'dvars = ', 1)
    if expand:
        varfunc = '''def Var(name): return "${%s}" % name\n'''
    else:
        varfunc = '''def Var(name): return 'Var("%s")+' % name\n'''

    dpdict = {}
    exec (varfunc + xdata) in globals(), dpdict
    for key in dpdict['deps']:
        dpdict['deps'][key] = dpdict['deps'][key].rstrip('+')

    return dpdict['dvars'], dpdict['deps'], contents


class RepoState(object):

    """
    Manage and track SCM states.

    This class represents a given moment in time, or state, of our
    source code repositories. The task is harder than it might seem
    since we may have multiple repositories, and even SCM tools such
    as svn and git, in use.

    Within a single svn repo the revision number is sufficient to
    identify a state; in a git repo the equivalent is a SHA-1 hash (but
    note that revisions have a natural ordering whereas git hashes do
    not). Both SCM tools, and in fact probably all, accept an ISO8601
    timestamp as an alternative way of selecting a given state. Thus
    timestamps are a "lingua franca" which can cross svn repos and
    between svn and git and probably any future SCM system. Timestamps
    also have a natural ordering. For these reasons we prefer to think
    in terms of timestamps for identifying a given code state.

    Timestamps do have a significant weakness in that they cannot easily
    be bisected. Individual commits may come minutes or even hours apart,
    whereas time is infinitely divisible in theory making binary searches
    difficult (in practice, svn tracks time only to microseconds).

    Because of this we need to keep track of "native" revisions and
    hashes for bisecting purposes. This class uses time as the primary
    identifier for a given state while associating that timestamp with
    the correlated key (revision or hash) in each repo. Two instances
    are considered equal if all their keys are equal even if their
    timestamps differ.

    Note that svn tracks timestamps in microseconds and its rules say
    that a timestamp selects any revision _before_ it.  The effect of
    this is that the time associated with a revision will not select
    that revision since it doesn't predate it. Therefore the timestamp
    we associate with a given svn revision is its recorded time plus
    one usec.

    """

    def __init__(self, spec, svnrepos=lib.times.SVNREPOS, gitrepos=()):
        self.posixtime = lib.times.iso2seconds(lib.times.timeref(spec))
        self.svnrepos = {}
        for svnrepo in svnrepos:
            self.svnrepos[svnrepo] = None, None
        self.gitrepos = {}
        for gitrepo in gitrepos:
            self.gitrepos[gitrepo] = None, None

    # The important point here is that two instances with
    # differing timestamps may still be identical if they
    # select the same versions.
    def __cmp__(self, other):
        if self.posixtime == other.posixtime:
            return 0
        for svnrepo in self.svnrepos:
            if self.key(svnrepo) != other.key(svnrepo):
                return 1 if self.posixtime > other.posixtime else -1
        for gitrepo in self.gitrepos:
            if self.key(gitrepo) != other.key(gitrepo):
                return 1 if self.posixtime > other.posixtime else -1
        return 0

    # We do not use the timestamp in the hash for the reason above.
    def __hash__(self):
        result = 0
        for svnrepo in sorted(self.svnrepos, reverse=True):
            result += hash(self.key(svnrepo))
        for gitrepo in sorted(self.gitrepos, reverse=True):
            result += hash(self.key(gitrepo))
        return result

    def __repr__(self):
        result = '%s (%f)' % (self.iso8601(), self.posixtime)
        for svnrepo in sorted(self.svnrepos, reverse=True):
            result += '\n%-7s %s (%s)' % (
                unicode(self.key(svnrepo)) + ':', svnrepo,
                lib.times.posix2timestamp(self.when(svnrepo)))
        for gitrepo in sorted(self.gitrepos, reverse=True):
            result += '\n%-7s %s' % (self.key(gitrepo) + ':', gitrepo)
        return result + '\n'

    def __str__(self):
        return self.iso8601()

    def iso8601(self):
        """Return a stringified version of the timestamp."""
        return lib.times.posix2timestamp(self.posixtime)

    def wlansvn(self):
        """Return a stringified summary with wlansvn rev and timestamp."""
        return '%s {%s}' % (self.key(), self.iso8601())

    def lookup(self, repo=None):
        """Return what we know about this instance wrt the given repo."""
        if repo in self.svnrepos:
            if self.svnrepos[repo][0] is None:
                self.svnrepos[repo] = self.querysvn(repo)
                assert self.svnrepos[repo][0] > 0
            return self.svnrepos[repo]
        else:
            if self.gitrepos[repo] is None:
                self.gitrepos[repo] = self.querygit(repo)
            return self.gitrepos[repo]

    def key(self, repo=lib.times.SVNREPOS[0]):
        """Return the associated key (revision or sha1) in given repo."""
        return self.lookup(repo)[0]

    def when(self, repo=lib.times.SVNREPOS[0]):
        """Return the time of commit in given repo."""
        return self.lookup(repo)[1]

    def latest_repo(self):
        """Return the latest committed repo in this instance."""
        ordered = sorted(self.svnrepos, key=lambda k: k[1])
        return ordered[0]

    def degrees(self, other):
        """Return the number of commits between self and other."""
        gap = 0
        for svnrepo in sorted(self.svnrepos, reverse=True):
            gap += abs(self.key(svnrepo) - other.key(svnrepo))
        return gap

    def bisect(self, other):
        """Return an instance midway between self and other."""
        if self == other or self.degrees(other) < 2:
            return None
        midtime = (self.posixtime + other.posixtime) / 2
        while True:
            midpoint = RepoState('{@%f}' % midtime,
                                 svnrepos=sorted(self.svnrepos.keys()),
                                 gitrepos=sorted(self.gitrepos.keys()))
            if midpoint == self:
                if midpoint == other:
                    return None
                midtime = (midtime + other.posixtime) / 2
            elif midpoint == other:
                midtime = (midtime + self.posixtime) / 2
            else:
                break
        return midpoint

    def querysvn(self, svnrepo):
        """Find the associated key (revision) in specified svn repo."""
        stats = svn_stats('{%s}' % self.iso8601(), svnrepo)
        # If the actual date was inaccessible for permissions reasons,
        # fake it with the reftime which should be fairly close.
        # An error message will have been printed.
        if stats[1] == -1:
            return stats[0], self.posixtime
        else:
            return stats

    def querygit(self, gitrepo):
        """Find the associated key (hash) in specified git repo."""
        assert 'TBD'
        return self.iso8601() + gitrepo

# vim: ts=8:sw=4:tw=80:et:
