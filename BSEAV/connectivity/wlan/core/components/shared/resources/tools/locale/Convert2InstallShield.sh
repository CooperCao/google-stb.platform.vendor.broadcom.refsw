#!/bin/sh

if [ $# -ne 5 ]
then
  echo "Usage: `basename $0` <locale_dir> <target_dir> <locales> <locale dir app name> <locale dir second app name>"
  exit 1
fi

sourceDir=$1
targetDir=$2
locales=$3
app=$4
app2=$5

for localeDir in $locales
do
    if [ ! -d "$sourceDir/$localeDir" ]
    then
	echo "Error: Language subdir \"$sourceDir/$localeDir\" does not exist.";
	exit 2
    fi

    if [ ! -e "$sourceDir/$localeDir/$app" ]
    then
	echo "Error: Language specific file \"$sourceDir/$localeDir/$app\" does not exist.";
	exit 3
    fi

    if [ ! -e "$sourceDir/$localeDir/$app2" ]
    then
	echo "Error: Language specific file \"$sourceDir/$localeDir/$app2\" does not exist.";
	exit 3
    fi

    # when adding new languages to IS you also have to tweak the lists in
    # InstallShield.ipr MediaDefaults.mda StringTablesDefault.shl

		# " 0003 Catalan"			\
		# " 0021 Indonesian			\
		# " 002d Basque"			\
    for isLang in 					\
		"czech 0005 Czech"			\
		"danish 0006 Danish"			\
		"german 0007 German"			\
		"greek 0008 Greek"			\
		"english 0009 English"			\
		"spanish 000a Spanish"			\
		"finnish 000b Finnish"			\
		"hungarian 000e Hungarian"		\
		"italian 0010 Italian"			\
		"japanese 0011 Japanese"		\
		"korean 0012 Korean"			\
		"dutch 0013 Dutch"			\
		"norwegian 0014 Norwegian"		\
		"polish 0015 Polish"			\
		"russian 0019 Russian"			\
		"croatian 001a Croatian"		\
		"slovak 001b Slovak"			\
		"swedish 001d Swedish"			\
		"thai 001e Thai"			\
		"turkish 001f Turkish"			\
		"slovenian 0024 Slovenian"		\
		"chinese_traditional 0404 Chinese (Traditional)"\
		"french 040c French (Standard)"			\
		"brazilian 0416 Portuguese (Brazilian)"		\
		"chinese_simplified 0804 Chinese (Simplified)"	\
		"portugese 0816 Portuguese (Standard)"		\
		"french 0c0c French (Canadian)"
    do
	set -- $isLang

	if [ "$localeDir" != "$1" ]
	then
	    continue
	fi
	last=""
	if [ "$4" != "" ]
	then
	    last=" $4"
	fi

	echo -e "Processing locale: $localeDir\tto InstallShield language $2-$3$last"

	if [ ! -d "$targetDir/InstallShield/String Tables/$2-$3$last" ]
	then
	    mkdir -p "$targetDir/InstallShield/String Tables/$2-$3$last"
	    if [ $? -ne 0 ]
	    then
		exit 4
	    fi
	fi

	echo -e "[General]\nType=STRINGTABLESPECIFIC\nVersion=1.00.000\nLanguage=$2\n\n[Data]" > \
	    "$targetDir/InstallShield/String Tables/$2-$3$last/value.shl"
	if [ $? -ne 0 ]
	then
	    exit 5
	fi
	cat "$sourceDir/$localeDir/$app" "$sourceDir/$localeDir/$app2" | awk '{ print }' > "$targetDir/temp1"
	"$targetDir/uni2utf8.exe" -r "$targetDir/temp1" "$targetDir/temp"
	"$targetDir/uni2mb.exe" "$targetDir/temp" > "$targetDir/temp2"
	awk '{ print }' < "$targetDir/temp2" | \
	    grep "STR_[^\"]*\".*" | \
	    sed -e "s/\"\"\"/\"\"/"g  \
		-e "s/ \"\"/ \"/"g  \
		-e "s/\"\" /\" /"g  \
		-e "s/x000A/n/"g \
		-e "s/^STR_\([[:alnum:]_]*\)[[:space:]]*L\"\(.*\)\"[^\"]*$/\1=\2/" \
		-e "s/%OEM_FULL_PRODUCT_NAME%/<OEM_FULL_PRODUCT_NAME>/"g \
		-e "s/%OEM_SHORT_PRODUCT_NAME%/<OEM_SHORT_PRODUCT_NAME>/"g \
		-e "s/%OEM_FULL_COMPANY_NAME%/<OEM_FULL_COMPANY_NAME>/"g \
		-e "s/%OEM_SHORT_COMPANY_NAME%/<OEM_SHORT_COMPANY_NAME>/"g \
		-e "s/%OEM_UTIL_NAME%/<OEM_UTIL_NAME>/"g >> \
	    "$targetDir/InstallShield/String Tables/$2-$3$last/value.shl"
	if [ $? -ne 0 ]
	then
	    exit 6
	fi

	mkdir -p "$targetDir/InstallShield/Setup Files/Compressed Files/$2-$3$last/Intel 16/"
	mkdir -p "$targetDir/InstallShield/Setup Files/Compressed Files/$2-$3$last/Intel 32/"
	mkdir -p "$targetDir/InstallShield/Setup Files/Compressed Files/$2-$3$last/OS Independent/"
	mkdir -p "$targetDir/InstallShield/Setup Files/Uncompressed Files/$2-$3$last/Intel 16/"
	mkdir -p "$targetDir/InstallShield/Setup Files/Uncompressed Files/$2-$3$last/Intel 32/"
	mkdir -p "$targetDir/InstallShield/Setup Files/Uncompressed Files/$2-$3$last/OS Independent/"
    done
done  

exit 0
