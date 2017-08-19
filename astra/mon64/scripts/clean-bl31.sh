#!/bin/bash

if [ ! -d scripts ]; then
	echo "All mon64 scripts have to be run from top-level dir."
	return
fi

set -ex

# Remove bl31_astra.bin
rm -rf bin
rm -rf arm-tf/build

set +ex
