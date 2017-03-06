#!/bin/bash
#
# Program LPC1114 ECG prototype via UART bootloader.
# Requires RESET (JP2 header pin 6) connected to UART DTR
# and ISP_MODE (JP header pin 2) connected to UART RTS.
# Loads file 'firmware.bin'.
# 
# Use example
# ./flash.sh /dev/ttyUSB0
#

UART_DEVICE=$1
LPC21ISP=/opt/arm/bin/lpc21isp
${LPC21ISP} -control -bin firmware.bin  ${UART_DEVICE} 115200  120000

