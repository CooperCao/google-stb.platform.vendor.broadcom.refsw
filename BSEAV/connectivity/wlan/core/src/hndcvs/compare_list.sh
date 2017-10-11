#!/bin/bash

if [ "$1" = "-d" ]; then
    defs=$2
    shift 2
fi

echo 1
`./rem_comments.awk $1 > list1.h`

echo 2

`./rem_comments.awk $2 > list2.h`

echo 3

make DEFS="$defs" FILE1=list1.h FILE2=list2.h -f diff_list.mk preprocess

./print_list.awk gcc_pre1 gcc_pre2
