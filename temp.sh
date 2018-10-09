#!/bin/bash

while :
do      
    echo "`date`: `/opt/vc/bin/vcgencmd measure_temp`"
    sleep 5
done
