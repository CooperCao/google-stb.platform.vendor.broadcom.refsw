#!/bin/bash
cp -rf gentree $1
#cd $1
cp $1/src/tools/release/$1.mk $1
cp $1/src/tools/release/linux-router.mk $1
make -C $1 TAG=MILLAU_BRANCH_5_70 SRC_CHECKOUT_DISABLED=1 -f $1.mk
