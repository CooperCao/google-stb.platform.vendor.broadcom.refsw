#!/bin/bash

# This script is a command line parameter logging script to collect usage
# information on the hndcvs infrastructure. Logged usage information is
# anonymous.

#LOG_FILE=/projects/hnd_software/gallery/hndcvs_log.txt
LOG_DIR_UNIX=/projects/hnd_swbuild/build_admin/logs/hndcvs/usage/
LOG_DIR_WIN=Z:/projects/hnd_swbuild/build_admin/logs/hndcvs/usage/
LOG_FILENAME=hndcvs_log.txt

if [[ -d $LOG_DIR_UNIX ]]; then
    # Construct the log path.
    LOG_FILE=$LOG_DIR_UNIX$LOG_FILENAME
    # Construct the logging string of the hndcvs parameter usage.
    log_string="$(date): \"$*\""
    # Append the command line paramters string to the log file.
    echo $log_string >> $LOG_FILE

elif [[ -d $LOG_DIR_WIN ]]; then
    # Construct the log path.
    LOG_FILE=$LOG_DIR_WIN$LOG_FILENAME
    # Construct the logging string of the hndcvs parameter usage.
    log_string="$(date): \"$*\""
    # Append the command line paramters string to the log file.
    echo $log_string >> $LOG_FILE
else
    echo "WARNING: Log directories '$LOG_DIR_UNIX' and '$LOG_DIR_WIN' don't exist."
fi
