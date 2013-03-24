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

//#define ADAS1000
//#define SRAM_23LC1024
//#define OUTPUT_DATA

void delay(void);
void setLED(int);
void testFail(void);
void sramSelect(void);
void sramDeselect(void);
void set_pins(void);
void adas1000_init(void);
void adas1000_register_write (uint8_t reg, uint32_t value);
uint32_t adas1000_register_read (uint8_t reg);
void adas1000_testtone_enable(void);
uint32_t reverse_byte_order (uint32_t in);

int main(void) {
	uint32_t i,j,k;
	uint32_t la,ra,ll;
	uint32_t base_addr;

	uint8_t request[SSP_FIFOSIZE];
	uint8_t response[SSP_FIFOSIZE];

	systemInit();



	uartInit(115200);

	set_pins();





#ifdef ASAS1000
	sspInit(0, sspClockPolarity_Low, sspClockPhase_RisingEdge);
#endif




#ifdef SRAM_23LC1024
	sspInit(0, sspClockPolarity_Low, sspClockPhase_RisingEdge);

	// 
	// Test byte mode read/write
	//

	// Set byte mode
	sramSelect();
	request[0] = 0x01;  // WRMR command
	request[1] = 0x00; // Byte mode
	gpioSetValue(1,8,1);
	sspSend(0, (uint8_t *)&request, 2);
	sramDeselect();

	// Test by writing to memory
	sramSelect();
	request[0] = 0x02;  // write command
	request[1] = 0x00;  // addr 0x000000
	request[2] = 0x00;  //
	request[3] = 0x00;  //
	request[4] = 0x55;
	sspSend(0, (uint8_t *)&request, 5);
	sramDeselect();

	// Read it back
	sramSelect();
	request[0] = 0x03;  // read command
	request[1] = 0x00;  // addr 0x000000
	request[2] = 0x00;  //
	request[3] = 0x00;  //
	sspSend(0, (uint8_t *)&request, 4);
	sspReceive (0, (uint8_t *)&response, 1);
	sramDeselect();

	printf ("Memory test:\n ");
	if (response[0] == 0x55) {
		printf ("SUCCESS\n");
	} else {
		testFail();
	}

	//
	// Test sequential mode read/write
	//

	// Set sequential mode
	sramSelect();
	request[0] = 0x01; // WRMR command
	request[1] = 0x40; // Sequential mode
	sspSend(0, (uint8_t *)&request, 2);
	sramDeselect();


	// Write to memory
	sramSelect();
	request[0] = 0x02;  // write command
	request[1] = 0x00;  // addr 0x000000
	request[2] = 0x00;  //
	request[3] = 0x00;  //
	sspSend(0, (uint8_t *)&request, 4);
	j=0;
	for (i = 1; i < 1999; i++) {
		request[0] = i;
		sspSend(0, (uint8_t *)&request, 1);
		j += i;
	}
	sramDeselect();

	// Read back	
	sramSelect();
	request[0] = 0x03;  // read command
	request[1] = 0x00;  // addr 0x000000
	request[2] = 0x00;  //
	request[3] = 0x00;  //
	sspSend(0, (uint8_t *)&request, 4);
	for (i = 1; i < 1999; i++) {
		request[0] = i;
		sspReceive(0, (uint8_t *)&response, 1);
		j -= i;
	}
	sramDeselect();
	if (j!=0) {
		testFail();
	}

	delay();

#endif


	// Power everything off by writing 0x0 into ECGCTL
	sspInit(0, sspClockPolarity_Low, sspClockPhase_RisingEdge);
	delay();
	adas1000_register_write (0x01, 0x000000);
	delay();
	set_pins();



	uint32_t frame[16];

	printf ("Starting...\r\n");
	delay();


  	while (1) {

		//printf ("Sleeping...\r\n");
		delay();

		#ifdef SLEEP_EN
		// Seems very important to setup pins before sleep. Sleeping after
		// SPI pin setup does not result in low power sleep.
		set_pins();

		pmuDeepSleep(1);

		//printf ("Wake!\r\n");
		delay();

		sspInit(0, sspClockPolarity_Low, sspClockPhase_RisingEdge);

		// Poll for /DRDY on PIO0_11 (pin 4). Does not work.
		//IOCON_JTAG_TDI_PIO0_11 = IOCON_JTAG_TDI_PIO0_11_FUNC_GPIO;
		//GPIO_GPIO0DIR &= ~(1<<11);

		// Poll for /DRDY on PIO0_5 (pin 5). Works!
		IOCON_PIO0_5 = IOCON_PIO0_5_FUNC_GPIO;
		GPIO_GPIO0DIR &= ~(1<<5);


		#endif // end ifdef SLEEP_EN

#ifdef ADAS1000
		adas1000_init();

		adas1000_testtone_enable();

		// This starts the data...
		adas1000_register_read (0x40);


		k = 0;
		for (j = 0; j<10000; j++) {

			// Wait for /DRDY
			while (gpioGetValue(0,5) != 0) ;
			// Data is available

			ssp0Select();

			// Skip to frame header
			do {
				sspReceive (0, (uint8_t *)&frame[0], 4);
				i = frame[0] &0xff;
				if ( (i & 0x80) == 0) {
					printf ("*");
				}
			} while ( (i&0x80) == 0);


			for (i = 1; i < 4; i++) {
				sspReceive (0, (uint8_t *)&frame[i], 4);
			}

			ssp0Deselect();


			la = reverse_byte_order(frame[1])&0xffffff;
			//la = frame[1]&0xffffff;
			//ll = reverse_byte_order(frame[3]&0xffffff);
			ra = reverse_byte_order(frame[2])&0xffffff;
			//ra = frame[2]&0xffffff;

			//la = 0x01020304;
			//ra = 0x55667788;

			#ifdef SRAM_23LC1024
			if ( (frame[0]&0x40) == 0) {
			// Write ECG record to memory
			sramSelect();
			base_addr = (k++) * 8;
			request[0] = 0x02;  // write command
			request[1] = (base_addr>>16)&0xff; 
			request[2] = (base_addr>>8)&0xff;
			request[3] = base_addr & 0xff;
			sspSend(0, (uint8_t *)&request, 4);
			// Write ECG header byte
			request[0] = frame[0] & 0xff;
			sspSend(0, (uint8_t *)&request, 1);
			// Write LA,RA
			sspSend(0, (uint8_t *)&la, 3);
			sspSend(0, (uint8_t *)&ra, 3);
			sramDeselect();
			}
			#endif

			#ifdef OUTPUT_DATA
			if ( (frame[0]&0x40) == 0) {
				/*
				printf ("%x %d\n", 
					frame[0]&0xff,
					(la&0xffff) 
				);
				*/
				
				printf ("%x\n", 
					frame[0]&0xff
				);
				
			}
			#endif


cmdPoll();

	
		}


		// Power everything off by writing 0x0 into ECGCTL
		adas1000_register_write (0x01, 0x000000);


		#ifdef SRAM_23LC1024
		// Read back data from SRAM
		for (j = 0; j < k; j++) {
			sramSelect();
			base_addr = j * 8;
			request[0] = 0x03;  // read command
			request[1] = (base_addr>>16)&0xff; 
			request[2] = (base_addr>>8)&0xff;
			request[3] = base_addr & 0xff;
			sspSend(0, (uint8_t *)&request, 4);
			sspReceive (0, (uint8_t *)&response, 1);
			printf ("%x ", response[0]);
			la=0; ra=0;
			sspReceive (0, (uint8_t *)&la, 3);
			sspReceive (0, (uint8_t *)&ra, 3);
			printf ("%x %x\n", la, ra);
			sramDeselect();
		}

		#endif

#endif // end ifdef ADAS1000

cmdPoll();


	}


 	 return 0;
}

void delay(void) {
	int i;
	for (i = 0; i < 1024; i++) {
		__asm volatile ("NOP");
	}
}
void setLED (int b) {
	gpioSetValue(1,8,b);
}
void testFail(void) {
	setLED(1);
	while(1);
}
void sramSelect () {
	gpioSetValue(0,3,0);
}
void sramDeselect () {
	gpioSetValue(0,3,1);
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
	#define LEAD1_LA_DIS (1<<23)
	#define LEAD2_LL_DIS (1<<22)
	#define LEAD3_RA_DIS (1<<21)
	#define V1_DIS (1<<20)
	#define V2_DIS (1<<19)
	#define PACE_DIS (1<<14)
	#define RESPM_DIS (1<<13)
	#define RESPPH_DIS (1<<12)
	#define LOFF_DIS (1<<11)
	#define GPIO_DIS (1<<10)
	#define CRC_DIS (1<<9)
	#define A_DIS (1<<7)
	#define RDYRPT (1<<6)
	#define DATAFMT (1<<4)
	#define FRMCTL_SKIP_MASK (0x0000000C) // 0b...0000 1100
	#define FRMCTL_SKIP_1 (0x00000004)
	#define FRMCTL_SKIP_3 (0x00000008)
	//adas1000_register_write (0x0A, 0x079200); // 0000 0111 1001 0010 0000 0000
	//adas1000_register_write (0x0A, 0x1FB600); // 0001 1111 1011 0110 0000 0000
	adas1000_register_write (0x0A,
		 LEAD3_RA_DIS | V1_DIS | V2_DIS 
		| PACE_DIS | RESPM_DIS | RESPPH_DIS 
		| GPIO_DIS | CRC_DIS
		| FRMCTL_SKIP_3  // 1/4 frames: 500Hz rate

	);

	// Write to GPIOCTL
	// Configure GPIO0 as input 0b00000000 00000000 0000[01]00
	//adas1000_register_write (0x06, 0x000004);

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

}

void adas1000_testtone_enable (void) {

	// 10Hz 1mV p-p sine wave to LA
	adas1000_register_write (0x08, // TESTTONE register
		(1<<23)  // connect test toneto LA
		| (1<<2) // TONINT (connect internally)
		| (1<<0) // TONEN (enable test tone)
	);
}

void adas1000_print_diagnostics(void) {
/*
	printf ("OPSTAT= %x\r\n" , adas1000_register_read(0x1f));
	printf ("CMREFCTL= %x\r\n" , adas1000_register_read(0x05));
	printf ("FRMCTL= %x\r\n" , adas1000_register_read(0x0a));
	printf ("ECGCTL= %x\r\n" , adas1000_register_read(0x01));	
	printf ("GPIOCTL= %x\r\n" , adas1000_register_read(0x06));	
	printf ("OPSTAT= %x\r\n" , adas1000_register_read(0x1f));
*/
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
	uint32_t response;
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
	IOCON_JTAG_TCK_PIO0_10 = IOCON_JTAG_TCK_PIO0_10_FUNC_GPIO; // pin 3
	IOCON_JTAG_TDI_PIO0_11 = IOCON_JTAG_TDI_PIO0_11_FUNC_GPIO; // pin 4
	IOCON_JTAG_TMS_PIO1_0 = IOCON_JTAG_TMS_PIO1_0_FUNC_GPIO;	
	IOCON_JTAG_TDO_PIO1_1 = IOCON_JTAG_TDO_PIO1_1_FUNC_GPIO;
	IOCON_JTAG_nTRST_PIO1_2 = IOCON_JTAG_nTRST_PIO1_2_FUNC_GPIO; // causes extra 150µA drain?!
	IOCON_SWDIO_PIO1_3 = IOCON_SWDIO_PIO1_3_FUNC_GPIO;
	IOCON_PIO1_4 = IOCON_PIO1_4_FUNC_GPIO;
	IOCON_PIO1_5 = IOCON_PIO1_5_FUNC_GPIO;
	//IOCON_PIO1_6 = IOCON_PIO1_6_FUNC_GPIO; // RXD
	//IOCON_PIO1_7 = IOCON_PIO1_7_FUNC_GPIO; // TXD
	IOCON_PIO1_8 = IOCON_PIO1_8_FUNC_GPIO; 
	IOCON_PIO1_9 = IOCON_PIO1_9_FUNC_GPIO; 
	IOCON_PIO1_10 = IOCON_PIO1_10_FUNC_GPIO; 
	IOCON_PIO1_11 = IOCON_PIO1_11_FUNC_GPIO;

	for (i = 0; i < 10; i++) {
		gpioSetDir (0,i,OUTPUT); // 1 = output
		gpioSetValue (0,i,1); // this seems to make no difference
	}
	for (i = 0; i < 7; i++) {
		gpioSetDir (1,i,OUTPUT); // 1 = output
		gpioSetValue (1,i,1); // this seems to make no difference
	}
	for (i = 9; i < 12; i++) {
		gpioSetDir (1,i,OUTPUT); // 1 = output
		gpioSetValue (1,i,1); // this seems to make no difference
	}

}

