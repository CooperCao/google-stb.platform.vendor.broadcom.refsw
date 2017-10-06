#!/usr/bin/evn python
import os, sys
from myutils import scanner
from myutils import process_ipadd
from myutils import firstword

nargs = len(sys.argv)

def main():
    print nargs
    if nargs < 2:
        print 'warning: no init file given, use the default one'
        initfile = '../../cmds/dut/ucc_cnxt.txt'
    else:
        initfile = sys.argv[1]   

    file = open(initfile)
    scanner(file, firstword)

    file.close()

if __name__ == "__main__":
    main()

