#!/bin/bash

BASE_URL='https://tianqi.moji.com/liveview/picture/'
#BASE_NUMS='10000000 20000000 30000000 40000000 50000000 60000000 70000000 80000000 90000000'
BASE_NUMS='10000000'

for base_num in $BASE_NUMS
do 
	index=$base_num

	while : 
	do
		URL=${BASE_URL}$((index++))

		echo "Process: "$URL
		wget -O out.html $URL 

		#404
		is_404=`grep "404" out.html | wc -c `
		if [ $is_404  -gt 0 ]
		then
			break
		fi

		image_url=`grep "jpg" out.html | grep simgs | grep -wio "https://[0-9_a-zA-Z\/.\-]*"`
		image_addr=`grep "picture_info_addr" out.html | grep -io ">.*<" | tr -d "><" | cut -c 1-2`
		echo "Image URL:  "$image_url
		echo "Image Addr: "$image_addr

		mkdir -p $image_addr

		cd $image_addr
		wget $image_url
		cd -
	done
done
