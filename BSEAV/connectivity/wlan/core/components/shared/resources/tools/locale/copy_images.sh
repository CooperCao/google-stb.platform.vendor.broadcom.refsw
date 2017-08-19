img_dir=/home/jyothik/WLAN_Images/05192005/Dell/XP

for dir in *
do
	if [ -d $img_dir/$dir ]
	then
		if [ -d $dir/images ]
		then
			if [ ! -d $dir/images/dell ]
			then
				mkdir $dir/images/dell
			fi
			cp $img_dir/$dir/*.gif $dir/images/dell
		fi
	fi
done
