#!/usr/bin/env python3
# Do not remove or change top string! This is mandatory for UNIX and Linux compatibility.
import os
import shutil
import fnmatch
import re
from os.path import abspath, dirname, join, exists, basename
from codecs import open as codecs_open

cnt = 0x00010000

# file paths from the stack folder
curdir = abspath(dirname(__file__))
stack_root = abspath(join(curdir, '..'))

headFilePath = r"prebuild/bbSysAtomicUids_head_h.txt"
tailFilePath = r"prebuild/bbSysAtomicUids_tail_h.txt"
outputFilePath = r"common/System/include/bbSysAtomicUids.h"

pattern_excl = re.compile(r'(# *define )|( TODO)|(^//)')
pattern_start = re.compile(r'ATOMIC_SECTION_ENTER\(')
pattern = re.compile(r'ATOMIC_SECTION_ENTER\((\w+)\)')


def find_ch(top=stack_root):
    for path, dirList, fileList in os.walk(top):
        for fileName in fnmatch.filter(fileList, '*.[ch]'):
            yield join(path, fileName)


def append_filename(file_obj, filename):
    rec = '    /* ' + basename(filename) + ' */'
    file_obj.write('\n%s\n' % rec)


def append_entry(file_obj, atomic_id):
    global cnt
    file_obj.write("    {:<102s} = 0x{:08x},\n".format(atomic_id, cnt))
    # rec = '    ' + atomic_id
    # rec = rec.ljust(max(len(rec + '\t'), 76))
    # file_obj.write('%s= 0x%08x,\n' % (rec, cnt))
    cnt += 1


def print_entry(filename, string, atomic_id):
    print('\n')
    print(filename)
    print('====================================\n')
    print(string)
    print(atomic_id)


def grepAtomics(top=stack_root):
    err_msg = '*** system error ***'
    filename = '*** not initialized yet ***'
    string = '*** not initialized yet ***'
    # head_file = top + "/" + headFilePath
    # tail_file = top + "/" + tailFilePath
    # output_file = top + "/" + outputFilePath
    head_file = join(top, headFilePath)
    tail_file = join(top, tailFilePath)
    output_file = join(top,  outputFilePath)

    cur_file_obj = None
    tail_file_obj = None
    output_file_obj = None

    try:
        if exists(output_file):
            os.remove(output_file)
        shutil.copyfile(head_file, output_file)
        output_file_obj = open(output_file, 'a')

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
                                atomic_id = match.expand(r'\1')

                                if atomic_id != 'SYS_ATOMIC_DEFAULT_UID':
                                    if shall_append_filename:
                                        append_filename(output_file_obj, filename)
                                        shall_append_filename = False
                                    print_entry(filename, string, atomic_id)
                                    append_entry(output_file_obj, atomic_id)

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
        print('Please use the following format: grepAtomics.py -i <stack path>')
        exit()
    grepAtomics(top)
