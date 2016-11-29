#!/bin/sh  
pid=`ps aux | grep uni | grep -v grep`  
if [  -z "$pid" ];then  
    nohup python /root/nrf24pihub/unicenter.py &  
else  
    echo "it's running " 
fi 
