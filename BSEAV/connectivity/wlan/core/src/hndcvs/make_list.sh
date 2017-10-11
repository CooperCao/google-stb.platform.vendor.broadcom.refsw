#!/bin/bash

filter=0
infile=""
outfile=master_filelist.txt
master_list=""
module=""

read_options=1

while [ "$read_options" = "1" ] ; do

if [ "$1" = "-f" ]; then
    filter=1
    shift

elif [ "$1" = "-defs" ]; then
    defs=$2
    shift 2

elif [ "$1" = "-m_udefs" ]; then
    mudefs=$2
    shift 2
elif [ "$1" = "-m_defs" ]; then
    mdefs=$2
    shift 2
elif [ "$1" = "-i" ]; then
    infile=$2
    shift 2

elif [ "$1" = "-o" ]; then
    outfile=$2
    shift 2

# sub modules
elif [ "$1" = "-m" ]; then
    module=$2
    shift 2
 
# master list
elif [ "$1" = "-ml" ]; then
    bom=$2
    tag=$3
    master_list=1
    shift 1
else
    read_options=0

fi;
done

HNDCVS=src/hndcvs/hndcvs_backend
#
# when buiilding the master list, we use the -l (local) option avoid 
# checking out all module filelists.
# This means that we assume that the checkout already occured 
# 
if [ "$master_list" = "1" ]; then
    if [ "$module" != "" ]; then 
	echo  $HNDCVS -l -defs "$defs" -of $outfile -m $module -flst $bom $tag
	$HNDCVS -l -defs "$defs" -of $outfile -m $module -flst $bom $tag
    else
	echo $HNDCVS -l -defs "$defs" -of $outfile  -flst $bom $tag
	$HNDCVS -l -defs "$defs" -of $outfile  -flst $bom $tag
    fi
    # append to input file, if any
    if [ "$infile" != "" ]; then
	cp $outfile list_tmp
	cat $infile list_tmp > $outfile
	rm list_tmp
    fi
fi

if [ $filter = 1 ]; then
    # fix the defs, to be used later
    # when we pass DEFS and UNDEFS directly
    defines="$defs"
    for var in $mdefs; do
	defines="$defines -D"$var"_SRC"
    done
    for var in $mudefs; do
	defines="$defines -DNO_"$var"_SRC"
    done
    #remove incompatible comments and copy to a .h file for gcc 
    ./src/hndcvs/rem_comments.awk $infile > filelist_tmp.h
    gcc  -E -undef $defines -Ulinux -o $outfile filelist_tmp.h

    rm filelist_tmp.h
fi
