#!/bin/bash

# run gitprep on all atlas source files
#   - replace existing files and do not save a backup
#   - ignore linenoise opensource software

# these call do NOT include ocap directory - add later
GITPREP="git prep -u"

# gitprep *.h *.c and *.cpp files
find . -iregex '.*\.\(c\|cpp\|h\)' \
    -not -path "./ocap/*"       \
    -not -path "./linenoise/*"  \
    -print0 | xargs -0 -n1 $GITPREP
    #-exec $GITPREP {} \; # use xargs instead of -exec so $GITPREP return value is preserved.

if [ $? -eq 0 ]; then
    printf "\033[1;32mSUCCESS!\033[0m\n"
else
    printf "\033[1;31mFAILURE!\033[0m\n"
fi
