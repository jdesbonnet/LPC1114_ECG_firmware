/**************************************************************************/
/*! 
    @file     main.c
    @author   K. Townsend (microBuilder.eu)

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2010, microBuilder SARL
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************************/
#include "projectconfig.h"
#include "sysinit.h"

#include "core/gpio/gpio.h"
#include "core/systick/systick.h"
#include "core/uart/uart.h"
#include "core/pmu/pmu.h"
#include "core/cpu/cpu.h"
#include "lpc111x.h"

#ifdef CFG_INTERFACE
  #include "core/cmd/cmd.h"
#endif

#define INPUT (0)
#define OUTPUT (1)

int main(void) {
	int i;
	systemInit();
	uartInit(115200);

	gpioSetValue (1, 8, 0);
	gpioSetValue (1, 8, 1);
	gpioSetValue (1, 8, 0);
	gpioSetValue (1, 8, 1);
	gpioSetValue (1, 8, 0);

/*
	// Making this input draws 182µA, output just 61µA
	gpioSetDir (1,8,INPUT);
	for (i = 1; i < 10; i++) {
		// 386µA if set to input(0), 63µA if output (1)
		gpioSetDir (0,i,0); // 1 = output
		gpioSetValue (0,i,1); // this seems to make no difference
	}
	gpioDisableInternalResistors();
*/


  	while (1) {

		// http://knowledgebase.nxp.com/showthread.php?t=187
		//Disable reset pin functionality by making port 0 pin 0 a GPIO pin:
		//LPC_IOCON->RESET_PIO0_0 |= 0x01
		// IOCONFIG base: 0x4004 4000
		// IOCON_nRESET_PIO0_0   : 0x4004 400C

		// The following conditioning of RESET does appear to yield a saveing of 8µA.
		// However it will disable RESET and a power cycle is needed to re-enter bootloader.

/*
		IOCON_nRESET_PIO0_0  = 0x01; // selects 0.0 as GPIO, 0x00 as RESET
		gpioSetDir(0, 0, 1); // 0.0 as output
		gpioSetValue (0, 0, 1); // 0.0 HIGH
*/




		// Want all pins as GPIO, pullups off, output at 0V.
		IOCON_PIO0_1 = IOCON_PIO0_1_FUNC_GPIO; // pin 24 (also bootloader entry)
		IOCON_PIO0_2 = IOCON_PIO0_2_FUNC_GPIO; // pin 25
		IOCON_PIO0_3 = IOCON_PIO0_3_FUNC_GPIO; // pin 26
		IOCON_PIO0_4 = IOCON_PIO0_4_FUNC_GPIO; // pin 27
		IOCON_PIO0_5 = IOCON_PIO0_5_FUNC_GPIO; // pin 
		IOCON_PIO0_6 = IOCON_PIO0_6_FUNC_GPIO; // pin 
		IOCON_PIO0_7 = IOCON_PIO0_7_FUNC_GPIO; // pin 28
		IOCON_PIO0_8 = IOCON_PIO0_8_FUNC_GPIO; // pin 1
		IOCON_PIO0_9 = IOCON_PIO0_9_FUNC_GPIO; // pin 2
		IOCON_JTAG_TCK_PIO0_10 = IOCON_JTAG_TCK_PIO0_10_FUNC_GPIO;
		IOCON_JTAG_TDI_PIO0_11 = IOCON_JTAG_TDI_PIO0_11_FUNC_GPIO; 
		IOCON_JTAG_TMS_PIO1_0 = IOCON_JTAG_TMS_PIO1_0_FUNC_GPIO;	
		IOCON_JTAG_TDO_PIO1_1 = IOCON_JTAG_TDO_PIO1_1_FUNC_GPIO;
		//IOCON_JTAG_nTRST_PIO1_2 = IOCON_JTAG_nTRST_PIO1_2_FUNC_GPIO; // causes extra 150µA drain?!
		IOCON_SWDIO_PIO1_3 = IOCON_SWDIO_PIO1_3_FUNC_GPIO;
		IOCON_PIO1_4 = IOCON_PIO1_4_FUNC_GPIO;
		IOCON_PIO1_5 = IOCON_PIO1_5_FUNC_GPIO;
		IOCON_PIO1_6 = IOCON_PIO1_6_FUNC_GPIO; // THIS MADE ALL THE DIFFERENCE! 4µA

		for (i = 0; i < 10; i++) {
			// 386µA if set to input(0), 63µA if output (1)
			gpioSetDir (0,i,1); // 1 = output
			gpioSetValue (0,i,1); // this seems to make no difference
		}
		for (i = 0; i < 7; i++) {
			gpioSetDir (1,i,1); // 1 = output
			gpioSetValue (1,i,1); // this seems to make no difference
		}
		//gpioSetDir (9,i,1); // 1 = output
		//gpioSetValue (9,i,1); // this seems to make no difference

 		pmuDeepSleep(1);
		//pmuPowerDown();


		// So sometimes this loop executes at WDT speed, and sometimes at
		// full internal osc speed. Why?
		// Observed that the wake IRQ was serviced *after* the LED pin toggle.

/*		
		for (i = 0; i <4; i++) {
			gpioSetValue (1, 8, CFG_LED_ON);
			gpioSetValue (1, 8, CFG_LED_OFF); 
		}
*/
		// Poll for CLI input if CFG_INTERFACE is enabled in projectconfig.h
		#ifdef CFG_INTERFACE 
			//cmdPoll(); 
		#endif
	}

 	 return 0;
}

