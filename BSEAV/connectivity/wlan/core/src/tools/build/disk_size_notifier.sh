#!/tools/bin/bash

# THIS SCRIPT WILL SEND MAIL TO ALL USERS WHO EXCEEDS NETAPP QUOTA"

# Below Function will find out name of those users who use more space than allowed limit
# It will Create temp file inside /tmp folder and delete it once the program is over.
function diskSpaceCheck ()
{
    project_Disk=$1
    space_limit=$2
    DATE=`date +%F`
    userID=
    userEMAIL=mbiswal@broadcom.com

    projrpt $project_Disk > /tmp/"$project_Disk"_size_check_"$DATE".txt
    if [ `echo $?` -ne 0 ]; then
        echo "The project name \"$project_Disk\" is not correct. Please provide the correct project name"
        exit 1;
    fi
    tmpfile=/tmp/"$project_Disk"_size_check_"$DATE".txt

    for space_used in `cat $tmpfile | grep user | awk '{print $3}'`
    do
        if [ $space_used -ge $space_limit ] ; then
            userID="`cat $tmpfile | grep $space_used | awk '{print $2}'` $userID"
            userEMAIL="`cat $tmpfile | grep $space_used | awk '{print $2}'`@broadcom.com, $userEMAIL"
        fi
    done
        #echo $userID
        #echo $userEMAIL

        Mail_Sub="DISK-$project_Disk: YOU ARE USING MORE SPACE THAN ALLOCATED SPACE LIMIT"
        echo "Hello \"$userID\"" > $tmpfile.mail
        echo "" >> $tmpfile.mail
        echo "You are using more space than space limit allocate for you. Please check below information and free up space" >> $tmpfile.mail
        echo "" >> $tmpfile.mail
        echo "" >> $tmpfile.mail
        cat $tmpfile >> $tmpfile.mail
        #mail -s "$Mail_Sub" "$userEMAIL" < $tmpfile.mail
        mail -s "$Mail_Sub" mbiswal@broadcom.com < $tmpfile.mail
        rm -rf $tmpfile $tmpfile.mail
}


# Below Function will Call diskSpaceCheck function with disk name as 1st argument and space limit as second argument in below function
# Write down all the project name and netApp quota in the below function

All_Disk_Check()
{
    diskSpaceCheck hnd_software_ext2 10000000
    diskSpaceCheck hnd_software_ext1 10000000
}

if [ "$1" = "all" ]; then
    All_Disk_Check;
elif [ $# -eq 2 ]; then
    diskSpaceCheck $1 $2
else
    echo "This program find out name of users exceeds NetApp Quota and send mail only to them"
    echo "Usage:"
    echo ""
    echo "1. for generating data and sending mail for a single project"
    echo "    $0 <project name> <NetApp Quota>"
    echo "2. for generating data and sending mail for all project"
    echo "    $0 all ";
fi
