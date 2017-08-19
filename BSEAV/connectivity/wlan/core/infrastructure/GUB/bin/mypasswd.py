#!/usr/bin/env python2.7
"""Return the user's cached svn password."""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import os
import getpass


def get_svn_password(realm=None, user=None):
    """Return the user's cached svn password, filtering by realm."""
    if not realm:
        realm = 'http://svn.sj.broadcom.com'

    if not user:
        user = getpass.getuser()

    exdir = os.path.expanduser('~/.subversion/auth/svn.simple')
    if not os.path.isdir(exdir):
        return None

    found = {}
    filelist = [fn for fn in os.listdir(exdir) if len(fn) == 32]
    for path in [os.path.join(exdir, x) for x in filelist]:
        username = password = realmstring = ''
        ustate = pstate = rstate = 0
        for line in open(path, 'r').readlines():
            line = line.rstrip()
            if pstate == 0 and line == 'password':
                pstate = 1
            elif pstate == 1 and line.startswith('V '):
                pstate = 2
            elif pstate == 2:
                pstate = -1
                password = line

            if ustate == 0 and line == 'username':
                ustate = 1
            elif ustate == 1 and line.startswith('V '):
                ustate = 2
            elif ustate == 2:
                ustate = -1
                username = line

            if rstate == 0 and line == 'svn:realmstring':
                rstate = 1
            elif rstate == 1 and line.startswith('V '):
                rstate = 2
            elif rstate == 2:
                rstate = -1
                if realm in line:
                    realmstring = realm

            if username and password and realmstring:
                break
        found['+'.join([realmstring, username])] = password

    return found.get('+'.join([realm, user]))


def main():
    """Return the user's cached svn password."""
    parser = argparse.ArgumentParser(
        epilog=main.__doc__.strip(),
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument(
        '-r', '--realm',
        help="a required fragment of the realm string")
    parser.add_argument(
        '-u', '--user',
        help="the username whose password is desired")
    opts = parser.parse_args()

    print(get_svn_password(realm=opts.realm, user=opts.user))

if __name__ == '__main__':
    main()

# vim: ts=8:sw=4:tw=80:et:
