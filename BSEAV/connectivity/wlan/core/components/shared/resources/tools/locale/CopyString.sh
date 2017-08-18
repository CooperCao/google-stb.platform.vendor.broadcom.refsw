#!/bin/sh

if [ $# -ne 4 ]
then
  echo "Usage: `basename $0` <locale_dir> <from_file> <to_file> <string>"
  echo "Example> tools/locale/`basename $0` wl/locale TrayAppStrs.txt BCMLogonStrs.txt \"STR_FOO\""
  exit 1
fi

scriptName=`basename $0`
localeDir=$1
fromFileName=$2
toFileName=$3
string=$4

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
    echo $localeDir/$language/$toFileName
    if [ ! -d "$localeDir/$language" ]
    then
	echo "Error: Language subdir \"$localeDir/$language\" does not exist.";
	exit 2
    fi

    if [ ! -e "$localeDir/$language/$fromFileName" ]
    then
	echo "Error: Language specific file \"$localeDir/$language/$fromFileName\" does not exist.";
	exit 3
    fi

    if [ ! -e "$localeDir/$language/$toFileName" ]
    then
	echo "Error: Language specific file \"$localeDir/$language/$toFileName\" does not exist.";
	exit 3
    fi
    tempName=`mktemp $scriptName.XXXXXX`
    cat $localeDir/$language/$toFileName | sed "s/\xEF\xBB\xBF//g" | grep -v '^END\s*$' > $tempName
    if [ $? -ne 0 ]
    then
	exit 4
    fi
    grep -w "$string" $localeDir/$language/$fromFileName >> $tempName
    if [ $? -ne 0 ]
    then
	exit 5
    fi
    echo "END" >> $tempName
    if [ $? -ne 0 ]
    then
	exit 6
    fi
    mv $tempName $localeDir/$language/$toFileName 
    if [ $? -ne 0 ]
    then
	exit 7
    fi
done

exit 0
