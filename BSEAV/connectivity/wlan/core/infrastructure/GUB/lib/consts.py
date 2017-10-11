"""A place to hold global constants."""

# The reason this exists is to avoid "import loops" where
# A imports B and B imports A just to get access to each
# others' constants. Since this has no semantics of its own,
# other modules can import it with minimal pollution.
# Therefore, this should import no other non-builtin modules
# (besides lib.state which is handled ok).

from __future__ import print_function
from __future__ import unicode_literals

import getpass
import multiprocessing
import os
import socket
import sys
import time

# Allowed by special case. No other non-core modules should be imported.
import lib.state

# All enabled (aka active) items of this type.
ALL_ENABLED = 'ALL'

# All known (aka defined) items of this type.
ALL_KNOWN = 'KNOWN'

# A place to collect all build summary data (if present).
BUILD_SUMMARIES = os.path.join(os.sep, 'projects', 'hnd_sw_buildteam_ext',
                               'hwnbuild-storage', 'build-summaries')

# I wanted to derive this dynamically but the complexity cost just turned
# out to be too high.
BUILDER = 'hwnbuild'

# Well-known filename referred to in multiple places.
CONTENTS_HTML = 'Contents.html'

# Aid in determining a good default amount of parallelism.
CPUS = multiprocessing.cpu_count()

# Content list locations.
CLREPO = 'http://svn.sj.broadcom.com/svn/wlansw'
CLBASE = CLREPO + '/groups/software/build/contents'
CLNAME = 'Contents.json'

# Duh.
DOMAIN = 'broadcom.com'

# The git repo for Olympic source delivery
GIT_REPO_PATH = '/projects/hnd_swbuild_git/olympic_git/public_repos'

# This git workspace contains symlinks back to the repo path,
# mapping a git repo name to its branch
GIT_WKSPC_PATH = '/projects/hnd_swbuild_git/olympic_git/public_workspaces'

# Unfortunately, this buildable must often be handled as a special case.
HDW = 'hndrte-dongle-wl'

# List of supported build host platforms.
HOST_PLATFORMS = ['linux', 'macos', 'windows']

# The current host (potentially a FQDN).
HOSTNAME = socket.gethostname()

# The URL root for HTTP browsing of NFS directories.
HTTP_BASE = 'http://home.sj.' + DOMAIN

# We treat this as the instant at which the script started.
INVOKE_TIME = time.time()

# Used to pass around a derived max load suggestion.
LOAD_LIMIT_EV = 'GUB_LOAD_LIMIT'

# A place for temp files which can be reached across LSF, ssh, etc.
NFS_TMPDIR = '/projects/hnd_swbuild_ext9_scratch/NFSTMP'

# Key to control nightly build schedules per tag and/or buildable.
NIGHTLY_DAYS = 'nightly_days'

# Special case - these should be modified by bin/preface.py on entry.
ARGV = '<*UNSET*>'
PROG = '<*UNSET*>'
TOOLDIR = '<*UNSET*>'

# The email list primarily responsible for builds et al.
SCM_LIST = '@'.join(['hnd-software-scm-list.pdl', DOMAIN])

# The number of seconds in a day.
SECONDS_PER_DAY = 60 * 60 * 24

# The number of seconds in an hour.
SECONDS_PER_HOUR = 60 * 60

# Someday we may find a more reliable way of determining the current site.
# From tima: "mount | sed -n 's/^fs-\([^-]*\)-[0-9]*:\/vol.*/\1/p;tq;b;:q q'"
try:
    SITE = os.environ.get('SITE',
                          open('/tools/lsf/lsfsite').read().rstrip())
except Exception:
    SITE = None

# An ssh host guaranteed to be at the 'main' (San Jose) facility.
SJ_SSH_HOST = 'xlinux.sj.' + DOMAIN

# Standard options for running ssh:
# BatchMode means "fail if passwordless login unavailable"
# ConnectTimeout and ConnectionAttempts combine for retry logic.
# LogLevel setting suppresses /etc/motd printing.
# StrictHostKeyChecking setting relieves stress on DNS.
SSH_CMD = [
    'ssh',
    '-o', 'BatchMode=yes',
    '-o', 'ConnectTimeout=60',
    '-o', 'ConnectionAttempts=10',
    '-o', 'LogLevel=error',
    '-o', 'StrictHostKeyChecking=no',
]

# Internal state tracked via environment variables through LSF and SSH.
STATE = lib.state.State()

# Name of invoking user.
USER = getpass.getuser()

# Our sites are inconsistent. Some have their own svn mirrors,
# some don't, and some borrow from nearby sites.
_SVN_SITE = SITE if SITE else 'sj'
_SITEMAP = {
    'REF': 'http://svn.sj.%s/svn/%%ssvn' % DOMAIN,
    'blr': 'http://svn.blr.%s/svn-sj1/%%ssvn' % DOMAIN,
    'cam': 'http://svn.sj.%s/svn/%%ssvn' % DOMAIN,
    'hc': 'http://svn.hc.%s/svn-sj1/%%ssvn' % DOMAIN,
    'irv': 'http://svn.sj.%s/svn/%%ssvn' % DOMAIN,
    'il': 'http://svn.il.%s/svn-sj1/%%ssvn' % DOMAIN,
    'neth': 'http://svn.sj.%s/svn/%%ssvn' % DOMAIN,
    'rdu': 'http://svn.sj.%s/svn/%%ssvn' % DOMAIN,
    'seo': 'http://svn.seo.%s/svn-sj1/%%ssvn' % DOMAIN,
    'sj': 'http://svn.sj.%s/svn/%%ssvn' % DOMAIN,
    'syd': 'http://svn.syd.%s/svn-sj1/%%ssvn' % DOMAIN,
    'tai': 'http://svn.hc.%s/svn-sj1/%%ssvn' % DOMAIN,
    'tlv': 'http://svn.il.%s/svn-sj1/%%ssvn' % DOMAIN,
    'tw': 'http://svn.hc.%s/svn-sj1/%%ssvn' % DOMAIN,
}

# Preferred SVN base URLs may vary per site. Some sites do not need
# or merit a mirror of their own so they use SJ. For instance Irvine
# shares the SJ repo. Other sites have a mirror which does not map
# to the site name in a standard way so they need special cases.
# Allow this setting to be overridden in the environment.

UC_REPO_ROOT_EV = 'UCODE_REPO_ROOT'
UC_REPO_ROOT = os.environ.get(UC_REPO_ROOT_EV, _SITEMAP[_SVN_SITE] % 'ucode')
UC_REPO_ROOT_MASTER = _SITEMAP['REF'] % 'ucode'

WL_REPO_ROOT_EV = 'WL_REPO_ROOT'
WL_REPO_ROOT = os.environ.get(WL_REPO_ROOT_EV, _SITEMAP[_SVN_SITE] % 'wlan')
WL_REPO_ROOT_MASTER = _SITEMAP['REF'] % 'wlan'

# Read-only (build) checkouts from SJ use a dedicated mirror:
# "svn://wlan-sj1-svn01.sj.broadcom.com/svn/wlansvn".
_SJ_SVN_REPO_RO = 'svn://wlan-sj1-svn01.sj.broadcom.com/svn/%ssvn'
UC_REPO_ROOT_RO = _SJ_SVN_REPO_RO % 'ucode'
WL_REPO_ROOT_RO = _SJ_SVN_REPO_RO % 'wlan'

# The prefix for going through CIFS from Windows.
UNC_BASE = '//corp.ad.%s/sjca' % DOMAIN
if sys.platform.startswith('win') or sys.platform.startswith('cyg'):
    Z = UNC_BASE
else:
    Z = ''

# The root of our conventional tools install area.
TOOLSBIN = '/projects/hnd/tools'

# The place we typically install local tools for Linux.
LXBIN = TOOLSBIN + '/linux/bin'

# The place we typically install local tools for Windows.
WINBIN = UNC_BASE + TOOLSBIN + '/win/bin'

# The setuid executable which provides builder privileges.
# May be built only for Linux.
AS_BUILDER = 'as_' + BUILDER
if sys.platform.startswith('linux') and os.access(LXBIN, os.X_OK):
    AS_BUILDER = os.path.join(LXBIN, AS_BUILDER)

# vim: ts=8:sw=4:tw=80:et:
