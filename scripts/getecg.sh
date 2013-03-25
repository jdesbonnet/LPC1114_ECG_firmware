#!/bin/bash
#echo "RESET" > /dev/ttyUSB0
ECG_DEV=/dev/ttyUSB1 
echo "E" > $ECG_DEV
sleep 5
echo "P" > $ECG_DEV
head -n 2000 $ECG_DEV | tail -n 1990 > t.t
gnuplot ecg.gp

