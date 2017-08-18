#!/bin/sh

if [ $# -eq 4 ]
then
    sourceDir=$1
    app=$2
    subDir=$3
    langSublang=$4
    echo -e "[strings.$langSublang]"
elif [ $# -eq 3 ]
then
    sourceDir=$1
    app=$2
    subDir=$3
    echo -e "[strings]"
else
  echo "Usage: `basename $0` <sourceDir> <app> <locale> <lang-sublang>"
  exit 5
fi
if [ $? -ne 0 ]
then
    exit 5
fi
cat "$sourceDir/$subDir/$app" | \
    grep "STR_[^\"]*\".*" | \
    sed -e "s/^STR_\([[:alnum:]_]*\)[[:space:]]*L\"\(.*\)\"[^\"]*$/\1=\"\2\"/"
if [ $? -ne 0 ]
then
    exit 6
fi
echo

exit 0
