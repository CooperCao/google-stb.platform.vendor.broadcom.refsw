#!/bin/sh

if [ $# -ne 4 ]
then
  echo "Renames a string within a locale file"	
  echo "Usage: `basename $0` <locale_dir> <file> <from_string> <to_string>"
  echo "Example> tools/locale/`basename $0` wl/locale TrayAppStrs.txt \"STR_FOO\" \"STR_BAR\""
  echo "This will go thru all TrayappStrs.txt files in all locales and rename the string STR_FOO"
  echo "within each file with a new tag STR_BAR."
  exit 1
fi

scriptName=`basename $0`
localeDir=$1
fileName=$2
fromString=$3
toString=$4

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
    echo "Renaming $fromString to $toString in $localeDir/$language/$fileName"
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
    cat $localeDir/$language/$fileName | sed "s/\xEF\xBB\xBF//g" | sed -e "s/^$fromString\(?*\)/$toString\1/" > $tempName
    if [ $? -ne 0 ]
    then
	echo "Error: Language specific file \"$localeDir/$language/$fileName\" no match.";
	exit 4
    fi
    mv $tempName $localeDir/$language/$fileName 
    if [ $? -ne 0 ]
    then
	echo "Error: Cannot overwrite language specific file \"$localeDir/$language/$fileName\".";
	exit 7
    fi
done

exit 0
