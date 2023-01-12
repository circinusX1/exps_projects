#!/bin/bash

pkill screen
a="y"
ls /dev/ttyUSB*
while [[ $a != "n" ]];do
    echo "enter serial port # 0,1,2,3"
    read SP
    export SERIAL_PORT=/dev/ttyUSB${SP}
    make upload
    echo "wanna flasgh more (y/n)"
    read y
done
