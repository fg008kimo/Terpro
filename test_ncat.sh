#! /bin/bash
#paste the code below on Linux interface directly to monitor the specific ip and port connect status
#ip and port can change to any

while true; 
do
    if nc -zv 192.168.160.12 15097 2>/dev/null; then 
        echo "yes"
    else
        echo "no"
    fi
    sleep 5
done