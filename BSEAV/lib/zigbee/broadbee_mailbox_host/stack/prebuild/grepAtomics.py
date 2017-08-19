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

import re
from os.path import abspath, dirname, join

try:
    from grepAsserts import get_args, process_path, get_matches, pattern_skip
except ImportError:
    from .grepAsserts import get_args, process_path, get_matches, pattern_skip

atomics_pattern = re.compile('ATOMIC_SECTION_ENTER\((?P<content>.+?)\)', flags=re.DOTALL)


def grep_atomics(stack_path=None):
    if stack_path is None:
        opts = get_args()
        stack_path = abspath(opts.stack_path or dirname(dirname(__file__)))
    prebuild_path = join(stack_path, 'prebuild')

    output_file_header = join(prebuild_path, 'bbSysAtomicUids_head_h.txt')
    output_file_footer = join(prebuild_path, 'bbSysAtomicUids_tail_h.txt')
    output_file = join(stack_path, 'common', 'System', 'include', 'bbSysAtomicUids.h')

    counter = 0x08010000
    with open(output_file_header) as f:
        c_header = f.read()
    with open(output_file_footer) as f:
        c_footer = f.read()
    with open(output_file, mode='w') as cf:
        cf.write(c_header)
        for atomic_id in process_path(stack_path, atomics_pattern):
            if atomic_id.startswith('\n'):
                cf.write(atomic_id)
            else:
                cf.write("    {:<102s} = 0x{:08x},\n".format(atomic_id, counter))
                counter += 1
        cf.write(c_footer)


if __name__ == '__main__':
    grep_atomics()

## eof grepAtomics.py #######################################################
