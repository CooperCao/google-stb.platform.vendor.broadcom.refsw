#!/bin/sh

# run uncrustify on all atlas source files using ../tools/uncrustify.cfg file settings
#   - replace existing files and do not save a backup
#   - ignore linenoise opensource software
#   - ignore atlas_cfg.h since it contains some tricky code reuse macros that confuse uncrustify
#   - ignore convert.cpp since it contains a lot of macros that confuse uncrustify

# these call do NOT include ocap directory - add later

# uncrustify *.c and *.cpp files
find . -iregex '.*\.\(c\|cpp\)' \
    -not -path "./ocap/*"       \
    -not -path "./linenoise/*"  \
    -not -path "./convert.cpp"  \
    -exec ../tools/uncrustify-0.60/src/uncrustify -c ../tools/uncrustify_c_cpp.cfg --replace --no-backup {} \;

# uncrustify *.h files
find . -iregex '.*\.\(h\)'      \
    -not -path "./ocap/*"       \
    -not -path "./linenoise/*"  \
    -not -path "./atlas_cfg.h"  \
    -exec ../tools/uncrustify-0.60/src/uncrustify -c ../tools/uncrustify_h_hpp.cfg --replace --no-backup {} \;
