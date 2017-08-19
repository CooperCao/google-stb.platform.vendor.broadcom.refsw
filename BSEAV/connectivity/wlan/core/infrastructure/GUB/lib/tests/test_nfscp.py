"""Test nfscp methods."""

from __future__ import print_function
from __future__ import unicode_literals

import logging
import os
import shutil
import sys

from nose.tools import raises, assert_raises, assert_raises_regexp

import general.tests.all_general as alg

import nfscp

FORMAT = ('%(asctime)s %(module)s.%(funcName)s,%(lineno)d ' +
          '%(levelname)s %(message)s')


class NfscpBase(alg.TstBase):

    testfiles = 'testfiles'
    test_group = 'nfscp_tests'
    dir_top = 'top'
    loglevel = logging.DEBUG

    def setup(self, *args, **kws):
        self.set_logging()
        alg.TstBase.setup(self, *args, **kws)
        self.make_wkspace()

    def set_logging(self):
        handler = logging.StreamHandler(sys.stdout)
        handler.setLevel(level=logging.DEBUG)
        handler.setFormatter(logging.Formatter(FORMAT))
        logging.getLogger('').addHandler(handler)

    def make_wkspace(self):
        self.top = os.path.join(self.testdir, self.dir_top)
        logging.info('Make workspace in %s' % self.top)
        self.make_hier(self.top)

    def make_hier(self, level_top, depth=4):
        """Make hierarchy of files and directories up to given depth."""

        for dir_ in [os.path.join(level_top, 'dir%d' % i)
                     for i in range(5)]:
            shutil.copytree(self.testfiles, os.path.join(level_top, dir_))
            if depth:
                self.make_hier(dir_, depth=depth - 1)

    def set_mode(self, top=None, mode=0):
        """Set file and directory permission mode in a hierarchy."""

        if top is None:
            top = self.top
        logging.info('Set mode in %s' % top)

        for dirpath, dirnames, filenames in os.walk(top, topdown=False):
            for f in dirnames + filenames:
                os.chmod(os.path.join(dirpath, f), mode)


class TestCopy(NfscpBase):

    def test_basic(self):
        """copy: Basic."""

        assert os.listdir(self.top)
        orig_hash = self.make_path_hashes(self.top)
        target = self.top + '1'
        tree_sync = nfscp.TreeSync(self.top, target)
        tree_sync.copy()
        copy_hash = self.make_path_hashes(target)
        self.verify_hashes(orig_hash, copy_hash)


class TestRmtree(NfscpBase):

    def test_basic(self):
        """rmtree: Basic."""

        assert os.listdir(self.top)
        tree_sync = nfscp.TreeSync(self.top, self.top)
        tree_sync.rmtree(self.top)
        assert not os.path.exists(self.top)

    def test_no_perms(self):
        """rmtree: Files/directories without permissions."""

        self.set_mode()
        dirs = os.listdir(self.top)
        assert dirs
        for dir_ in dirs:
            with assert_raises_regexp(OSError, 'Permission denied'):
                os.listdir(os.path.join(self.top, dir_))
        tree_sync = nfscp.TreeSync(self.top, self.top)
        tree_sync.rmtree(self.top)
        assert not os.path.exists(self.top)
