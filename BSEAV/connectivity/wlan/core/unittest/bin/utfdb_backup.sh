#!/bin/sh -ex
file=/projects/hnd_sig/utfdb_backups/utfdb_$(date +'%Y.%m.%d').sql
mysqldump --user=utf -p --all-databases --result-file $file
gzip $file


