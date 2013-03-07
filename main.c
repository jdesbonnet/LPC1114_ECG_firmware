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
#include "core/ssp/ssp.h"

#include "lpc111x.h"

#ifdef CFG_INTERFACE
  #include "core/cmd/cmd.h"
#endif

#define INPUT (0)
#define OUTPUT (1)

void delay(void);
void set_pins(void);
void adas1000_init(void);
void adas1000_register_write (uint8_t reg, uint32_t value);
uint32_t adas1000_register_read (uint8_t reg);
uint32_t reverse_byte_order (uint32_t in);

int main(void) {
	int i,j;
	uint32_t la,ra,ll;
	systemInit();
	uartInit(115200);
	set_pins();

	sspInit(0, sspClockPolarity_Low, sspClockPhase_RisingEdge);

	uint8_t request[SSP_FIFOSIZE];
	uint8_t response[SSP_FIFOSIZE];

	uint32_t frame[12];



  	while (1) {

		adas1000_init();
		delay();
/*
	printf ("OPSTAT= %x\r\n" , adas1000_register_read(0x1f));
	printf ("CMREFCTL= %x\r\n" , adas1000_register_read(0x05));
	printf ("FRMCTL= %x\r\n" , adas1000_register_read(0x0a));
	printf ("ECGCTL= %x\r\n" , adas1000_register_read(0x01));	
	printf ("GPIOCTL= %x\r\n" , adas1000_register_read(0x06));	
	printf ("OPSTAT= %x\r\n" , adas1000_register_read(0x1f));
*/

		// This starts the data...
		adas1000_register_read (0x40);

		delay();


		for (j = 0; j<1000; j++) {
			for (i = 0; i < 10; i++) {
				ssp0Select();
				// Receive 32 bits
				//sspReceive (0, (uint8_t *)&response, 4);
				sspReceive (0, (uint8_t *)&frame[i], 4);
				ssp0Deselect();
			}

	
			for (i = 0; i < 10; i++) {
				//printf ("%0x " , reverse_byte_order(frame[i]));
				//printf ("%d " , reverse_byte_order(frame[i]&0xffffff)-290000000 );
			}

			la = reverse_byte_order(frame[0]&0xffffff);
			ll = reverse_byte_order(frame[1]&0xffffff);
			ra = reverse_byte_order(frame[2]&0xffffff);
			printf ("%d %d", 
				la-ra, // Lead I
				reverse_byte_order(frame[5])&0xffffff	// pace
			); 
			printf ("\n");
	
		}

		pmuDeepSleep(10);

	}


 	 return 0;
}

void delay(void) {
	int i;
	for (i = 0; i < 1024; i++) {
		__asm volatile ("NOP");
	}
}

void adas1000_init (void) {

	// Write to CMREFCTL Common Mode, Reference and Shield Drive Control Register
	// Bit [23:19] common mode electrode select. Selecting mean of LA, LL, RA.
	// Bit [7:4] = 0000 : select RLD_OUT pin for reference drive 
	// Bit 3 DRVCM = 1 common mode is driven out of the external common-mode pin
	// Bit 2 EXTEM = 0 use internal common mode
	// Bit 1 RLDSEL = 1 enable Right-leg drive
	// Bit 0 SHLDEN = 1 enable shield drive
	adas1000_register_write (0x05, 0xE0000B);

	// Write to FRMCTL (Frame Control Register)
	// This determines what's included in the data frame.
	//
	adas1000_register_write (0x0A, 0x079200);

	// Write to ECGCTL
	// Bits [23:19]: all 1 enable all 5 ECG channels
	// Bit 10: CHCONFIG = 1 differential input (analog lead mode)
	// Bits [9:8] GAIN = 00 (x1.4)
	// Bit 7 VREFBUF VREF buffer enable = 1
	// Bit 6 CLKEXT = 0 Use XTAL
	// Bit 5 Master = 1
	// Bit 4 Gang = 0
	// Bit 3 HP = 1 2MSP high performance / low noise
	// Bit 2 CNVEN Conversion Enable = 1
	// Bit 1 PWREN Power Enable = 1
	// Bit 0 SWRST Reset = 0
	adas1000_register_write (0x01, 0xF804AE);

	// Write to GPIOCTL
	// Configure GPIO0 as input 0b00000000 00000000 0000[01]00
	adas1000_register_write (0x06, 0x000004);
}

void adas1000_register_write (uint8_t reg, uint32_t value) {

	uint8_t request[4];
	request[0] = 0x80 | (reg & 0x7f);
	request[1] = (value >> 16)&0xff;
	request[2] = (value >> 8) & 0xff;
	request[3] = value & 0xff;

	ssp0Select();
	sspSend(0, (uint8_t *)&request, 4);
	ssp0Deselect();
}

uint32_t adas1000_register_read (uint8_t reg) {
	uint8_t request[4];
	//uint8_t response[4];
	uint32_t response, ret;
	ssp0Select();
	request[0] = reg & 0x7F;
	sspSend(0, (uint8_t *)&request, 4);
	sspReceive (0, (uint8_t *)&response, 4);
	ssp0Deselect();
	// Reverse byte order
	return reverse_byte_order(response);
}

uint32_t reverse_byte_order (uint32_t in) {
	return (in>>24) | ((in>>8)&0x0000ff00) | ((in<<8)&0x00ff0000) | (in<<24);
}

void set_pins(void) {

	int i;
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
	IOCON_JTAG_nTRST_PIO1_2 = IOCON_JTAG_nTRST_PIO1_2_FUNC_GPIO; // causes extra 150µA drain?!
	IOCON_SWDIO_PIO1_3 = IOCON_SWDIO_PIO1_3_FUNC_GPIO;
	IOCON_PIO1_4 = IOCON_PIO1_4_FUNC_GPIO;
	IOCON_PIO1_5 = IOCON_PIO1_5_FUNC_GPIO;
	//IOCON_PIO1_6 = IOCON_PIO1_6_FUNC_GPIO; // THIS *SEEMED* MADE ALL THE DIFFERENCE! 4µA
	//IOCON_PIO1_7 = IOCON_PIO1_7_FUNC_GPIO;
	IOCON_PIO1_8 = IOCON_PIO1_8_FUNC_GPIO;
	IOCON_PIO1_9 = IOCON_PIO1_9_FUNC_GPIO;
	IOCON_PIO1_10 = IOCON_PIO1_10_FUNC_GPIO;
	IOCON_PIO1_11 = IOCON_PIO1_11_FUNC_GPIO;

	for (i = 0; i < 10; i++) {
		gpioSetDir (0,i,1); // 1 = output
		gpioSetValue (0,i,1); // this seems to make no difference
	}
	for (i = 0; i < 7; i++) {
		gpioSetDir (1,i,1); // 1 = output
		gpioSetValue (1,i,1); // this seems to make no difference
	}
	for (i = 9; i < 12; i++) {
		gpioSetDir (1,i,1); // 1 = output
		gpioSetValue (1,i,1); // this seems to make no difference
	}

}

