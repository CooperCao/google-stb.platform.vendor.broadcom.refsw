#! /usr/bin/env python
#########################################################################
# (c) 2014-2015 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its
# licensors, and may only be used, duplicated, modified or distributed pursuant
# to the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied), right
# to use, or waiver of any kind with respect to the Software, and Broadcom
# expressly reserves all rights in and to the Software and all intellectual
# property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
#    constitutes the valuable trade secrets of Broadcom, and you shall use all
#    reasonable efforts to protect the confidentiality thereof, and to use
#    this information only in connection with your use of Broadcom integrated
#    circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
#    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
#    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
#    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
#    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
#    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
#    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
#    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
#    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
#    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
#########################################################################
#
#  Conversion tool to take the SID S-Record hex file and convert it to a
#  C file containing the binary image data for inclusion into SID PI
#
#  Run without arguments to convert the default FW image
#  sid.hex => bsid_fw.c
#
#  Source file expected in current directory (directory script was launched from)
#

from __future__ import print_function
import sys
import argparse
import os
import subprocess
import re

_DEFAULT_TOOL = 'srec'
_DEFAULT_IN_FILE = "sid.hex"
_DEFAULT_OUT_FILE = "bsid_fw.c"
_DEFAULT_DEST_DIR = ".."
_DEFAULT_CHUNK_SIZE = "49152" # 48Kb
_DEFAULT_ARRAY_PREFIX = "BSID_FW_Sid"
_DEFAULT_TEMPLATE_FILE = "bsid_fw_prot.c"
_DEFAULT_ENDIAN = "LE"

def process_args():
   parser = argparse.ArgumentParser(description="SID S-record conversion tool", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
   parser.add_argument('-in', metavar="SOURCE_FILE", dest='source', help='S-Record file to convert', default=_DEFAULT_IN_FILE)
   parser.add_argument('-out', metavar="OUTPUT_FILE", help='Filename to write the C file to', default=_DEFAULT_OUT_FILE)
   parser.add_argument('-dest', metavar='DEST_DIR', help='Destination directory for the output', default=_DEFAULT_DEST_DIR)
   parser.add_argument('-chunk', metavar='CHUNK_SIZE', dest='size', help='Chunk/Array size to break the FW into (in bytes)', default=_DEFAULT_CHUNK_SIZE)
   parser.add_argument('-prefix', help='Prefix to use for output array names (e.g: <prefix>_0[ ] etc)', default=_DEFAULT_ARRAY_PREFIX)
   parser.add_argument('-endian', choices=['BE', 'LE'], help='Endianness of the output data words', default=_DEFAULT_ENDIAN)
   parser.add_argument('-template', metavar='TEMPLATE_FILE', help='Specify the file to use as the template for the output C file', default=_DEFAULT_TEMPLATE_FILE)
   return parser.parse_args()

if __name__ == '__main__':
   args = process_args()
   dest_file = os.path.join(args.dest, args.out)
   print("Converting %s to %s ..." % (args.source, dest_file))
   # run the "srec" utility to convert the file
   cmd_line = ["./{tool}".format(tool=_DEFAULT_TOOL), "-hex", args.source, "-out", dest_file, "-prefix", args.prefix, "-template", args.template, "-size", args.size, "-endian", args.endian]
   try:
      #print("Executing: '%s'" % " ".join(cmd_line))
      output = subprocess.check_output(cmd_line, stderr=subprocess.STDOUT)
   except subprocess.CalledProcessError as why:
      print("Unable to execute '{tool}' utility...".format(tool=_DEFAULT_TOOL))
      print(output)
      sys.exit(1)
   except (IOError, OSError) as why:
      print(why)
      sys.exit(1)
   print("Post-processing output...")
   try:
      with open(dest_file, "r") as fw_file:
         fw_data = fw_file.read()

      # Remove trailing whitespace in output (git doesn't like it)
      print("\tRemoving trailing whitespace...")
      fw_data = re.sub(r'[ \t]+\n', '\n', fw_data)

      # Fix size of arrays (should be variable size as per SW7435-939)
      # Look for <prefix>_<num>[<int>]
      # replace <int> with nothing
      print("\tFixing array sizes for SW7435-939...")
      fw_data = re.sub(r'({prefix}_\d+)\[\d+\]'.format(prefix=args.prefix), r'\1[]', fw_data)

      print("\tAdding chunk size information...")
      # Write the size information to the file for use by loader
      match = re.search(r'(const void \*){prefix}\[(\d+)\]='.format(prefix=args.prefix), fw_data)
      if not match:
         print("Unable to locate: 'const void *{prefix}[] array ... output from {tool} is invalid".format(prefix=args.prefix, tool=_DEFAULT_TOOL))
         sys.exit(1)
      fw_data += "const unsigned {prefix}_ChunkSize = {size};\n".format(prefix=args.prefix, size=args.size)
      fw_data += "const unsigned {prefix}_NumEntries = {num_entries};\n".format(prefix=args.prefix, num_entries=match.group(2))

      print("\tFixing not-const-globals violations...")
      # re-write array definitions to avoid not-const-globals static anaylsis violation
      # see: http://twiki-01.broadcom.com/bin/view/Bseavsw/NexusStaticAnalysis)
      # look for const void *<prefix>[<int>]
      # replace "const void *<prefix> with const void * const <prefix>
      fw_data = re.sub(r'const void \*{prefix}\['.format(prefix=args.prefix), r'const void * const {prefix}['.format(prefix=args.prefix), fw_data)

      # write the modified result back
      # NOTE: open as binary to ensure file is written as Unix mode
      with open(dest_file, "wb") as fw_file:
         fw_file.write(fw_data)
   except (IOError, OSError) as why:
      print(why)
      sys.exit(1)
   print("Done")

#########################################################################
#
# End of File
#
#########################################################################
