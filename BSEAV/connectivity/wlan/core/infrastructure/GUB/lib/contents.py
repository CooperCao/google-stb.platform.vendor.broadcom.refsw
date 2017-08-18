"""
Track the contents of a particular buildable over its branch history.

BACKGROUND:

A complicating factor is that svn commits aren't restricted to a given
branch as in many other SCM tools; a commit can modify any set of files
across any set of branches. Some change sets may be to files all on
one branch, others may affect the same logical file across multiple
branches. From the POV of branch X, commits fall into these categories:

    1. Changes were only to branch X. Only branch X cares.
    2. Changes were to branch X and others. X cares, as do the others.
    3. Changes were only to an ancestor of branch X prior to the branch point.
       Branch X cares, as do any other descendants.
    4. Changes were to an ancestor of branch X and others, prior to the
       branch point. Branch X cares, as do other descendants of the
       ancestor, as do the unrelated branches involved in the commit.
    5. Changes were outside branch X's line of history. X doesn't care.

ALGORITHM:

The derive() function in this module needs a tag/branch, a starting
time, an ending time, the paths of a JSON database to write to and a
JSON database to read from (could be the same), and an optional checkout
tree appropriate to the buildable item (i.e. a router tree for a router
buildable). It also needs the name of the buildable and the number of
the current build (e.g. yyyy.m.d.n).

If an input database path is given with no output database path, the
assumption is that no new data is being derived and the task is only to
process the input data, e.g. to do blames or to generate HTML or similar.
If an output path is given with no input, the assumption is that new
data must be derived from scratch. If both are given, the task is to
extend the input DB and write the result to the output DB (these may be
and typically are the same).

Both start and end times have defaults. The default end time is the
time of the checkout and the default start time is a fixed date in
the past known as the CONTENTS_EPOCH. However, it will typically work
incrementally as described above; when provided an input contents
database the default start time becomes the end time of the previous
update. In other words it will pick up where the previous update
left off.

The buildable name, build number, and tag are required.

It will need to know the full set of branching events leading from
trunk to the current branch and the datestamp of each, and it needs
to know this for the current branch of each component involved in the
build. Since deriving this data can be slow and it never changes, we
cache branch ancestry data within the JSON. After it's derived once,
new branches should only have to track back to their immediate ancestor
and subsequent builds on an existing branch should be all set.

This ancestry data is used to associate a given commit with the branch
it was originally made to, which may be an ancestor of the current
branch/tag. In other words each commit is assigned a "currently active
branch" by date.  For instance, if branch A came off trunk on date X
and branch B came off A on later date Y, any commits made between X and
Y must have been on branch A. Commits earlier than X were on trunk, etc.

Next it queries the metadata to find the set of individual svn checkouts
involved. It then runs an svn log command on each checkout to get
the set of commits between start and end times. These could overlap,
meaning the same commit might show up in more than one checkout.

For each checkout, after running svn log it enters a state machine
which parses the textual log data into a Commit object. These Commit
objects are classified as relevant or irrelevant depending on whether
they affected files visible to the build. This relevance test is only
needed because we have some sparse svn trees remaining; once those are
gone all commits found by svn log should be relevant. Note that it's
impossible to determine which files will *actually* be used during the
build, so we assume that any file present in the checkout tree *may*
be involved. Therefore, any commit which affects a file found in the
checkout is considered relevant. Also, commits which delete files
are *always* considered relevant. When in doubt the philosophy is to
assume relevance.

The relevance test is why an actual checkout tree is used. However,
the checkout tree is not required. If not present, all commits to a
relevant (ancestor) branch are considered relevant.

Relevant Commit objects are added to the History object representing
the history of the current buildable item. Commits are kept in a set()
data type, ensuring that each commit is shown only once even if it was
found in multiple checkouts or multiple invocations of this module.

STORAGE:

The standard storage format is JSON. We could have used YAML instead but
(a) PyYaml is non-core, JSON is core, (b) PyYaml is much slower than the
json module and (c) PyYaml dumps OrderedDicts in a less readable format.

The top-level JSON "schema" is really simple: ({}, []). This is a tuple
containing two items of which the first is a dict and the second a
list. The dict holds metadata while the list holds Commit objects.

To ensure delta stability (as the JSON file is stored in SCM) the
data is stored in ordered form even though it's used in unordered form
internally.  In other words any dictionaries in the History object are
converted to sorted OrderedDicts before being written out to JSON using
json.dump(). JSON will still represent them as dicts but the text will
retain the canonical order. When json.load() reads them back in they'll
revert to traditional unordered dicts, which is fine.  Similarly, Commit
objects are kept in a set() internally but are stored as an ordered list.

To better preserve history, and also to save processing time, the
generated JSON can be stored to SCM. Each build then checks out the
previous version, adds all new work since the previous build, and commits
it back. This means that the svn log commands usually cover just a day
or so incrementally.

The data may also be written out as HTML for human consumption but
that happens after it's already been generated as JSON. The core of
this module reads and writes JSON; writing the data out as HTML is a
later step.

TRACKING:

The whole purpose of this feature is to track history across branching
and tagging events. Therefore history must be inherited from a parent
branch into a child branch, twig, or tag. The first build on any new
branch or tag will not find stored JSON database so it needs to search
backward through the branch hierarchy to find the nearest predecessor.
Usually history will be found on the immediate parent and is then copied
into the new branch and used from there.

The only data here which cannot be derived from SCM is the history of
which build a given commit was first present in. This is why the JSON
is kept in svn and modified incrementally, though the speedup of using
cached data helps too.

"""

from __future__ import print_function
from __future__ import unicode_literals

from collections import OrderedDict

import cgi
import json
import os
import re
import string
import sys
import textwrap
import time

import lib.consts
import lib.item
import lib.pop
import lib.scm
import lib.times
import lib.util

ARROW = ' => '

# Ignore data prior to this point in time.
CONTENTS_EPOCH = '{2014-09-01T00:00:00-0700}'

# Names of fields we save in the database.
AUTHOR = 'Author'
BLDTAG = 'BldTag'
BRANCH = 'Branch'
BUILD = 'Build'
DATE = 'Date'
FILES = 'Files'
JIRA = 'JIRA'
MESSAGE = 'Message'
RB = 'RB'
SCMID = 'SCMID'

# This is the canonical field order used in both JSON and HTML.
# Ordering is important for delta stability.
# Be cautious about changing this. The JavaScript may need adjusting
# if changes are made.
FIELDS = [
    BUILD,
    BLDTAG,
    BRANCH,
    SCMID,
    DATE,
    AUTHOR,
    JIRA,
    RB,
    MESSAGE,
    FILES,
]

ANCESTRIES = 'ANCESTRIES'
AS_OF_BUILD = 'AS_OF_BUILD'
BUILDABLE = 'BUILDABLE'
CHECKOUTS_SVN = 'CHECKOUTS_SVN'
CMDLINE = 'CMDLINE'
FAILED = 'FAILED'
MAIN_TAG = 'MAIN_TAG'
SUCCEEDED = 'SUCCEEDED'
STATUSES = 'STATUSES'
TIME_EARLIEST = 'TIME_EARLIEST'
TIME_LATEST = 'TIME_LATEST'

# Statuses recorded only from r616679 (2016-02-02).
STATUS_EPOCH = 1454426034.0

# Commits from before recorded build history are marked as this.
UNKNOWN = 'UNKNOWN'

# A regex to pull JIRA ids out of log messages.
JIRA_RE = r'(?i)\b((JIRA)[:\s]*\S*?([A-Z]+[\d-]+\d))'

# URL to prepend to make JIRA ids clickable.
JIRA_URL = 'http://jira.%s/browse/' % lib.consts.DOMAIN

# A regex to pull RB ids out of log messages.
RB_RE = r'(?i)\b((RB)[:\s]*[^\s<]*?(\d+))'

# URL to prepend to make RB ids clickable.
RB_URL = 'http://wlan-rb.sj.%s/r/' % lib.consts.DOMAIN

# URL to prepend to make svn revisions clickable.
VIEWVC = 'http://svn.sj.broadcom.com/viewvc/wlansvn?view=revision&revision='

# JavaScript magic to create sortable table columns using JQuery TableSorter.
JSRVR = 'http://wlan-dashboard.sj.' + lib.consts.DOMAIN
JBASE = JSRVR + '/javascript/BuildFeatures/jquery_tablesorter'
TABLESORTER = '''
<script type="text/javascript" src="$jbase/jquery-latest.js"></script>
<script type="text/javascript" src="$jbase/jquery.tablesorter.js"></script>
<link rel="stylesheet" href="$jbase/themes/blue/style.css"
    type="text/css" media="print, projection, screen" />
<script type="text/javascript" src="$jbase/jquery-latest.js"></script>
<script type="text/javascript" src="$jbase/jquery.tablesorter.min.js"></script>
    <script type="text/javascript">
        $$(function() {
            $$("#buildDetailsTable").tablesorter({sortList: [[0,0]], headers: {
                0: { sorter: false },   // "Build" column doesn't sort well
                2: { sorter: false },   // "SCMID" could be a rev or a SHA-1
                7: { sorter: false },   // "Message" makes no sense to sort on
                8: { sorter: false },   // "Files" makes no sense to sort on
            }});
        });
    </script>
'''

# Prefixes used by svn log for modified, added, and deleted files.
MOD = '   M '
ADD = '   A '
DEL = '   D '


def by_build_order(name):
    """Sort builds by YYYY.mm.dd.N name."""
    parts = [int(p) for p in name.split('.')]
    return int('%d%02d%02d%04d' % tuple(parts))


class Commit(object):

    """Represent a single historical commit to an SCM system."""

    def __init__(self, scmid, date, raw=None):
        if not raw:
            raw = {}

        if isinstance(date, basestring):
            date = lib.times.iso2seconds(date)

        # These fields are stored in the same order as the FIELDS list.
        # It's not required but makes things more predictable.
        self.fields = OrderedDict()
        self.build = self.fields[BUILD] = raw.get(BUILD, None)
        self.bldtag = self.fields[BLDTAG] = raw.get(BLDTAG, None)
        # The lookup of 'Tag' and fallback to trunk are to make up
        # for bugs in early content list iterations.
        self.branch = self.fields[BRANCH] = raw.get(BRANCH, raw.get('Tag'))
        if not self.branch:
            self.branch = self.fields[BRANCH] = 'trunk'
        self.scmid = self.fields[SCMID] = scmid
        self.date = self.fields[DATE] = date
        self.author = self.fields[AUTHOR] = raw.get(AUTHOR, None)
        self.jira = self.fields[JIRA] = raw.get(JIRA, [])
        self.rb = self.fields[RB] = raw.get(RB, [])
        self.message = self.fields[MESSAGE] = raw.get(MESSAGE, '')
        self.files = self.fields[FILES] = raw.get(FILES, [])

    def __str__(self):
        # This format is relied upon by the GUBCI implementation.
        text = '%s | %s@%s | %s | %s:\n' % (self.scmid,
                                            self.author, lib.consts.DOMAIN,
                                            self.date, self.build)
        for line in self.relevant:
            vpath, fpath = line.split(ARROW, 1)
            vvc = vpath.replace('/svn/', '/viewvc/', 1)
            vvc += '?r1=%s' % self.scmid
            vvc += '&r2=%s' % (int(self.scmid) - 1)
            text += '    %s%s%s\n' % (vvc, ARROW, fpath)
        return text

    def __repr__(self):
        # This is helpful in debugging, not used in production.
        ret = ''
        for key in self.ordered:
            ret += '%-10s %s' % (key + ':', self.__dict__[key.lower()])
            if key == DATE:
                ret += ' (%s)' % time.ctime(self.__dict__[key.lower()])
            ret += '\n'
        return ret.rstrip()

    def __cmp__(self, other):
        # We avoid comparing IDs numerically in order to leave room for
        # later Git SHA-1 support, but since it's technically possible for
        # two commits (to different repos) to happen at the same time,
        # we use IDs to compare for identity while relying on the
        # datestamp for ordering.
        if self.scmid == other.scmid:
            return 0
        else:
            return cmp(self.date, other.date)

    def __hash__(self):
        return hash(self.scmid)

    def add_vpath(self, vpath, fpath):
        """Add a versioned-file path to the set."""
        # Canonicalize svn URLs to use the standard repo.
        if lib.consts.WL_REPO_ROOT != lib.consts.WL_REPO_ROOT_MASTER:
            vpath = vpath.replace(lib.consts.WL_REPO_ROOT,
                                  lib.consts.WL_REPO_ROOT_MASTER, 1)

        # Relevant files get a mapping to the local file path.
        if fpath:
            self.files.append(''.join([vpath, ARROW, fpath]))
        else:
            self.files.append(vpath)

    def add_message_line(self, text):
        """Add a line of text to the recorded message."""
        if self.message:
            self.message += '\n'
        self.message += text.lstrip(' ,:;.-').rstrip()

    def add_jira(self, jid):
        """Add a JIRA id to the recorded set."""
        # The same JIRA could be mentioned twice in the same message.
        if jid not in self.jira:
            self.jira.append(jid)
            self.jira.sort()

    def add_rb(self, rid):
        """Add a ReviewBoard id to the recorded set."""
        # The same RB could be mentioned twice in the same message.
        if rid not in self.rb:
            self.rb.append(rid)
            self.rb.sort()

    @property
    def relevant(self):
        """Return the list of relevant files in this commit."""
        relevants = [ln for ln in self.files if ARROW in ln]
        return relevants

    @property
    def ordered(self):
        """Return this object's attributes as an ordered dict."""
        for field in self.fields:
            self.fields[field] = self.__dict__[field.lower()]

        return self.fields

    def byname(self, name, default=None):
        """Look up and return a commit attribute by name."""
        result = self.ordered.get(name, default)
        if result is None:
            result = default
        return result


class History(object):

    """
    Represent the entire content-list history of a buildable.

    This is a singleton object which is essentially serialized into the
    JSON file at finish time and deserialized when working incrementally.
    The complication is that internally it prefers to use unordered
    data structures such as sets and dicts but it serializes in the form
    of ordered data by converting dicts to OrderedDicts and sets to
    sorted lists.

    An instance of this class represents the entire content-list history
    of a given buildable on a given branch/tag, showing changes per-build
    along with the pass/fail status of each build if known.
    """

    def __init__(self,
                 buildable=None,
                 build=None,
                 maintag=None,
                 path=None,
                 reftime='HEAD',
                 time_earliest=None,
                 time_latest=None,
                 ):
        self.buildable = buildable
        self.build = self.as_of_build = build
        self.maintag = maintag
        self.path = path
        self.reftime = reftime
        self.time_earliest = time_earliest
        self.time_latest = time_latest

        self.ancestries = {'trunk': {'trunk': 0}}
        self.commits = set()
        self.bldmap = {}
        self.checkouts = {}
        self.database = {}, []
        self.newcl = self.load_db()

        # Fill in defaults from previous database if not supplied.
        ometa = self.database[0]
        if not self.buildable:
            self.buildable = ometa.get(BUILDABLE)
        if not self.build:
            self.build = self.as_of_build = ometa.get(AS_OF_BUILD)
            self.cmdline = ometa.get(CMDLINE)
        else:
            self.cmdline = lib.util.cmdline(sys.argv)
        if not self.maintag:
            self.maintag = ometa.get(MAIN_TAG)
        if not self.time_latest:
            self.time_latest = ometa.get(TIME_LATEST)

    def load_db(self):
        """Load the history database from a URL or file."""
        newcl = False
        if not self.path or not lib.util.is_url(self.path):
            if self.path:
                try:
                    self.database = json.load(open(self.path, 'r'))
                except IOError:
                    newcl = True
            else:
                newcl = True
        else:
            url = self.path
            uparts = lib.scm.svn_tag_split(url)
            tpath, rev = self.maintag, None
            while True:
                if lib.scm.urlexists(url):
                    cmd = lib.scm.svncmd('cat', '@'.join([url, self.reftime]))
                    proc = lib.util.execute(cmd, stdout=lib.util.PIPE, vl=2)
                    jsonstr = proc.stdout.read()
                    if proc.wait() == 0:
                        self.database = json.loads(jsonstr.decode('utf-8'))
                    else:
                        newcl = True
                    break
                newcl = True
                if not tpath or os.path.basename(tpath) == 'trunk':
                    break
                ppath, rev, _, _, _ = lib.scm.svn_tag_parent(tpath, rev)
                if not ppath:
                    break
                tpath = ppath
                tparts = lib.scm.svn_tag_split(ppath)
                url = '/'.join([uparts[0], tparts[1], uparts[2]])

        # Transitional hack adding a build-status entry to the DB.
        if STATUSES not in self.database[0]:
            self.database[0][STATUSES] = {}

        # The ancestries structure is a dict of dicts in which the keys
        # are tags and the values are the time of the branch event.
        ancs = self.database[0].get(ANCESTRIES, {})
        for t1 in ancs:
            self.ancestries[t1] = {}
            for t2 in ancs[t1]:
                self.ancestries[t1][t2] = ancs[t1][t2]

        self.checkouts.update(self.database[0].get(CHECKOUTS_SVN, {}))

        if newcl:
            if not self.time_earliest:
                self.time_earliest = CONTENTS_EPOCH

            # When beginning a new branch the only thing we really
            # need to inherit is the mapping of which commits
            # belong to which builds. For everything else it's better
            # to derive historical data from scratch for the first
            # build on a new branch and go incrementally after that.
            for raw in self.database[1]:
                self.bldmap[raw[SCMID]] = raw[BUILD]
            return True
        else:
            # Unless a specific start time was requested, pick up where
            # previous history left off.
            if not self.time_earliest:
                self.time_earliest = self.database[0].get(TIME_EARLIEST,
                                                          CONTENTS_EPOCH)

            # Commit history is stored as a list but manipulated as a
            # set to ensure uniqueness. Also, each commit is stored as
            # a raw dict which must be turned back into a Commit object.
            for raw in self.database[1]:
                cmt = Commit(raw[SCMID], raw[DATE], raw=raw)
                self.commits.add(cmt)
            return False

    def dump_db(self, f):
        """ Write self out to the specified file object.

        Store all data in canonical order to ensure delta stability.
        """

        nmeta = OrderedDict()
        nmeta[ANCESTRIES] = OrderedDict()
        nmeta[AS_OF_BUILD] = self.as_of_build
        nmeta[BUILDABLE] = self.buildable
        nmeta[CHECKOUTS_SVN] = OrderedDict()
        nmeta[CMDLINE] = self.cmdline
        nmeta[MAIN_TAG] = self.maintag
        nmeta[STATUSES] = OrderedDict()
        nmeta[TIME_EARLIEST] = self.time_earliest
        nmeta[TIME_LATEST] = self.time_latest
        nmeta[lib.consts.WL_REPO_ROOT_EV] = lib.consts.WL_REPO_ROOT

        # Branch names are sorted alphabetically and each
        # one's history is sorted by date.
        ancs = nmeta[ANCESTRIES]
        for t1 in sorted(self.ancestries):
            ancs[t1] = OrderedDict()
            for t2 in reversed(sorted(self.ancestries[t1],
                                      key=lambda x: self.ancestries[t1][x])):
                ancs[t1][t2] = self.ancestries[t1][t2]

        # Record the checkout map derived from deps file.
        for key in sorted(self.checkouts, reverse=True):
            nmeta[CHECKOUTS_SVN][key] = self.checkouts[key]

        # Record the pass/fail status of each build in the STATUSES dict.
        for name in sorted(self.database[0][STATUSES], key=by_build_order):
            nmeta[STATUSES][name] = self.database[0][STATUSES][name]

        # Copy the commit history into an ordered list.
        nhist = []
        for cmt in sorted(self.commits, reverse=True):
            nhist.append(cmt.ordered)

        # Take the ordered temp copy and write it out.
        # JSON accepts, but does not write, a trailing newline.
        # Humans and editors prefer to have it.
        ndb = nmeta, nhist
        json.dump(ndb, f, indent=2)
        f.write('\n')

    def store_db(self, db_to, message=None):
        """Write this History object out to a storage medium."""
        if not db_to or db_to == '-':
            self.dump_db(sys.stdout)
        elif not lib.util.is_url(db_to):
            self.dump_db(open(db_to, 'w'))
        else:
            if message:
                msg = message
            else:
                msg = 'automated update by ' + self.as_of_build
            dtemp = lib.util.tmpdir(prefix='contents.')
            jsonfile = os.path.join(dtemp, os.path.basename(db_to))
            cocmd = lib.scm.svncmd('co', '-q', os.path.dirname(db_to), dtemp)
            if lib.util.execute(cocmd, stderr=open(os.devnull, 'w'), vl=2):
                mkcmd = lib.scm.svncmd('mkdir', '-q', '--parents', '-m', msg)
                mkcmd.append(os.path.dirname(db_to))
                # There's always a race in mkdir -p cmds wrt parent dirs.
                if lib.util.execute(mkcmd, vl=2) != 0:
                    lib.util.execute(mkcmd, vl=2)
                lib.util.execute(cocmd, vl=2)
            if not os.path.exists(jsonfile):
                open(jsonfile, 'w').close()
                addcmd = lib.scm.svncmd('add', '-q', jsonfile)
                lib.util.execute(addcmd, vl=2)
            self.dump_db(open(jsonfile, 'w'))
            cicmd = lib.scm.svncmd('ci', '-q', '-m', msg, dtemp)
            lib.util.execute(cicmd, vl=2)

    def dump_html(self, html_to, refdb, tree):
        """Dump results in HTML format."""
        def anchor(desc, href=None, name=None, tab=True):
            """Return an HTML anchor tag."""
            href = (' href="%s"' % href if href else '')
            name = (' id="%s"' % name if name else '')
            tgt = (' target="_blank"' if tab else '')
            return '<a%s%s%s>%s</a>' % (href, name, tgt, desc)

        title = 'Contents Report: Build %s of %s on %s' % (
            self.as_of_build,
            self.buildable,
            self.maintag,
        )

        if html_to == '-':
            f = sys.stdout
        else:
            f = open(html_to, 'w')

        f.write('<!DOCTYPE html>\n<html>\n<head>\n')
        f.write('<title>%s</title>\n' % title)
        f.write('<meta http-equiv="content-type"')
        f.write(' content="text/html; charset=UTF-8">\n')
        f.write('<style>\n')
        f.write('  table, th, td { border: 1px solid black;}\n')
        f.write('  th {text-align: left;}\n')
        f.write('  h1 {text-align: center;}\n')
        f.write('  table#comp { width: 100%; background-color: #f1f1c1;}\n')
        f.write('</style>\n')
        f.write(string.Template(TABLESORTER).substitute(jbase=JBASE).lstrip())
        f.write('</head>\n')
        f.write('\n')

        f.write('<body>\n')
        f.write('<h1>%s</h1>\n' % title)
        if refdb:
            f.write('<div align="right">Generated from %s</div>\n' %
                    anchor('JSON', href=refdb))
        f.write('\n')

        f.write('<br>\n')
        f.write('<h1>Components Used</h1>\n')
        f.write('<p></p>\n')
        f.write('<table id="comp">\n')
        f.write('<thead>\n  <tr>\n')
        f.write('    <th>Checkout</th>\n')
        f.write('    <th>URL</th>\n')
        f.write('</tr>\n  </thead>\n')
        for subdir in sorted(self.checkouts, reverse=True):
            f.write('  <tr>\n')
            f.write('    <td>%s</td>\n' % subdir)
            f.write('    <td>%s</td>\n' % self.checkouts[subdir])
            f.write('  </tr>\n')
        f.write('</table>\n')
        f.write('<p></p>\n')
        f.write('<br>\n')
        f.write('\n')

        f.write('<h1>History of Contributing Commits</h1>\n')
        f.write('<table id="buildDetailsTable" class="tablesorter">\n')
        f.write('<thead>\n  <tr>\n')
        for key in FIELDS:
            if key == BLDTAG:
                continue
            if key == BRANCH:
                key = 'Commit Branch'
            f.write('    <th>%s</th>\n' % key)
        f.write('</tr>\n  </thead>\n')

        prevbld = None
        for cmt in sorted(self.commits, reverse=True):
            curbld = cmt.build
            stanza = '  <tr>\n'

            if curbld == prevbld:
                pre = '    <td><pre>'
                post = '    </pre></td>\n'
            else:
                pre = '    <td><pre><b>'
                post = '    </b></pre></td>\n'

            for key in FIELDS:
                if key == BLDTAG:
                    # BLDTAG is handled specially for HTML purposes.
                    continue

                stanza += pre

                val = cmt.byname(key, 'None')
                if key == BUILD:
                    if curbld != prevbld:
                        bldtag = cmt.byname(BLDTAG, cmt.branch)
                        txt = '%s on\n%s' % (val, bldtag)
                        if lib.scm.is_static(bldtag) and tree and \
                           tree.startswith('/projects'):
                            url = lib.consts.HTTP_BASE + tree
                            cell = anchor(txt, href=url, name='BUILD:' + val)
                        else:
                            cell = anchor(txt, name='BUILD:' + val, tab=False)
                        status = self.get_status(val)
                        if status is not None:
                            color = 'green' if status else 'red'
                            cell = '<font color="%s">%s</font>' % (color, cell)
                        stanza += cell
                elif key == AUTHOR:
                    mto = val + '?Subject=Revision%20' + cmt.scmid
                    stanza += '<a href="mailto:%s">%s</a>' % (mto, val)
                elif key == SCMID:
                    stanza += anchor(val, href=VIEWVC + val,
                                     name='SCMID:' + val)
                elif key == DATE:
                    # Add a newline to keep this column narrow.
                    stanza += time.strftime('%Y-%m-%d\n%H:%M:%S %Z',
                                            time.localtime(val))
                elif key == JIRA:
                    if val:
                        for jira in val:
                            stanza += anchor(jira, href=JIRA_URL + jira,
                                             name='JIRA:' + jira)
                            stanza += '\n'
                    else:
                        stanza += 'None'
                elif key == RB:
                    if val:
                        for rb in val:
                            stanza += anchor(rb, href=RB_URL + rb,
                                             name='RB:' + rb)
                            stanza += '\n'
                    else:
                        stanza += 'None'
                elif key == MESSAGE:
                    msg = val.replace('\t', ' ')
                    msg = textwrap.fill(msg, replace_whitespace=False)
                    msg = cgi.escape(msg)
                    if cmt.jira and cmt.rb:
                        msg = '<font color="green">' + msg + '</font>'
                    elif re.search(r'\bNO_[A-Z]+_CHECK\b', msg):
                        msg = '<font color="red">' + msg + '</font>'
                    stanza += msg
                elif key == FILES:
                    # When showing paths in HTML we provide ViewVC anchors.
                    dpaths = {}
                    for line in val:
                        parts = line.split(ARROW, 1)
                        if len(parts) < 2:
                            continue
                        vpath, dpath = parts[0], parts[1]
                        vvc = vpath.replace('/svn/', '/viewvc/', 1)
                        dpaths[dpath] = anchor(dpath, href=vvc)
                    paths = [dpaths[dp] for dp in sorted(dpaths)]
                    stanza += '\n'.join(paths)
                else:
                    stanza += val

                stanza += post
            stanza += '  </tr>\n'
            f.write(stanza.encode('utf-8', 'xmlcharrefreplace'))
            prevbld = curbld
        f.write('</table>\n')
        f.write('</body>\n</html>\n')

    def add_commit(self, cmt):
        """Add a commit to the recorded set."""
        # When history has been inherited from a parent branch we need
        # to borrow the parent's knowledge of which commits went into
        # which builds.
        if cmt.scmid in self.bldmap and self.bldmap[cmt.scmid]:
            # Inheriting the build number from prior history.
            cmt.build = self.bldmap[cmt.scmid]
        elif lib.scm.is_static(cmt.branch):
            # For REL tags, give unassigned commits to current build.
            cmt.build = self.build
        elif lib.consts.INVOKE_TIME - cmt.date < lib.consts.SECONDS_PER_DAY:
            # If the commit took place during the current day it belongs
            # to the current build.
            cmt.build = self.build
        else:
            # As a fallback we assume the commit belongs to the first (.0)
            # build of the day.
            nextday = time.localtime(cmt.date + lib.consts.SECONDS_PER_DAY)
            # On Unix the format string '%Y.%-m.%-d.0' strips leading zeroes
            # but for some reason on Windows, even Cygwin, it fails.
            with0 = time.strftime('%Y.%m.%d.0', nextday)
            cmt.build = re.sub(r'\.0+(\d)', r'.\1', with0)

        self.commits.add(cmt)

    def acquire_tag(self, tagline, url):
        """Add specified tag line to cached branching history."""
        if tagline not in self.ancestries:
            self.ancestries[tagline] = lib.scm.svn_tag_history(url)

    def assign_tag(self, cmt, tagline):
        """Give the commit to the correct ancestral tag (branch).

        A commit may matter to the current tag if it was made to an
        ancestor branch. We assign commits to branches by comparing the
        date they occurred with the branching history of the current
        checkout.
        """

        cmt.branch = 'trunk'
        if tagline:
            for tag in reversed(sorted(self.ancestries.get(tagline, {}),
                                       key=lambda x:
                                       self.ancestries[tagline][x])):
                if cmt.date >= self.ancestries[tagline][tag]:
                    cmt.branch = tag
                    break

        if not cmt.bldtag:
            cmt.bldtag = self.maintag

    def get_checkouts(self, cfgroot, opts):
        """Derive a dict mapping local dirs to URLs."""
        tree = lib.pop.Tree(basedir='.', question={})
        for todo in lib.item.items2todos(cfgroot, opts, self.maintag):
            url = '%s?to=%s' % (todo[1], todo[0])
            tree.add(url, reftime=self.time_latest, vl=0)
        tree.populate(jobs=1, reftime=self.time_latest)
        self.checkouts = tree.question

    def set_status(self, name, good):
        """Mark a given build as passed or failed."""
        self.database[0][STATUSES][name] = SUCCEEDED if good else FAILED

    def get_status(self, name):
        """Return True|False|None to indicate status of named build."""
        if name in self.database[0][STATUSES]:
            return self.database[0][STATUSES][name] == SUCCEEDED
        else:
            return None

    def blames_by_lgb(self):
        """Print blame data going back to the last good build."""
        cmtlist = sorted(self.commits, reverse=True)
        blamelist = []
        for cmt in cmtlist:
            if cmt.date <= STATUS_EPOCH:
                break
            elif self.get_status(cmt.build):
                break
            else:
                blamelist.append(cmt)
        return blamelist

    def blames_by_date(self, date_range):
        """Print a blame map for the specified date range."""
        cmtlist = sorted(self.commits, reverse=True)
        parts = date_range.split(',')
        first = lib.times.iso2seconds(lib.times.timeref(parts[0]))
        if len(parts) == 1:
            last = cmtlist[0].date
        else:
            last = lib.times.iso2seconds(lib.times.timeref(parts[1]))
        blamelist = [c for c in cmtlist if c.date >= first and c.date <= last]
        return blamelist

    def lgb(self):
        """Print data about the last good build."""
        for name in sorted(self.database[0][STATUSES],
                           key=by_build_order, reverse=True):
            if self.database[0][STATUSES][name] == SUCCEEDED:
                return name


def derive(buildable,
           build,
           maintag,
           tree,
           cfgroot=None,
           db_from=None,
           db_to=None,
           html_to=None,
           message=None,
           opts=None,
           time_earliest=None,
           time_latest=None):
    """Generate or continue the history of a branch/twig/tag."""
    if tree:
        lib.util.chdir(tree, vl=2)

    def involved(path):
        """Return True if this path seems involved in the build."""
        return os.path.isfile(path) if tree else True

    # Start with a single object representing build history.
    hist = History(
        buildable=buildable,
        build=build,
        maintag=maintag,
        path=db_from,
        reftime='HEAD',
        time_earliest=time_earliest,
        time_latest=time_latest,
    )

    # This mode is used to generate HTML from an existing JSON DB.
    if html_to and not db_to:
        hist.dump_html(html_to, db_from, tree)
        return

    lib.util.assert_(maintag, 'no main tag specified')

    hist.get_checkouts(cfgroot, opts)

    # Now we enter a state machine for parsing svn log entries like the
    # example below. It may have been better to use --xml but this works.

    ##########################################################################
    # ------------------------------------------------------------------------
    # r505946 | nehru | 2014-10-01 16:48:24 -0700 (Wed, 01 Oct 2014) | 1 line
    # Changed paths:
    #    M /proj/branches/BISON_BRANCH_7_10/src/common/include/proto/802.11.h
    #    M /proj/branches/BISON_BRANCH_7_10/src/wl/proxd/src/pdftmproto.c
    #    M /proj/branches/BISON_BRANCH_7_10/src/wl/proxd/src/pdftmpvt.h
    #
    # JIRA:SWWLAN-59890 RB:38194 - revise override support based on discussion
    ##########################################################################

    # We prefer the read-only wlansvn mirror for log operations because
    # it's tuned for speed. Make sure it's up to date before continuing.
    if lib.consts.WL_REPO_ROOT_EV not in os.environ:
        lib.scm.svn_wait(master=lib.consts.WL_REPO_ROOT,
                         mirror=lib.consts.WL_REPO_ROOT_RO)

    for checkout in sorted(hist.checkouts, reverse=True):
        cmd = lib.scm.svncmd('log', '-v')
        svnurl = hist.checkouts[checkout]
        if '@' in svnurl:
            url_latest = svnurl.split('@')[-1]
            cmd.append('-r%s:%s' % (url_latest, hist.time_earliest))
        else:
            cmd.append('-r%s:%s' % (hist.time_latest, hist.time_earliest))
            svnurl = '@'.join([svnurl, hist.time_latest])

        tagline = lib.scm.svn_tag_name(svnurl)
        if tagline and tagline != 'trunk':
            hist.acquire_tag(tagline, svnurl)

        # Get log data from the RO wlansvn mirror if applicable.
        cmd.append(svnurl.replace(lib.consts.WL_REPO_ROOT,
                                  lib.consts.WL_REPO_ROOT_RO))

        start_of_commit = False
        cmt = None
        proc = lib.util.execute(cmd, stdout=lib.util.PIPE, vl=2)
        for ln in proc.stdout:
            # Treat all svn data as UTF-8 because some is not ASCII.
            ln = ln.decode('utf-8', 'xmlcharrefreplace')
            if not ln.strip():
                continue

            if re.match(r'^-+$', ln):
                if cmt and cmt.relevant:
                    # Record the previous commit if considered relevant.
                    hist.add_commit(cmt)

                # Start a new commit.
                start_of_commit = True
                continue

            if start_of_commit and ln[0] == 'r' and ln.count(' | ') == 3:
                # Start parsing the next commit into a Commit object.
                detail = ln.split(' | ')
                rev = detail[0][1:]
                date = lib.times.iso2seconds(' '.join(detail[2].split()[0:3]))
                cmt = Commit(rev, date)
                cmt.author = detail[1]
                hist.assign_tag(cmt, tagline)
            elif ln.startswith('Changed paths:'):
                pass
            elif ln.startswith(MOD) or \
                    ln.startswith(ADD) or \
                    ln.startswith(DEL):
                # See if we can get a useful path out of the log data.
                # Determine both URL and local paths to this file.
                try:
                    buf = ln[5:-1].strip()
                    buf = buf.split(' (from ', 1)[0]  # matching the )
                    vpath = lib.consts.WL_REPO_ROOT + buf
                    _, tagpath, tailpath = lib.scm.svn_tag_split(buf)
                    tagnm = (os.path.basename(tagpath) if tagpath else None)
                except (IndexError, ValueError):
                    continue

                # A *commit* is considered relevant if it contains any relevant
                # files. A *file* is considered relevant if it was modified
                # on the "currently active branch" AND it is either present
                # in the checkout or the action was a deletion (since in that
                # case it's hard to determine relevance). The "currently active
                # branch" is determined by date; for instance, if branch A came
                # off trunk on date X and branch B came off A on date Y, any
                # commits made between X and Y must be on branch A.
                # NOTE: going forward all trees may be expected to be
                # "bushy" (non-sparse). Only the ./src trees on EAGLE and
                # older are sparse, so it may be possible to simplify
                # the test for 'presence' someday.

                if tailpath:
                    fpath = os.path.join(os.path.dirname(checkout), tailpath)
                    if os.path.isdir(fpath):
                        pass
                    elif tagnm and tagnm == cmt.branch and involved(fpath):
                        cmt.add_vpath(vpath, fpath)
                    else:
                        # We seem to have some non-standard branch patterns
                        # so try another place.
                        # TODO if we can check files out to predictable paths
                        # we should be able to predict where to find them.
                        # Is something wrong in this logic?
                        fpath = os.path.join(checkout, tailpath)
                        if os.path.isdir(fpath):
                            pass
                        elif tagnm and tagnm == cmt.branch and involved(fpath):
                            cmt.add_vpath(vpath, fpath)
                        else:
                            cmt.add_vpath(vpath, None)
            else:
                def extract_id(match):
                    """Extract JIRA or RB ids from commit message."""
                    key = match.group(2).upper()
                    if key == JIRA:
                        cmt.add_jira(match.group(3))
                    elif key == RB:
                        cmt.add_rb(match.group(3))
                    return match.group(1)

                # Extract JIRA:xxxxxx and RB:yyyyyy references iteratively
                # until there are no more in the text.
                while True:
                    subbed = re.sub(JIRA_RE, extract_id, ln)
                    subbed = re.sub(RB_RE, extract_id, ln)
                    if subbed == ln:
                        break
                    ln = subbed

                cmt.add_message_line(ln)

            start_of_commit = False

        if proc.wait():
            sys.exit(2)

    if html_to:
        hist.dump_html(html_to, db_to, tree)

    if db_to:
        hist.store_db(db_to, message=message)

    return hist

# vim: ts=8:sw=4:tw=80:et:
