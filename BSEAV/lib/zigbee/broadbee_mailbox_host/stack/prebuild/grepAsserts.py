#!/usr/bin/env python3
#############################################################################
# Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to the terms and
# conditions of a separate, written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# no license (express or implied), right to use, or waiver of any kind with respect to the
# Software, and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of Broadcom integrated circuit products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
# THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
# LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
# OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
# USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
# USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
# ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
# LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
# ANY LIMITED REMEDY.
#############################################################################

import fnmatch
import os
import re
import sqlite3
from argparse import ArgumentParser
from os.path import abspath, dirname, join, basename

pattern_skip = re.compile('(?:(?:# *define )|(?: TODO)|(?://)).*', flags=re.MULTILINE)
assert_pattern = re.compile('SYS_Dbg'
                            '(?:(?:Assert\(.*?,)|'
                            '(?:AssertComplex\(.*?,)|'
                            '(?:AssertLog\(.*?,)|'
                            '(?:LogId\()|'
                            '(?:Halt\())'
                            '.*?'
                            '(?P<content>\w+)\s*\);', flags=re.DOTALL)

python_header = """#!/usr/bin/env python3
from enum import IntEnum

class UIDs(IntEnum):
    zero                                                                                                    = 0x00000000
"""


def get_matches(fpath, pattern):
    result = []
    with open(fpath, encoding='iso-8859-1') as f:
        text = pattern_skip.sub('', f.read())
        for match in pattern.finditer(text):
            match_id = match.group('content')
            if match_id != 'SYS_ATOMIC_DEFAULT_UID':
                result.append(match_id)
    return result


def process_path(path, pattern):
    for root, _, files in os.walk(path):
        for file in fnmatch.filter(files, '*.[ch]'):
            fpath = join(root, file)
            asserts = get_matches(fpath, pattern)
            if asserts:
                yield '\n    /* ' + basename(fpath) + ' */\n'
            for assert_id in asserts:
                yield assert_id


def get_args():
    parser = ArgumentParser(description='Grep Asserts')
    parser.add_argument('-i',
                        dest='stack_path',
                        required=False,
                        help='Path to stack')
    options = parser.parse_args()
    return options


def grep_asserts(stack_path=None):
    if stack_path is None:
        opts = get_args()
        stack_path = abspath(opts.stack_path or dirname(dirname(__file__)))
    projects_path = join(dirname(stack_path), 'projects')
    prebuild_path = join(stack_path, 'prebuild')

    output_file_header = join(prebuild_path, 'bbSysDbgUids_head_h.txt')
    output_file_footer = join(prebuild_path, 'bbSysDbgUids_tail_h.txt')
    output_file = join(stack_path, 'common', 'System', 'include', 'bbSysDbgUids.h')
    output_python = join(prebuild_path, 'uids.py')
    output_db = join(prebuild_path, 'uids.db')

    with open(output_file_header) as f:
        c_header = f.read()
    with open(output_file_footer) as f:
        c_footer = f.read()

    counter = 0x08020000
    with open(output_python, mode='w') as pf, \
            open(output_file, mode='w') as cf, \
            sqlite3.connect(output_db) as db:
        pf.write(python_header)
        cf.write(c_header)
        db.execute('CREATE TABLE IF NOT EXISTS t_uids (id INTEGER PRIMARY KEY, name TEXT UNIQUE NOT NULL)')
        db.execute('INSERT OR REPLACE INTO t_uids (id, name) VALUES (0, "zero")')
        ### for folder in (stack_path, projects_path):  ### Stack side
        for folder in (stack_path, ""):  ### Host side
            for assert_id in process_path(folder, assert_pattern):
                if assert_id.startswith('\n'):
                    cf.write(assert_id)
                else:
                    cf.write("    {:<102s} = 0x{:08x},\n".format(assert_id, counter))
                    pf.write("    {:<103} = 0x{:08x}\n".format(assert_id, counter))
                    db.execute('INSERT OR REPLACE INTO t_uids (id, name) VALUES (?, ?)', (counter, assert_id))
                    counter += 1
        cf.write(c_footer)
        db.commit()


if __name__ == '__main__':
    grep_asserts()

## eof grepAsserts.py #######################################################
