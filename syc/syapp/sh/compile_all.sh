#! /bin/bash

batchNum=1

if [ $1 ];then
	batchNum=$1
fi

platformName=""
if [ $2 ];then
	platformName=$2
fi

macAddr=`ip addr | grep link/ether | head -n 1 | cut -d " " -f6`
msg="unknow_$batchNum"
if [ "$macAddr" = "0a:7f:39:87:75:a8" ]; then
	if [ "$batchNum" = "1" ]; then
		msg="vpower88"
	fi
elif [ "$macAddr" = "06:8c:e2:41:d9:b0" ]; then
	if [ "$batchNum" = "1" ]; then
		msg="stargame88"
	fi
elif [ "$macAddr" = "06:9c:0b:74:68:68" ]; then
	if [ "$batchNum" = "1" ]; then
		msg="vnstar88"
	fi
elif [ "$macAddr" = "0a:f5:85:03:e2:d2" ]; then
	if [ "$batchNum" = "1" ]; then
		msg="vpower77"
	fi
elif [ "$macAddr" = "06:71:91:5b:51:bc" ]; then
	if [ "$batchNum" = "1" ]; then
		msg="thstar88"
	elif [ "$batchNum" = "2" ]; then
		msg="indostar88"
	fi
elif [ "$macAddr" = "06:76:f4:2c:01:d2" ]; then
	if [ "$batchNum" = "1" ]; then
		msg="goldendragon77"
	elif [ "$batchNum" = "2" ]; then
		msg="game777"
	elif [ "$batchNum" = "3" ]; then
		msg="auvpower"
	fi
elif [ "$macAddr" = "06:0b:ad:c5:33:4c" ]; then
	if [ "$batchNum" = "1" ]; then
		msg="goldendragon99"
	fi
elif [ "$macAddr" = "06:ff:59:98:f1:70" ]; then
	if [ "$batchNum" = "1" ]; then
		msg="winclub"
	fi
elif [ "$macAddr" = "06:f4:bd:76:f0:6a" ]; then
	if [ "$batchNum" = "1" ]; then
		msg="naga"
	elif [ "$batchNum" = "2" ]; then
		msg="gamecenter"
	elif [ "$batchNum" = "3" ]; then
		msg="vpower66"
	fi
elif [ "$macAddr" = "02:09:e7:f9:df:46" ]; then
	if [ "$batchNum" = "1" ]; then
		msg="vpower777"
	fi
elif [ "$macAddr" = "06:ea:de:b3:a1:ce" ]; then
	if [ "$batchNum" = "1" ]; then
		msg="xgame99"
	fi
elif [ "$macAddr" = "02:4f:1d:28:53:b4" ]; then
	if [ "$batchNum" = "1" ]; then
		msg="ultrapower"
	fi
elif [ "$macAddr" = "02:d1:0b:fa:51:42" ]; then
	if [ "$batchNum" = "1" ]; then
		msg="pcg"
	elif [ "$batchNum" = "2" ]; then
		msg="kocean99"
	elif [ "$batchNum" = "3" ]; then
		msg="flaminghot99"
	elif [ "$batchNum" = "4" ]; then
		msg="goldenmonster99"
	fi
elif [ "$macAddr" = "02:ec:bd:75:4e:0a" ]; then
	if [ "$batchNum" = "1" ]; then
		msg="goldenfortune"
	fi
elif [ "$macAddr" = "02:60:7f:46:2f:64" ]; then
	if [ "$batchNum" = "1" ]; then
		msg="ultramonster"
	fi
elif [ "$macAddr" = "02:cb:51:61:13:f4" ]; then
	if [ "$batchNum" = "1" ]; then
		msg="goldendragon88"
	fi
elif [ "$macAddr" = "0e:36:fc:55:26:88" ]; then
	if [ "$batchNum" = "1" ]; then
		msg="goldencity"
	fi
elif [ "$macAddr" = "02:4e:83:17:00:b8" ]; then
	if [ "$batchNum" = "1" ]; then
		msg="acebook"
	fi
elif [ "$macAddr" = "00:16:3e:01:2e:a5" ]; then
	msg="demonMan_$batchNum"
elif [ "$macAddr" = "00:16:3e:02:63:75" ]; then
	msg="mmclub2_$batchNum"
elif [ "$macAddr" = "00:50:56:3d:75:92" ]; then
    msg="test_$batchNum"
elif [ "$macAddr" = "00:50:56:21:a4:88" ]; then
	msg="test2_$batchNum"
fi

echo "Update $msg:$platformName? Y/N"
read isYes
if [ "$isYes" != "Y" ]; then
	echo "No update"
	exit 0
fi
echo "Updating $msg..."
sleep 1

if [[ $platformName == "" ]]; then
    platformName=$msg
fi

#echo "########################################"
#echo "##               platform             ##"
#echo "########################################"
#cd /home/syc/syapp/platform
#make clean
#make -j$(nproc) PLATFORM=$platformName
#make install SUBPATH=$batchNum
#echo "########################################"
#echo "##               rng                  ##"
#echo "########################################"
#cd /home/syc/syapp/randomNumberGenerator
#make clean
#make -j$(nproc) PLATFORM=$platformName
#make install SUBPATH=$batchNum
#echo "########################################"
#echo "##               fish                 ##"
#echo "########################################"
#cd /home/syc/syapp/algfish
#make clean
#make -j$(nproc) PLATFORM=$platformName 
#make install SUBPATH=$batchNum
#echo "########################################"
#echo "##               chargepoint          ##"
#echo "########################################"
#cd /home/syc/syapp/algchargepoint
#make clean
#make -j$(nproc) PLATFORM=$platformName 
#make install SUBPATH=$batchNum

#echo "########################################"
#echo "##               3000195              ##"
#echo "########################################"
#cd /home/syc/syapp/slot/3000195
#make clean
#make -j$(nproc) PLATFORM=$platformName 
#make install SUBPATH=$batchNum

#echo "########################################"
#echo "##               3000201              ##"
#echo "########################################"
#cd /home/syc/syapp/slot/3000201
#make clean
#make -j$(nproc) PLATFORM=$platformName
#make install SUBPATH=$batchNum

#echo "########################################"
#echo "##               3000202              ##"
#echo "########################################"
#cd /home/syc/syapp/slot/3000202
#make clean
#make -j$(nproc) PLATFORM=$platformName
#make install SUBPATH=$batchNum

#echo "########################################"
#echo "##               3000199              ##"
#echo "########################################"
#cd /home/syc/syapp/slot/3000199
#make clean
#make -j$(nproc) PLATFORM=$platformName
#make install SUBPATH=$batchNum

#echo "########################################"
#echo "##               3000212              ##"
#echo "########################################"
#cd /home/syc/syapp/slot/3000212
#make clean
#make -j$(nproc) PLATFORM=$platformName 
#make install SUBPATH=$batchNum

#echo "########################################"
#echo "##               3000213              ##"
#echo "########################################"
#cd /home/syc/syapp/slot/3000213
#make clean
#make -j$(nproc) PLATFORM=$platformName 
#make install SUBPATH=$batchNum

#echo "########################################"
#echo "##               3000503              ##"
#echo "########################################"
#cd /home/syc/syapp/slot/3000503
#make clean
#make -j$(nproc) PLATFORM=$platformName 
#make install SUBPATH=$batchNum

echo "########################################"
echo "##               3000504              ##"
echo "########################################"
cd /home/syc/syapp/slot/3000504
make clean
make -j$(nproc) PLATFORM=$platformName 
make install SUBPATH=$batchNum
