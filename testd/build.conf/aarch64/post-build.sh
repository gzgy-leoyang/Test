echo " >>进入后处理..."
#scp out/aarch64/bin/yqsrv_testd root@192.168.10.2:/home/root
adb push out/aarch64/bin/yqsrv_testd /home/root/app
echo " >>后处理完成"
