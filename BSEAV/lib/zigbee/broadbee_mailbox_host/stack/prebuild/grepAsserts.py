#!/usr/bin/env python3
# Do not remove or change top string! This is mandatory for UNIX and Linux compatibility.
import os
import shutil
import fnmatch
import re
from os.path import abspath, dirname, join, exists, basename
import sys
from codecs import open as codecs_open

cnt = 0x00010000

# file paths from the stack folder
curdir = abspath(dirname(__file__))
stack_root = abspath(join(curdir, '..'))

head_file_path = r"prebuild/bbSysDbgUids_head_h.txt"
tail_file_path = r"prebuild/bbSysDbgUids_tail_h.txt"
output_file_path = r"common/System/include/bbSysDbgUids.h"

python_file_path = r"prebuild/uids.py"

pattern_excl = re.compile(r'(# *define )|( TODO)|(^//)')
pattern_start = re.compile(r'SYS_Dbg((Assert\()|(AssertComplex\()|(AssertLog\()|(LogId\()|(Halt\())')
pattern = re.compile(r'SYS_Dbg((Assert\(.*,)|(AssertComplex\(.*,)|(AssertLog\(.*,)|(LogId\()|(Halt\()) *(\w+)\);')


def find_ch(top=stack_root):
    for path, dirList, fileList in os.walk(top):
        for fileName in fnmatch.filter(fileList, '*.[ch]'):
            yield join(path, fileName)


def append_filename(file_obj, filename):
    rec = '    /* ' + basename(filename) + ' */'
    file_obj.write('\n%s\n' % rec)


def print_entry(filename, string, assert_id):
    print('\n')
    print(filename)
    print('====================================\n')
    print(string)
    print(assert_id)


def grepAsserts(top=stack_root):
    global cnt
    err_msg = '*** system error ***'
    filename = '*** not initialized yet ***'
    string = '*** not initialized yet ***'
    head_file = join(top, head_file_path)
    tail_file = join(top, tail_file_path)
    output_file = join(top, output_file_path)

    python_file = join(top, python_file_path)

    cur_file_obj = None
    tail_file_obj = None
    output_file_obj = None

    try:
        if exists(output_file):
            os.remove(output_file)

        if exists(python_file):
            os.remove(python_file)

        shutil.copyfile(head_file, output_file)
        output_file_obj = open(output_file, 'a')

        python_file_obj = open(python_file, mode='wt', newline='\n')
        python_file_obj.write('#!/usr/bin/env python3\n')
        python_file_obj.write('from enum import IntEnum\n')
        python_file_obj.write('\n')
        python_file_obj.write('class UIDs(IntEnum):\n')
        python_file_obj.write("    {:<103} = 0x{:08x}\n".format('zero', 0))

        file_names = find_ch(top)
        for filename in file_names:
            shall_append_filename = True
            with codecs_open(filename, 'r', encoding='iso-8859-1') as cur_file_obj:
                match_start = False
                for in_line in cur_file_obj:

                    if not match_start:
                        string = in_line.strip()
                        match_start = pattern_start.search(string) and not pattern_excl.search(string)

                    else:
                        string = string + in_line.strip()

                    if match_start:
                        match = pattern.search(string)
                        if match:
                            if len(pattern_start.findall(string)) == 1:
                                assert_id = match.expand(r'\7')

                                if shall_append_filename:
                                    append_filename(output_file_obj, filename)
                                    shall_append_filename = False
                                print_entry(filename, string, assert_id)

                                output_file_obj.write("    {:<102s} = 0x{:08x},\n".format(assert_id, cnt))
                                python_file_obj.write("    {:<103} = 0x{:08x}\n".format(assert_id, cnt))
                                cnt += 1

                                match_start = False

                            else:
                                err_msg = 'more than one occurence of the pattern in the line'
                                raise Exception

                else:
                    if match_start:
                        err_msg = 'a not terminated pattern'
                        raise Exception

            cur_file_obj.close()

        else:
            tail_file_obj = codecs_open(tail_file, 'r', encoding='iso-8859-1')
            output_file_obj.write(tail_file_obj.read())
            output_file_obj.close()
            tail_file_obj.close()
            python_file_obj.close()
            print('\n Grepping has been DONE!!!')

    except BaseException:
        print('FILE: %s' % filename)
        print('CODE: %s' % string)
        print('ERROR: %s' % err_msg)
        if cur_file_obj is not None:
            cur_file_obj.close()
        if tail_file_obj is not None:
            tail_file_obj.close()
        if output_file_obj is not None:
            output_file_obj.close()
        print('An error happens!!! The grepAsserts failed!!!')
        sys.exit()

#----------------------- Start here -------------------------
if __name__ == '__main__':
    from sys import argv, exit
    import getopt

    top = stack_root
    try:
        if len(argv) == 1:
            # raise Exception  # We can parse this scenario
            print('Please use the following format: grepAsserts.py -i <stack path>')
            print('Running as: grepAsserts.py -i %s' % top)
        try:
            opts, args = getopt.getopt(argv[1:], "hi:")
        except getopt.GetoptError:
            raise Exception

        for opt, arg in opts:
            if opt == '-h':
                raise Exception
            elif opt == '-i':
                top = arg
        if top is None:
            raise AttributeError
    except BaseException:
        print('Please use the following format: grepAsserts.py -i <stack path>')
        exit()
    grepAsserts(top)
