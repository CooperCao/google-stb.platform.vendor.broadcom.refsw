#!/bin/sh

usage() {
    echo "Usage  : ${0} [-s] [-r]"
    echo " where :"
    echo "    -s : save bp3.bin to flash"
    echo "    -r : restore bp3.bin from flash"
    exit
}

if [ "$#" -lt 1 ]; then
    usage
fi

save() {
  if ls /dev/mmc* 1> /dev/null 2>&1 && for f in /dev/mmcblk?; do sgdisk -p $f | grep $1; done 1> /dev/null 2>&1 ; then
    export bp3=($(for f in /dev/mmcblk?; do sgdisk -p $f | grep $1; done))
    export part=$(ls /dev/mmcblk*p$bp3)
  elif grep $1 /sys/class/mtd/mtd*/name 1> /dev/null 2>&1 ; then
    export bp3=$(dirname `grep $1 /sys/class/mtd/mtd*/name`)
    export part=/dev/$(ls $bp3 | grep mtd)
  else
    echo "Flash partition $1 doesn't exist. Skipping save"
    exit 1
  fi
  echo BP30`md5sum bp3.bin` | dd of=$part bs=1 count=36
  stat -c %8s bp3.bin | dd of=$part bs=1 count=8 seek=36
  dd if=bp3.bin of=$part bs=1 seek=44
}

restore() {
  if ls /dev/mmc* 1> /dev/null 2>&1 && for f in /dev/mmcblk?; do sgdisk -p $f | grep $1; done 1> /dev/null 2>&1 ; then
    export bp3=($(for f in /dev/mmcblk?; do sgdisk -p $f | grep $1; done))
    export part=$(ls /dev/mmcblk*p$bp3)
  elif grep $1 /sys/class/mtd/mtd*/name 1> /dev/null 2>&1 ; then
    export bp3=$(dirname `grep $1 /sys/class/mtd/mtd*/name`)
    export part=/dev/$(ls $bp3 | grep mtd)
   else
    echo "Flash partition $1 doesn't exist. Skipping bp3.bin restore"
    exit 1
  fi

  export ver=$(dd if=$part bs=1 count=4)
  if [ "$ver" == "BP30" ]; then
    export md5=$(dd if=$part bs=1 skip=4 count=32)
    export bp3_size=$(echo `dd if=$part bs=1 skip=36 count=8`)
    export cal=($(dd if=$part bs=1 skip=44 count=$bp3_size | md5sum))
    if [ "$md5" == "$cal" ]; then
      dd if=$part of=bp3.bin bs=1 skip=44 count=$bp3_size
      return 0
    else
      echo "MD5 of bp3.bin in flash $1 doesn't match"
      return 2
    fi
  else
    # old format, no version, 9 bytes size
    export bp3_size=$(echo `dd if=$part bs=9 count=1`)
    dd if=$part of=bp3.bin bs=1 skip=9 count=$bp3_size
    return 0
  fi
}

while [ "$1" != "" ]; do
  case $1 in
    -s ) shift
      if [ ! -e bp3.bin ]; then
        echo "bp3.bin not found. save failed"
        exit 1
      fi
      save bp30
      save bp31
      ;;

    -r ) shift
      restore bp30
      if [ "$?" != "0" ]; then
        restore bp31
      fi
      exit $?
      ;;

    * ) shift
        usage
  esac
done
