#!/bin/bash 
echo "                     归档脚本展开"
sleep  0.3
read -p "                     是否进行归档 y/n?:" key
if [ "$key" != "y" ];then
    exit 1
else
    rm -f ./zip/*
    for targetdir in $(ls ./|grep "^[[:digit:]]");do
        sleep 0.3
        tar -zcf ./zip/$targetdir.tar.gz  ./$targetdir
        echo "$targetdir.tar.gz 压缩结果:  $?"
    done
fi

