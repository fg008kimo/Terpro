#! /bin/bash
#  author hyh 

batchNums=(1)

if [ $1 ]; then
	batchNums=($1)
fi

getPid()
{
    port=$1
    result=$(ps aux | grep "\-p $port" | grep -v grep | awk '{print $2}')
    echo $result
}

killProcess()
{
    pid=$(getPid $1)
    $(kill -9 $pid)
}

startProcess()
{
   $1 -p $2
   echo "$1 -p $2"
}

restartProcess()
{
	killProcess $2
	sleep 0.1
	startProcess $1 $2
}

setCurBatchNum()
{
	curBatchNum=${1}
}

setRootPath()
{
	rootPath="/home/syc/bin/${1}/"
}

echo "########################################"
echo "##               begin reStart        ##"
echo "########################################"

# /home/syc/syapp_whole_sh/guard/guardProcess_end.sh

restartOneBatchProcess()
{
	setCurBatchNum $1
	setRootPath $curBatchNum

#	restartProcess ${rootPath}platform_server ${curBatchNum}0000
#	restartProcess ${rootPath}random_number_generator ${curBatchNum}0001
#	restartProcess ${rootPath}mammon_3000195_server ${curBatchNum}5079 #雷霆之怒
#   restartProcess ${rootPath}mammon_3000201_server ${curBatchNum}5087 #西部野牛
#   restartProcess ${rootPath}mammon_3000202_server ${curBatchNum}5088 #荒野牛仔
#	restartProcess ${rootPath}mammon_3000199_server ${curBatchNum}5086 #超級甜
#	restartProcess ${rootPath}mammon_3000212_server ${curBatchNum}5091 #幸運兔
#	restartProcess ${rootPath}mammon_3000213_server ${curBatchNum}5148 #經典寶石B
#	restartProcess ${rootPath}mammon_3000503_server ${curBatchNum}5097 #經典寶石A
	restartProcess ${rootPath}mammon_3000504_server ${curBatchNum}5123 #荒野西部40線


	
	#【注意】添加新游戏时要填写若干gamealgo.config，以指定服务端要连的算法端口（详情参考gamealgo.xlsx）

}

MakeSureRestart()
{
	batchNum=$1
	macAddr=`ip addr | grep link/ether | head -n 1 | cut -d " " -f6`
	msg="???:$batchNum"
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
		msg="demonMan:$batchNum"
	elif [ "$macAddr" = "00:16:3e:02:63:75" ]; then
		if [ "$batchNum" = "1" ]; then
			msg="mmclub2 normal"
		elif [ "$batchNum" = "2" ]; then
			msg="mmclub2 big"
		else
			msg="mmclub2:$batchNum"
		fi
	elif [ "$macAddr" = "00:50:56:3d:75:92" ]; then
		if [ "$batchNum" = "1" ]; then
			msg="1.8:$batchNum develop"
		elif [ "$batchNum" = "2" ]; then
			msg="1.8:$batchNum online"
		elif [ "$batchNum" = "3" ]; then
			msg="1.8:$batchNum update"
		elif [ "$batchNum" = "4" ]; then
			msg="1.8:$batchNum big"
		else
			msg="1.8:$batchNum"
		fi
	elif [ "$macAddr" = "00:50:56:21:a4:88" ]; then
		msg="1.9:$batchNum"
	fi

	echo "Update $msg? Y/N"
	read isYes
	if [ "$isYes" != "Y" ]; then
		echo "No update"
		exit 0
	fi
	echo "Updating $msg..."
	sleep 1
}

index=0
while [ "$index" -lt ${#batchNums[@]} ]
do
	MakeSureRestart ${batchNums[$index]}
	restartOneBatchProcess ${batchNums[$index]}
	index=$(($index + 1))
done

#exit 0

# sleep 5
# /home/syc/syapp_whole_sh/guard/guardProcess_start.sh

