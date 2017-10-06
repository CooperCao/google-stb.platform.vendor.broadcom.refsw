#!/bin/sh -x
# Pull in published UTF updates
#
# $Id$


if git config svn-remote.svn.url; then
    # git workspace slaved to svn
    git svn rebase  
elif git config remote.origin.url; then
    # pure git workspace
    git pull
else
    # Fall back to svn
    svn up
fi
