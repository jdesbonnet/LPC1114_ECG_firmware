#!/bin/bash
#echo "RESET" > /dev/ttyUSB0
ECG_DEV=/dev/ttyUSB1 
echo "CAPTURE 4000 -r1000" > $ECG_DEV
sleep 10
echo "DOWNLOAD" > $ECG_DEV
head -n 4000 $ECG_DEV | tail -n 3900 > t.t
gnuplot ecg.gp

