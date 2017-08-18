#!/usr/bin/env python2.7
"""This is for playing around with yaml data."""

from __future__ import print_function
from __future__ import unicode_literals

import preface

import argparse
import sys

# Look for the fast C-based implementation first.
import yaml
try:
    from yaml import CLoader as Loader
except ImportError:
    from yaml import Loader

import lib.util


def main():
    """This is just for playing with yaml."""
    parser = argparse.ArgumentParser()
    parser.add_argument(
        'dbfile', nargs='?',
        help="yaml file")
    opts = parser.parse_args()

    if opts.dbfile:
        with open(opts.dbfile, 'r') as f:
            root = yaml.load(f, Loader=Loader)
    else:
        root = {
            'repo_map': {
                'BIS715GALA_BRANCH_7_21': 'gala_airport',
                'PHO2203RC1_TWIG_6_25_178': 'monarch_43342_sdio',
            }
        }

    yaml.dump(lib.util.bytestrings(root), sys.stdout,
              indent=2, default_flow_style=False)

if __name__ == '__main__':
    main()

# vim: ts=8:sw=4:tw=80:et:
