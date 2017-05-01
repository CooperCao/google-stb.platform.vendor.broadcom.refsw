#!/bin/bash

if [ ! -d scripts ]; then
	echo "All mon64 scripts have to be run from top-level dir."
	return
fi

set -ex

# Remove arm-tf
rm -rf arm-tf

# Remove bl31_astra.bin
rm -f bl31_astra.bin

set +ex
