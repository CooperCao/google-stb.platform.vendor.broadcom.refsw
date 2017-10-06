"""Operations related to IP (intellectual property) checking."""

from __future__ import print_function
from __future__ import unicode_literals

import os
import re
import shutil

import lib.util

WIKI = 'http://confluence.broadcom.com/display/WLAN/BuildIPValidation'
OPEN = 'open'
PROPRIETARY = 'proprietary'
DUAL_1 = r'Proprietary,Open'
DUAL_2 = r'Open,Proprietary'
IPTAG_MAP = 'IPTAG_MAP'
IPTAG_START = '(?:<<|<string>)'
IPTAG_END = '(?:</string>|>>)'
IPTAG_ROOT = 'Broadcom-WL-IPTag'
IPTAG_OPEN_LINE = r'%s/Open:.*' % IPTAG_ROOT
IPTAG_PROP_LINE = r'%s/Proprietary:.*' % IPTAG_ROOT
IPTAG_DUAL_LINE_1 = r'%s/%s.*' % (IPTAG_ROOT, DUAL_1)
IPTAG_DUAL_LINE_2 = r'%s/%s.*' % (IPTAG_ROOT, DUAL_2)
IPTAG_FILE = 'BRCM_IP_TAG.txt'
VENDOR = 'vendor'
PRIVATE = 'private'

# Router builds generate stub .o files which in some cases can be short enough
# to look like text files. Ignore "built-in.o" since it cannot have an IP tag.
WHITELIST = ('built-in.o',)


def allowed_re(iptags):
    """Turn a list of allowed IP tags into a regular expression."""
    allowed = set()
    for iptag in iptags.split(','):
        iptagl = iptag.lower()
        if iptagl == OPEN:
            allowed.add(IPTAG_OPEN_LINE)
            allowed.add(IPTAG_DUAL_LINE_1)
            allowed.add(IPTAG_DUAL_LINE_2)
        elif iptagl == PROPRIETARY:
            allowed.add(IPTAG_PROP_LINE)
            allowed.add(IPTAG_DUAL_LINE_1)
            allowed.add(IPTAG_DUAL_LINE_2)
        elif iptagl.startswith(VENDOR):
            allowed.add(r'%s/%s:.*' % (IPTAG_ROOT, iptag))
        elif iptagl.startswith(PRIVATE):
            # Literal angle brackets cannot be used in xml text.
            # The normal solution would be to put the ip tag in
            # a comment but Xcode removes comments in plists.
            allowed.add(r'%s/%s:.*' % (IPTAG_ROOT, iptag))
        else:
            lib.util.error('invalid key "%s" in %s' % (iptag, iptags))
            return None

    return '%s(?:%s)%s' % (IPTAG_START, '|'.join(sorted(allowed)), IPTAG_END)


def checkfile(apath, rpath, check_re=None, iptags=None):
    """
    Check a single file for proper IP tags.

    If iptags is provided an RE is generated from it. If not, the
    RE that's passed in is used. If neither is given, check only for
    presence of an IP tag.
    """

    if lib.util.verbose_enough(3):
        print('Checking ' + rpath)

    # These should never leak into a release.
    if os.path.basename(apath) in ('.git', '.svn'):
        lib.util.error('SCM metadata leakage: %s' % rpath)
        return False

    contents = lib.util.get_file_contents_if_utf8(apath)
    if not contents:
        return True

    if IPTAG_ROOT not in contents:
        if os.path.basename(apath) not in WHITELIST:
            lib.util.error('no ip tag in text file %s' % rpath)
            return False
    else:
        check_re = (allowed_re(iptags) if iptags else check_re)
        if check_re:
            # Check whether this file's IP tag is valid.
            if not re.search(check_re, contents):
                lib.util.error('invalid ip tag in %s' % rpath)
                return False

    return True


def checkdir(release, iptags):
    """Check all source files in directory for proper IP tags."""
    def find_ip_files(path):
        """Return the set of files in the specified tree."""
        ipfiles = {}
        for parent, dnames, fnames in os.walk(path):
            for dname in dnames[:]:
                if dname == '.svn' or dname == '.git':
                    dnames.remove(dname)

                    # These should never be here. Return them
                    # so they can be dealt with by the caller.
                    apath = os.path.join(parent, dname)
                    rpath = lib.util.shortpath(apath, path)
                    ipfiles[apath] = rpath
                elif os.path.isfile(os.path.join(parent, dname, IPTAG_FILE)):
                    dnames.remove(dname)

            for fname in fnames:
                # Ignore generated .map & .dis files since these files can
                # help debug crashes without forcing customers to build.
                if fname.endswith('.dis') or fname.endswith('.map'):
                    continue
                apath = os.path.join(parent, fname)
                rpath = lib.util.shortpath(apath, path)
                ipfiles[apath] = rpath

        return ipfiles

    dtmp = lib.util.tmpdir(prefix='ipval.')
    iptmp = os.path.join(dtmp, os.path.basename(os.path.abspath(release)))
    lib.util.xtrace('cp -r %s %s' % (release, iptmp), vl=2)
    shutil.copytree(release, iptmp)

    # Extract source packages. Other archive types and recursive
    # archive extraction are deferred until needed.
    for path in find_ip_files(iptmp):
        if path.endswith('tar.gz'):
            cmd = ['tar', '-C', os.path.dirname(path), '-xzf', path]
            if lib.util.execute(cmd, vl=2) != 0:
                return False

    # Do a final traversal after archives are expanded.
    passed = True
    check_re = (allowed_re(iptags) if iptags else None)
    ip_files = find_ip_files(iptmp)
    for apath in sorted(ip_files):
        if not checkfile(apath, ip_files[apath], check_re=check_re):
            passed = False

    return passed

# vim: ts=8:sw=4:tw=80:et:
