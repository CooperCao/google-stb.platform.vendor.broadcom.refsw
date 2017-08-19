#!/bin/bash

#copy all proprietary copyrighted to proprietary tree
#omit linux sub-dir since it has some of these (??)

#grep -l -r "Copy.*Broadcom" | xargs grep -L Open.*Broadcom | xargs -i cp --parents {} ../proprietary

# linux-router.mk  
# mkdir -p src/proprietary
# use hndcvs -k to build the union of all boms

tree_dir=$1

fix_sym=$2

mkdir -p $tree_dir

dir_list=""

# since we collapse under the most inclusive directory
# do not allow src and src/router to encompass everything
# the problem comes from singular files under those directories

do_not_keep="./src ./src/router"

create_dir_list ()
{
# create list of directories
file_list=$1
output_file=$2
dir_list=""
prev_dir=""
echo "" > tmplist.txt
for file in $file_list; do
    dir=${file%/*}
    if [ "$prev_dir" != "$dir" ]; then
	echo $dir >> tmplist.txt
	prev_dir=$dir
    fi
done

echo "" > $output_file
#eliminate duplicates and dirs already included in previous ones.
file_list=`sort tmplist.txt`
prev_dir="UNMATCHED"
dir_list=""
for dir in $file_list; do
    matched=false
    #echo dir : $dir
    if test ! -e $dir; then 
	continue
    fi
    for ignore in $do_not_keep; do
	if [ "$ignore" = "$dir" ]; then
	    matched=true
	    break
	fi
    done
    if [ "$matched" != "true" ]; then 
	if [ "$prev_dir" != "$dir" ]; then
	    matched=false
	    matched=${dir/$prev_dir\/*/true}
	    if [ "$matched" != "true" ]; then 
		echo $dir >> $output_file
		dir_list="$dir_list $dir" 
		prev_dir=$dir
	    fi
	fi
    fi
done
}


local_dir=`pwd`

# if the prop tree already exist, fix sym links

if test -e $tree_dir/src;  then 
# fix sym links
    cd $tree_dir
    links_to_open=`find ./src -path ./src/linux -prune -o -type l -name  "*" | grep -v src/linux | grep -v src/router/Makefile | grep -v src/cfe`
    cd $local_dir 
    links_to_prop=`find ./src -path ./src/linux -prune -o -type l -name  "*" | grep -v src/linux | grep -v src/router/Makefile | grep -v src/cfe`
    
    for link in $links_to_open; do
	rm -f  $tree_dir/$link
	echo ln -f -s $local_dir/$link $tree_dir/$link
	ln -f -s $local_dir/$link $tree_dir/$link
    done

    for link in $links_to_prop; do
	rm -f $local_dir/$link
	echo ln -f -s $local_dir/$tree_dir/$link $local_dir/$link
	ln -f -s $local_dir/$tree_dir/$link $local_dir/$link
    done
 
   # do linux itself
    if test -e $tree_dir/src/linux/linux; then
	rm -f $tree_dir/src/linux/linux
	echo ln -f -s $local_dir/src/linux/linux $tree_dir/src/linux/linux 
	ln -f -s $local_dir/src/linux/linux $tree_dir/src/linux/linux
    fi
    if test -e $tree_dir/src/linux/linux-2.6; then
	rm -f $tree_dir/src/linux/linux-2.6
	echo ln -f -s $local_dir/src/linux/linux-2.6 $tree_dir/src/linux/linux-2.6 
	ln -f -s $local_dir/src/linux/linux-2.6 $tree_dir/src/linux/linux-2.6
    fi
 
    if [ "$fix_sym" = "fix" ]; then 
	exit 0
    fi
fi

# do the proprietary first

# find list of proprietary files in source src.

prop_file_list=`find ./src -path ./src/linux -prune -o -type f -name  "*" | grep -v .src/linux | grep -v ./src/cfe | xargs grep -l -r "Copy.*Broadcom" | xargs grep -L Open.*Broadcom`

# after this call, $dir_list contains the list of highest level 
# dirs containing at least one proprietary file 
create_dir_list "$prop_file_list"  dir_list.txt 

cp tmplist.txt prop_tmplist.txt

#cat dir_list.txt



#echo "" > prop_tmplist.txt

# remove proprietary directories from source src
for dir in $dir_list; do
#    cp --parents -rf $dir $tree_dir/
#    rm -rf $dir
    parent=${dir%/*}
    mkdir -p $tree_dir/$parent
    mv $dir $tree_dir/$parent/
    # check it the parent is now empty
    #echo parent : $parent
    matched=false
    for ignore in $do_not_keep; do
	if [ "$ignore" = "$parent" ]; then
	    matched=true
	    break
	fi
    done
    parents_dirs=""
    #while [ "$parent_dirs" = "" ]; do
    if [ "$matched" = "false" ]; then
	parent_dirs=`find $parent -type f -name "*" | grep -v ./CVS`
            # echo parent dirs $parent_dirs
	if [ "$parent_dirs" = "" ]; then
	    echo $parent is empty, removing from open tree
	    cp -rf $parent/CVS $tree_dir/$parent
	    rm -rf $parent
	    dir=$parent
	fi
	#else
	 #   break
    fi
    #done
    echo    ln -f -s $local_dir/$tree_dir/$dir $dir
    ln -f -s $local_dir/$tree_dir/$dir $dir
done



# create links to open source directories
open_file_list=`find ./src -path ./src/linux -prune -o -type f -name  "*" | grep -v ./src/cfe | grep -v CVS`

create_dir_list "$open_file_list"  open_dir_list.txt

for dir in $dir_list; do    
    parent=${dir%/*}
    while test ! -e $tree_dir/$parent; do
	dir=$parent
	parent=${dir%/*}
	#echo $dir $parent
    done
    if test ! -e $tree_dir/$dir; then
	echo ln -f -s $local_dir/$dir $tree_dir/$dir
	ln -f -s $local_dir/$dir $tree_dir/$dir
    fi 
done

# link linux and cfe as well

#if test ! -e $tree_dir/src/linux/linux; then
#    mkdir -p $tree_dir/src/linux
#    ln -f -s $local_dir/src/linux/linux $tree_dir/src/linux/linux
#fi

if test -e $local_dir/src/linux/linux; then
    mkdir -p $tree_dir/src/linux
    echo ln -f -s $local_dir/src/linux/linux $tree_dir/src/linux/linux 
    ln -f -s $local_dir/src/linux/linux $tree_dir/src/linux/linux
fi

if test -e $local_dir/src/linux/linux-2.6; then
    mkdir -p $tree_dir/src/linux
    echo ln -f -s $local_dir/src/linux/linux-2.6 $tree_dir/src/linux/linux-2.6 
    ln -f -s $local_dir/src/linux/linux-2.6 $tree_dir/src/linux/linux-2.6
fi
 

if test ! -e $tree_dir/src/cfe; then
    ln -f -s $local_dir/src/cfe $tree_dir/src/cfe
fi

# finally, copy those single files
makeenv="branding.inc GNUmakefile.inc Makerules Makerules.env" 
for file in $makeenv; do
    cp --parents src/$file $tree_dir/ 
done

if test -e $local_dir/src/router/Makefile; then 
    mv $local_dir/src/router/Makefile $tree_dir/src/router
fi 

#cat open_dir_list.txt
