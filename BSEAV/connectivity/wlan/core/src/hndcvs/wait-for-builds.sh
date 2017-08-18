#!/bin/bash

create_links=N

while [ "$1" != "" ]; do
    if [ "$1" = "-b" ]; then
	alist=($2)
	blist=$2
	shift 2
    elif [ "$1" = "-link" ]; then
	create_links=Y
	shift
    else
	echo unrecognized option
	exit 1
    fi
done

tmplist=""
i=0
n=0
ilist=""
all=0

for b in $blist; do
    ilist="$ilist $all" 
    let all=all+1
done

echo ilist : $ilist

local_dir=`pwd`
link_dir=`dirname $local_dir`
date=`basename $local_dir`

echo  $all 


while [ "$ilist" != "" ]; do
    i=0
    sleep 1
    nlist=""
    for i in $ilist; do
	if [ -e ${alist[i]}/BUILD_DONE ]; then
	    echo ${alist[i]} is done
	    if [ "$create_links" = "Y" ]; then
		mkdir -p ../../${alist[i]}
		echo ln -s $local_dir/${alist[i]}/build_tree ../../${alist[i]}/$date
		ln -s $local_dir/${alist[i]}/build_tree ../../${alist[i]}/$date
	    fi
	else
	    nlist="$nlist $i"
	fi
    done 
    ilist="$nlist"
done
