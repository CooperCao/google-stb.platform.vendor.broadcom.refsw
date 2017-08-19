"""Parse an SVN log object and return various parts of the message."""

from __future__ import print_function
from __future__ import unicode_literals

import re
import xml.dom.minidom


class SVNLog(object):

    """Record for an individual svn log entry."""

    def __init__(self, node):
        self._revision = int(node.getAttribute('revision'))
        self._author = node.getElementsByTagName('author')[0].firstChild.data
        self._revdate = node.getElementsByTagName('date')[0].firstChild.data
        self._msg = node.getElementsByTagName('msg')[0].firstChild.data
        self._dirlist = []
        self._filelist = []

        # some files need to be masked out of the transfer process
        self._excludes = [
            'whitelist'
        ]

        paths = node.getElementsByTagName('path')
        for path in paths:

            # Debug print self._revision, p.firstChild.data
            #
            # Turns out src/tools/release/*filelist.txt can be changed to add
            # new source files to the .tgz so filtering out src/tools/release
            # can generate bugs. Have to build,untgz,git diff, then decide.
            #
            # # src/tools/release files do not count
            # #if 'src/tools/release' in p.firstChild.data:
            # #    continue

            if 'src/tools/build/rename_fw_cfg.yaml' in path.firstChild.data:
                continue

            if 'file' == path.attributes['kind'].value:
                self._filelist.append([
                    path.attributes['action'].value,
                    path.firstChild.data  # filepath and name
                ])

            else:
                self._dirlist.append([
                    path.attributes['action'].value,
                    path.firstChild.data  # filepath and name
                ])

    def is_excluded(self):
        """Commit sets are small. if an excluded file is present
           exclude whole set."""

        filelist = self._filelist[:]

        for fileobj in self._filelist:
            for exfile in self._excludes:
                if fileobj[-1].endswith('/' + exfile):
                    filelist.remove(fileobj)

        if not len(filelist):
            return True

        return False

    def get_revision(self):
        """return"""

        return self._revision

    def get_author(self):
        """return"""

        return self._author

    def get_revdate(self):
        """return"""

        return self._revdate

    def get_msg(self):
        """return"""

        return self._msg

    def get_filemods(self):
        """return"""

        return [item for item in self._filelist if item[0] == 'M']

    def get_filedels(self):
        """return"""

        return [item for item in self._filelist if item[0] == 'D']

    def get_fileadds(self):
        """return"""

        return [item for item in self._filelist if item[0] == 'A']


class SVNLogSet(object):

    """Full set of svn logs for a given branch."""

    def __init__(self, filename):
        self.filename = filename
        self.xmldoc = xml.dom.minidom.parseString(filename)
        self.logset = []
        self.iterint = 0

        self.parse()

    def parse(self):
        """Get elements from the xml."""

        svnlist = self.xmldoc.getElementsByTagName('logentry')

        for node in svnlist:
            self.logset.append(SVNLog(node))

    def get_logs(self):
        """Get logs in reverse order."""

        logset = self.logset[:]
        logset.reverse()
        return logset

    def __iter__(self):
        return self

    def next(self):
        """Get next record."""

        if self.iterint >= len(self.logset):
            raise StopIteration()
        logrec = self.logset[self.iterint]
        self.iterint += 1
        return logrec


def jira_match(onelog):
    """Regex search for jira match."""

    jira = onelog.get_msg()
    jira = ' '.join(jira.split('\n'))
    match = re.search('^.*JIRA:(SW[A-Z0-9-]+).*', jira)
    jira = (match.group(1) if match else None)
    return jira

# vim: ts=8:sw=4:tw=80:et:
