#!/bin/sh -e
#
# Generate a private downloadable CLM object
#
# usage: cmltrx output.trx <compiler args>

output=$1; shift

ABI=/projects/hnd/tools/linux/hndtools-armeabi-2011.09/bin/arm-none-eabi
gsrc=/projects/hnd/software/gallery/src
swc=$gsrc/wl/clm
trx=/projects/hnd/tools/linux/bin/trx

tmp=$(mktemp -d)

# clean up
trap "rm -rf $tmp" 0

python $swc/bin/ClmCompiler.py "$@" \
    $swc/private/wlc_clm_data.xml $tmp/wlc_clm_data.c

$ABI-gcc -c -I $swc/include -I $gsrc/shared/bcmwifi/include/ \
    -o $tmp/wlc_clm_data.o $tmp/wlc_clm_data.c

$ABI-ld -T $gsrc/shared/clm_inc.lds -static -o $tmp/wlc_clm_data.elf \
    $tmp/wlc_clm_data.o

$ABI-objcopy -O binary $tmp/wlc_clm_data.elf $output
