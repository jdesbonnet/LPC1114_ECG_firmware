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

int main(void) {
	int i;
	systemInit();
	uartInit(115200);

	pmuInit();

	gpioSetValue (1, 8, 0); 


  	while (1) {

		//pmuInit();


		// http://knowledgebase.nxp.com/showthread.php?t=187
		//Disable reset pin functionality by making port 0 pin 0 a GPIO pin:
		//LPC_IOCON->RESET_PIO0_0 |= 0x01

 		pmuDeepSleep(10);

		// So sometimes this loop executes at WDT speed, and sometimes at
		// full internal osc speed. Why?

		//pmuRestoreHW();

/*
	// Switch back to internal osc
	SCB_MAINCLKSEL = SCB_MAINCLKSEL_SOURCE_INTERNALOSC;
	SCB_MAINCLKUEN = SCB_MAINCLKUEN_UPDATE;       // Update clock source
	SCB_MAINCLKUEN = SCB_MAINCLKUEN_DISABLE;      // Toggle update register once
	SCB_MAINCLKUEN = SCB_MAINCLKUEN_UPDATE;
	// Wait until the clock is updated
	while (!(SCB_MAINCLKUEN & SCB_MAINCLKUEN_UPDATE));
*/

		
		for (i = 0; i <4; i++) {
			gpioSetValue (1, 8, 1);
			gpioSetValue (1, 8, 0); 
		}

		// Poll for CLI input if CFG_INTERFACE is enabled in projectconfig.h
		#ifdef CFG_INTERFACE 
			//cmdPoll(); 
		#endif
	}

 	 return 0;
}

/**
 * Entering Deep Sleep Mode is covered in UM10398 §3.9.3.2. Copied verbatim:
 *
 * The following steps must be performed to enter Deep-sleep mode:
 * 1. The DPDEN bit in the PCON [Power Control Register] register must be set to zero (Table 49).
 * 2. Select the power configuration in Deep-sleep mode in the PDSLEEPCFG (Table 41)
 * register.
 *    a. If a timer-controlled wake-up is needed, ensure that the watchdog oscillator is
 *       powered in the PDRUNCFG register and switch the clock source to WD oscillator
 *       in the MAINCLKSEL register (Table 18).
 *    b. If no timer-controlled wake-up is needed and the watchdog oscillator is shut down,
 *       ensure that the IRC is powered in the PDRUNCFG register and switch the clock
 *       source to IRC in the MAINCLKSEL register (Table 18). This ensures that the
 *       system clock is shut down glitch-free.
 * 3. Select the power configuration after wake-up in the PDAWAKECFG (Table 42)
 *    register.
 * 4. If an external pin is used for wake-up, enable and clear the wake-up pin in the start
 *    logic registers (Table 36 to Table 39), and enable the start logic interrupt in the NVIC.
 * 5. In the SYSAHBCLKCTRL register (Table 21), disable all peripherals except
 *    counter/timer or WDT if needed.
 * 6. Write one to the SLEEPDEEP bit in the ARM Cortex-M0 SCR register (Table 452).
 * 7. Use the ARM WFI instruction.
 * 
 * Related registers:
 * PCON (Power Control Register,  0x40038000)
 * bit[1] DPDEN (Deep power-down mode enable)
 * 
 *
 */
void enterDeepSleepMode (void) {
	//PMU_PMUCTRL = 
}
