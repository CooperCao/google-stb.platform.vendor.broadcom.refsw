#!/bin/sh

if [ $# -ne 3 ]
then
  echo "Usage: `basename $0` <locale_dir> <file_name> <string>"
  echo "Example> tools/locale/`basename $0` wl/locale bcm.txt \"STR_FOO L\\\"Foo\\\"\""
  exit 1
fi

scriptName=`basename $0`
localeDir=$1
fileName=$2
string=$3

locales="arabic
	brazilian
	bulgarian
	chinese_simplified 
	chinese_traditional 
	croatian 
	czech 
	danish 
	dutch 
	english 
	estonian 
	finnish 
	french 
	german 
	greek 
	hebrew 
	hungarian 
	italian 
	japanese 
	korean 
	latvian 
	lithuanian 
	norwegian 
	polish 
	portugese 
	romanian 
	russian 
	slovak 
	slovenian 
	spanish 
	swedish 
	thai 
	turkish"

for language in $locales
do
    echo $localeDir/$language/$fileName
    if [ ! -d "$localeDir/$language" ]
    then
	echo "Error: Language subdir \"$localeDir/$language\" does not exist.";
	exit 2
    fi

    if [ ! -e "$localeDir/$language/$fileName" ]
    then
	echo "Error: Language specific file \"$localeDir/$language/$fileName\" does not exist.";
	exit 3
    fi
    tempName=`mktemp $scriptName.XXXXXX`
    cat $localeDir/$language/$fileName | sed "s/\xEF\xBB\xBF//g" | sed "1,/^END\s*$/s/^END\s*$/$string\nEND/" > $tempName
    if [ $? -ne 0 ]
    then
	exit 4
    fi
    mv $tempName $localeDir/$language/$fileName
    if [ $? -ne 0 ]
    then
	exit 5
    fi
done

exit 0
