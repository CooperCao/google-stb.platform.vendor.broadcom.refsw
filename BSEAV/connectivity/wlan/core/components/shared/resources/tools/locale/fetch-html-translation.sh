trans_dir=/h/ogre/projects/wlan/wlan-docs/SDL/deliverables_103105
#/h/ogre/projects/wlan/wlan-docs/SDL/deliverables_050905
#	trans_dir=/tmp/

for dir in brazilian \
	chinese_simplified \
	chinese_traditional \
	danish \
	dutch \
	finnish \
	french \
	german \
	greek \
	hungarian \
	italian \
	japanese \
	korean \
	norwegian \
	polish \
	portugese \
	russian \
	spanish \
	swedish \
	thai \
	turkish
do
	if [ -f $trans_dir/$dir/untranslated-blocks.htm ]
	then
		echo $dir
		cat $trans_dir/$dir/translated-*.htm | perl ../../tools/htmlpp/recover_htmlpp.pl > /tmp/b
		cat $trans_dir/$dir/untranslated-*.htm | perl ../../tools/htmlpp/recover_htmlpp.pl >> /tmp/b
		cat $dir/*.htm >> /tmp/b
		cd english
		for file in *.htm
		do
			echo $file
			cd ../$dir
			if [ -f $file.unused ]
			then
				perl ../../../../src/tools/locale/merge_html_translation.pl include $file /tmp/b >| /tmp/a
				cp /tmp/a $file
			else
				perl ../../../../src/tools/locale/merge_html_translation.pl exclude ../english/$file /tmp/b >| /tmp/a
				cp /tmp/a $file
			fi
		done
		cd ..
	fi
done
