#!/bin/sh

# Script to run iov/cmd update and see if any changes appear
# This script, the immediate output and the version currently
# integrated into the twiki pages are all in the same place:
# src/tools/misc.

# Where are scripts
SCRIPT_DIR=/projects/hnd_software/gallery/src/tools/misc
PARSE_SCRIPT=${SCRIPT_DIR}/iov-cmd-parse.pl

# Where is output
OUTPUT_DIR=/home/dtalayco/pub/iov-cmd-output
CMD_OUTPUT=${OUTPUT_DIR}/cmd_tables.txt
IOV_OUTPUT=${OUTPUT_DIR}/iov_tables.txt

# Where is current info
CURRENT_DIR=$SCRIPT_DIR
CMD_CURRENT=${CURRENT_DIR}/cmd_tables.current
IOV_CURRENT=${CURRENT_DIR}/iov_tables.current

EMAIL_LIST=dtalayco
DIFF=/tools/bin/diff

perl $PARSE_SCRIPT

$DIFF -I 'list content generated' $CMD_CURRENT $CMD_OUTPUT > /dev/null || \
    $DIFF -u -I 'list content generated' $CMD_CURRENT $CMD_OUTPUT | \
    mailx -s "Autogen commands changed" $EMAIL_LIST

$DIFF -I 'Variable content generated' $IOV_CURRENT $IOV_OUTPUT > /dev/null || \
    $DIFF -u -I 'Variable content generated' $IOV_CURRENT $IOV_OUTPUT | \
    mailx -s "Autogen IOVars changed" $EMAIL_LIST
