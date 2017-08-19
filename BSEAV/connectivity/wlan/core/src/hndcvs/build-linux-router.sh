#!/bin/bash

# set SUBMIT to bsub if running on LSF
if [ "$SUBMIT" = "" ]; then
    SUBMIT=time
fi

# if -x is not present, perform preparation only
# the actual build will be submitted in a loop the -x enabled

execute_build=N
prepare_only=N
kernel_only=N
no_checkout=N
no_src_update=1

local_dir=`pwd`


# create log directory
LOG_DIR=logs_and_filelists


mkdir -p ${LOG_DIR}

prepare()
{

echo cvs co -p src/hndcvs/build_prop_tree.sh > build_prop_tree.sh 
cvs co -p src/hndcvs/build_prop_tree.sh > build_prop_tree.sh 
chmod a+x build_prop_tree.sh

# do all checkouts 
if [ "$no_checkout" != "Y" ]; then
    for BRAND in $BRANDS; do
	cvs co -p ${TAG:+ -r $TAG} "src/tools/release/${BRAND}.mk" > ${BRAND}.mk
	make NO_SRC_UPDATE=$no_src_update TAG=$TAG -f ${BRAND}.mk checkout
    done
fi

# create generic un-mogrified proprietary tree
date
mkdir -p open_tree
cp -rf src open_tree/
cd open_tree
echo  ../build_prop_tree.sh ../gen_prop_tree

../build_prop_tree.sh ../gen_prop_tree

date
# move and link cfe
mkdir -p ../cfe/src
mv src/cfe ../cfe/src/
# copy proprietary pieces instead of link because they need their specific mogrification
cp --parents -H -rf src/include src/tools/release src/tools/misc src/tools/build components/et src/shared ../cfe/
ln -s ${local_dir}/cfe/src/cfe src/cfe

cd  $local_dir

./build-linux-router.sh -t > ${LOG_DIR}/tar_and_cfe.log 2>&1 &

}

create_tar_and_cfe ()
{

# use cpio to create the tarball
# so we can use find to eliminate links without 
# removing them (there is a race condition with the copy 
# of the open tree which assumes that the links are present)
# using find to feed tar seems to have problems when 
# the number of files gets too high
# do not compress since we will need to concatenate 
# to the proprietary tree later 

date
cd open_tree
echo "find src -type f | cpio -o -H tar > ../open_tree.tar"
find src -type f | cpio -o -H tar > ../open_tree.tar
find src/cfe/ -type f | cpio -o -H tar >  ../cfe.tar
gzip ../cfe.tar
cpio -o -A -H tar -F  ../open_tree.tar ../cfe.tar.gz
date
cd $local_dir


# copy cfe tree to make internal cfe
# TODO use copy_open_tree 

echo cp -rf cfe cfe-internal
cp -rf  cfe cfe-internal

# use generic external router.. 

cp open_tree/src/tools/release/linux-external-router.mk .
date
cd cfe
cp ../linux-external-router.mk ../linux-router.mk .
echo make -f  linux-external-router.mk cfe
make -f  linux-external-router.mk cfe
cd $local_dir
date

# for incp open_tree/src/tools/release/linux-external-router.mk .
#if [ 0 = 1 ]; then
cp open_tree/src/tools/release/linux-internal-router.mk .
cd cfe-internal
cp ../linux-internal-router.mk ../linux-router.mk .
echo make -f  linux-internal-router.mk cfe
make -f  linux-internal-router.mk cfe
cd $local_dir
date
# fi
}

# use cpio to copy the open tree, it's faster
copy_open_tree ()
{
    # make directory structure first to preserve the permissions
    mkdir -p $2
    echo "find $1 -type d | xargs -i mkdir -p $2/{}"
    find $1 -type d | xargs -i mkdir -p "$2/{}"
    
    # create hark links to files
    echo "find $1 -type f | cpio -p  -l $2"
    find $1 -type f | cpio -p  -l $2
    # copy sym links
    echo "find $1 -type l | cpio -p $2"
    find $1 -type l | cpio -p $2
    # exit
}

submit_job ()
{
    job_cmd=(${SUBMIT})

    if [ "${job_cmd[0]}" = "bsub" ]; then 
	if [ "$2" != "" ]; then
	    echo  $SUBMIT  -o $2 $1
	    $SUBMIT -o $2 $1
	    
	else
	    $SUBMIT $1
	fi
    else
	if [ "$2" != "" ]; then
	    $SUBMIT $1 > $2 2>&1 &
	else
	    $SUBMIT $1 &
	fi
    fi
}


execute_single_build ()
{
    BRAND=$1
    TAG=$2
    IFS="-" brand_array=(${BRAND})
    brand_type=${brand_array[1]}
    unset IFS

    echo brand type : $brand_type



    if [ "$prepare_only" != "Y" ]; then
	cp gen_prop_tree/src/tools/release/linux-router.mk .

# copy over the build directory
	if [ -e ${BRAND} ]; then
	    echo remove ${BRAND} 
	    rm -rf ${BRAND}
	fi
	mkdir -p $BRAND
	date
	echo `pwd`
	echo  cp -rf gen_prop_tree ${BRAND}/build_tree
	cp -rf gen_prop_tree ${BRAND}/build_tree
	cp ${BRAND}/build_tree/src/tools/release/${BRAND}.mk ${BRAND}/build_tree/
	
# copy open tree
	date
	echo copy_open_tree open_tree ${BRAND}
	copy_open_tree open_tree ${BRAND}
	
#fix sym links for the copy
	date
	cp build_prop_tree.sh ${BRAND}	
	cd ${BRAND}/open_tree
	echo `pwd`
	echo ../build_prop_tree.sh ../build_tree fix
	../build_prop_tree.sh ../build_tree fix
	cd $local_dir
	echo `pwd`
#build without checkout
	
	echo cp ${BRAND}/build_tree/src/tools/release/linux-router.mk ${BRAND}/build_tree/
	cp ${BRAND}/build_tree/src/tools/release/linux-router.mk ${BRAND}/build_tree/	
	cp ${BRAND}/build_tree/src/tools/release/${BRAND}.mk ${BRAND}/
	date
	# mogrify first to avoid race conditions between router and kernel builds
	echo make -C ${BRAND}/build_tree KERN_IMG_DIR=$local_dir/kernel_images TAG=$TAG -f ${BRAND}.mk mogrify
	make -C ${BRAND}/build_tree KERN_IMG_DIR=$local_dir/kernel_images TAG=$TAG -f ${BRAND}.mk mogrify
	date
	echo cp  open_tree.tar ${BRAND}/build_tree/
	cp  open_tree.tar ${BRAND}/build_tree/
	date
	
        # launch kernel image build. This will return immediately if the image already exists
	echo make -C ${BRAND}/build_tree KERN_IMG_DIR=$local_dir/kernel_images TAG=$TAG BRAND=$BRAND -f ${BRAND}.mk kernel_image 
	make -C ${BRAND}/build_tree KERN_IMG_DIR=$local_dir/kernel_images TAG=$TAG BRAND=$BRAND -f ${BRAND}.mk kernel_image &
        # launch router build. Right before install, this will wait for kernel image to be done.
        # most of the time (if not every time), the kernel will be done.
	if [ "kernel_only" != "Y" ]; then
	    if [ "$brand_type" = "external" ]; then
		date
		echo  make -C ${BRAND}/build_tree KERN_IMG_DIR=$local_dir/kernel_images TAG=$TAG -f ${BRAND}.mk router_build
		#exit
		make -C ${BRAND}/build_tree KERN_IMG_DIR=$local_dir/kernel_images TAG=$TAG  -f ${BRAND}.mk router_build
		date
		make -C ${BRAND}/build_tree -f ${BRAND}.mk build_tarballs
		date
	    else
		date
		echo  make -C ${BRAND}/build_tree KERN_IMG_DIR=$local_dir/kernel_images TAG=$TAG -f ${BRAND}.mk prebuild
		#exit
		make -C ${BRAND}/build_tree KERN_IMG_DIR=$local_dir/kernel_images TAG=$TAG  -f ${BRAND}.mk prebuild
		date
	    fi
	    
	fi
    fi
    echo cd $local_dir
    cd $local_dir
    # remove local .mk
    echo rm ${BRAND}.mk
    rm ${BRAND}.mk
    # indicate that we are done. Should say if we were successful ... ! 
    touch ${BRAND}/BUILD_DONE

}


while [ "$1" != "" ]; do
    
    if [ "$1" = "-b" ]; then
	BRANDS=$2
	shift 2
    elif [ "$1" = "-t" ]; then
	create_tar_and_cfe
	exit 0
    elif [ "$1" = "-r" ]; then
	TAG=$2
	shift 2
    elif [ "$1" = "-j" ]; then
	SUBMIT=$2
	shift 2
    # -p : prepare for builds without updating existng modules
    # optimization for nighlty builds
    elif [ "$1" = "-p" ]; then
	prepare_build=Y
	no_src_update=1
	shift
    # -u : prepare for builds with updating existng modules
    # for developpers to merge changes from cvs 
    elif [ "$1" = "-u" ]; then
	prepare_build=Y
	no_src_update=0
	shift
    elif [ "$1" = "-c" ]; then
	prepare_only=Y
	shift
    elif [ "$1" = "-nc" ]; then
	no_checkout=Y
	shift
    elif [ "$1" = "-k" ]; then
	kernel_only=Y
	shift
    elif [ "$1" = "-e" ]; then
	execute_build=Y
	shift
    fi
done

if [ "$prepare_build" = "Y" ]; then
    prepare
fi

if [ "$execute_build" = "Y" ]; then
	echo build kernel only : $kernel_only
	echo  execute_single_build $BRANDS $TAG
	execute_single_build $BRANDS $TAG
	exit
fi

ARRAY=( $BRANDS )

NBRANDS=${#ARRAY[@]}

echo number of brands : $NBRANDS
if [ $NBRANDS -gt 1 ]; then
    for BRAND in $BRANDS; do
	rm ${BRAND}/BUILD_DONE
	submit_job "./build-linux-router.sh -b $BRAND -r $TAG" ${LOG_DIR}/${BRAND}.log
    done
else
 #    if [ "$prepare_only" != "Y" ]; then
    touch ${BRAND}/BUILD_DONE
    submit_job "./build-linux-router.sh -e -b $BRANDS -r $TAG" ${LOG_DIR}/${BRANDS}.log
#     fi
fi
