#! /bin/bash
#
# Fix files in svn with Id headers but no supporting keywords.
#
# $Id: d364f12e75d7348a6e210e7cca7dd24750fba2fe $
# $Copyright Broadcom Corporation$
#

[ -d .svn ] || exit 0

svn up
missing=$(grep -l "\$Id\: " -d skip \
    $(comm -23 <(svn list -R|sort) \
    <(svn propget svn:keywords . -R|sed 's/ - .*//'|sort))) && \
    svn propset svn:keywords "Author Date Id Revision" $missing

if [ -n "$missing" ]; then
    read -p "Check in? [n] " yorn
    if [ "$yorn" == "y" ]; then
	svn ci -m "keywords" $missing
    fi
fi

