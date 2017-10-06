#!/tools/bin/bash

# THIS SCRIPT IS USED TO VERIFY THE INTEGRITY OF ARCHIVED FOLDER
# IT ALSO VERIFY ANY DUPLICATE RELEASE BUILD LOCATIONS
# AND BROKEN LINK INSIDE ARCHIVE FOLDER

ARCHIVED_DISK=("hnd_sw_archive_ext2 hnd_sw_archive_ro_ext8 hnd_sw_archive_ro_ext7 hnd_sw_archive");
ARCHIVED_DISK=("$ARCHIVED_DISK hnd_sw_archive_ro_ext3 hnd_sw_archive_ro_ext5 hnd_sw_archive_ro_ext6");
ARCHIVED_DISK=("$ARCHIVED_DISK hnd_sw_archive_ro_ext2 hnd_sw_archive_ro_ext");
REL_BUILD=
NOT_LINKED=
DUPLICATE_LINKED=
ARCHIVE_BUILD_PATH=
BROKEN_LINK=
NOT_LINKED_IN_ARCHIVE=
UNPROPER_LINK=

if [ "$1" = windows ]; then
    OS=build_window
    ARCHIVED_LINK_DIR=/projects/hnd/swbuild/build_window/ARCHIVED
elif [ "$1" = linux ]; then
    OS=build_linux
    ARCHIVED_LINK_DIR=/projects/hnd/swbuild/build_linux/ARCHIVED
else
    echo "Please Pass command line argument properly as below example"
    echo "\"sh $0 windows\" or \"sh $0 linux\" "
    exit 1;
fi
echo "----------------------------------------------------------"
echo "ARCHIVED FOLDER=$ARCHIVED_LINK_DIR"
echo "----------------------------------------------------------"

# Searching for archived dir for finding out duplicate builds and
# builds those are not having a link inside ARCHIVED dir

for DIR in $ARCHIVED_DISK
do
    ARCHIVED_PATH=`find /projects/$DIR -maxdepth 2 -type d -name $OS`
    ARCHIVE_BUILD_PATH="$ARCHIVE_BUILD_PATH $ARCHIVED_PATH";

    if [ $ARCHIVED_PATH ]; then
        for rel in `find $ARCHIVED_PATH -maxdepth 1 -type d -name *_REL_*`
        do
            rel_build=`echo $rel | awk -F / '{print $NF}'`
            if [ ! -h $ARCHIVED_LINK_DIR/$rel_build ]; then
                NOT_LINKED="$NOT_LINKED $rel";

            else
                ARCHIVED_LINK=`ls -l $ARCHIVED_LINK_DIR/$rel_build | awk {'print $11'}`
                if [ $ARCHIVED_LINK != $rel ]; then
                    DUPLICATE_LINKED="$DUPLICATE_LINKED $ARCHIVED_LINK $rel"
                fi


            fi
        done
   fi
done

# Searching ARCHIVED folder for any broken link

for release_link in `find $ARCHIVED_LINK_DIR -maxdepth 1 -type l`
do
    LINKED_DIR=`ls -l $release_link | awk {'print $11'}`
    if [ ! -d $LINKED_DIR   ];then
        BROKEN_LINK="$BROKEN_LINK $LINKED_DIR"
    fi
done

echo "$1 RELEASE BUILDS ARE SAVED IN BELOW DISK"
echo "---------------------------------------------------------"
for release_path in $ARCHIVE_BUILD_PATH
do
    echo $release_path
done


if [ ! -z "$NOT_LINKED" ];then
    echo "---------------------------------------------------------"
    echo "FOLLOWING RELEASES ARE NOT LINKED INSIDE ARCHIVED FOLDER"
    echo "----------------------------------------------------------"

    for no_link in $NOT_LINKED
    do
        echo $no_link
    done
fi

if [ ! -z "$DUPLICATE_LINKED" ];then
    echo "------------------------------------------------------------------------------"
    echo "FOLLOWING RELEASES ARE AVAILABLE AT MULTIPLE LOCATIONS, VERIFY BEFORE DELETE"
    echo "------------------------------------------------------------------------------"

    for duplicate_link in $DUPLICATE_LINKED
    do
        echo $duplicate_link
    done
fi


if [ ! -z "$BROKEN_LINK" ];then
    echo "---------------------------------------------------------"
    echo "FOLLOWING ARCHIVED LINKS ARE BROKEN"
    echo "---------------------------------------------------------"

    for broken_link in $BROKEN_LINK
    do
        echo $broken_link
    done
fi
