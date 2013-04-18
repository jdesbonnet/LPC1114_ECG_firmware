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
#include "core/pwm/pwm.h"

#include "lpc111x.h"

#ifdef CFG_INTERFACE
  #include "core/cmd/cmd.h"
#endif

#define INPUT (0)
#define OUTPUT (1)

#define CMD_WAKEUP (0x02)
#define CMD_STANDBY (0x04)
#define CMD_RESET (0x06)
#define CMD_START (0x08)
#define CMD_STOP (0x0a)
#define CMD_RDATAC (0x10)
#define CMD_SDATAC (0x11)
#define CMD_RDATA (0x12)

#define REG_ID (0x00)
#define REG_CONFIG1 (0x01)
#define REG_CONFIG2 (0x02)

void ads1x9x_command (uint8_t command);
void ads1x9x_ecg_read (uint8_t *buf);
int ads1x9x_drdy_wait (int timeout);
uint8_t ads1x9x_register_read (uint8_t registerId);
void ads1x9x_register_write (uint8_t registerId, uint8_t registerValue);
void ads1x9x_hw_reset (void);
void delay(int delay);

#define ADS1x9x_DRDY_PORT (1)
#define ADS1x9x_DRDY_PIN (0)


uint8_t ads1292r_default_register_settings[15] = {
	0x00, //Device ID read Ony

	// CONFIG1 (0x01)
	//0x02, // SINGLE_SHOT=0 (continuous), 500sps
	0x82, // SINGLE_SHOT=1 , 500sps


	// CONFIG2 (0x02)
	// was E0
	//0xC8, //CONFIG2: PDB_LOFF_COMP=1 (lead off comp enabled), PDB_REFBUF=1 (ref buf en), VREF_4V=0, CLK_EN=1
	0xCB, //CONFIG2: PDB_LOFF_COMP=1 (lead off comp enabled), PDB_REFBUF=1 (ref buf en), VREF_4V=0, CLK_EN=1, int test

	// LOFF (0x03)
     	0xF0,

	//CH1SET (0x04) (PGA gain = 6)
	//0x00,
	0x05, // MUX1=Test

	//CH2SET (0x05) (PGA gain = 6)
	0x00,

	//RLD_SENS (0x06) (default)
	0x2C,

	//LOFF_SENS (0x07) (default)
	0x0F,    

	//LOFF_STAT (0x08)
 	0x00,

	//RESP1 (0x09)
	0xEA,

	//RESP2 (0x0a0)
	0x03,

	//GPIO
	//0x0C // GPIOC2=INPUT, GPIOC1=INPUT
	0x01 // GPIOC2=OUTPUT, GPIOC1=OUTPUT, GPIOD2=0, GPIOD1=1
};



int main(void) {
	int i,j,n;
	int id;
	uint8_t record[12];
	systemInit();
	uartInit(115200);

	set_pins();


	// Experiment set PIO1_9 as external clock. 500kHz, 50% duty cycle.
	#ifdef PWM_EN
	pwmInit();
	pwmSetFrequencyInMicroseconds(2);
	pwmSetDutyCycle(50);
	pwmStart();
	#endif




	//sspInit(0, sspClockPolarity_Low, sspClockPhase_RisingEdge);
	sspInit(0, sspClockPolarity_Low, sspClockPhase_FallingEdge);
	uint8_t request[SSP_FIFOSIZE];
	uint8_t response[SSP_FIFOSIZE];


	// Configure the /DRDY monitoring pin for input
	gpioSetDir(ADS1x9x_DRDY_PORT,ADS1x9x_DRDY_PIN,INPUT);


  	while (1) {



		n=6;
		for (j = 0; j < 1000; j++) {


// for debugging-- so we can see RST on scope using CS line
ssp0Select();
delay(64);
ssp0Deselect();
delay(64);

			// Observation: after a hard reset, DOUT is always 0.
			ads1x9x_hw_reset();
			delay(100000);

			//ads1x9x_command (CMD_STOP);
			ads1x9x_command (CMD_SDATAC);

			// CLKSEL tied high (internal ck)
			for (i = 1; i < 12; i++) {
				ads1x9x_register_write (i,ads1292r_default_register_settings[i]);
			}
			delay(512);

			ads1x9x_command (CMD_STOP);
			delay(512);

			// Read all registers
			printf ("{");
			for (i = 0; i < 12; i++) {
				id = ads1x9x_register_read(i);
				printf ("%x ", id);
			}
			printf ("}\r\n");

			delay (1024);

			// Read ECG record
			ads1x9x_command (CMD_START);



			// Wait for /DRDY
	ssp0Select();
			ads1x9x_drdy_wait(0);
	ssp0Deselect(); 

delay (2048);

			// Read data by commandf
			ads1x9x_command(CMD_RDATA);




			// Read ECG record
			ads1x9x_ecg_read (&record);
			// Display ECG record
			printf ("[");
			for (i = 0; i < 9; i++) {
				printf ("%x ", record[i]);
			}
			printf ("]");
			

	
			delay(4096);


		}

	}


 	 return 0;
}

/**
 * Wait until /DRDY signal is asserted.
 *
 * @param timeout Timeout in iterations. 0 means no timeout.
 * @return 0 /DRDY detected. -1 for timeout.
 */
int ads1x9x_drdy_wait (int timeout) {
	printf ("drdy=(%d)",gpioGetValue(ADS1x9x_DRDY_PORT,ADS1x9x_DRDY_PIN));
	while (gpioGetValue(ADS1x9x_DRDY_PORT,ADS1x9x_DRDY_PIN) == 1)  {
		delay(16);
		if (timeout != 0) {
			if ( (--timeout) == 0 ) {
				// reached timeout
				return -1;
			}
		} 
	}
	return 0;
}

void ads1x9x_command (uint8_t command) {
	uint8_t request[4];
	request[0] = command;
	ssp0Select(); delay(1);
	sspSend(0, (uint8_t *)&request, 1);
	delay(32);
	ssp0Deselect();
	delay(32);
}

uint8_t ads1x9x_register_read (uint8_t registerId) {
	uint8_t buf[4];
	buf[0] = 0x20 | registerId; // RREG
	buf[1] = 0x00; // n-1 registers
	ssp0Select(); delay(32);
	sspSend(0, (uint8_t *)&buf, 2);
	sspReceive (0, (uint8_t *)&buf, 1);
	delay(32);
	ssp0Deselect();
	delay(32);
	return buf[0];
}

void ads1x9x_ecg_read (uint8_t *buf) {
	ssp0Select(); delay(32);
	sspReceive (0, (uint8_t *)buf, 9);
	delay(32);
	ssp0Deselect();
	delay(32);
}

void ads1x9x_register_write (uint8_t registerId, uint8_t registerValue) {
	uint8_t request[4];
	request[0] = 0x40 | registerId; // WREG
	request[1] = 0x00; // n-1 registers
	request[2] = registerValue;
	ssp0Select(); delay(1);
	sspSend(0, (uint8_t *)&request, 3);
	delay(32);
	ssp0Deselect();
	delay(32);
}

void ads1x9x_hw_reset (void) {
	// Assert physical reset line
	gpioSetValue (0,6,0);
	delay(16);
	gpioSetValue (0,6,1);
	delay(512);
}

void delay (int n) {
	int i;
	for (i = 0; i < n; i++) {
		__asm volatile ("NOP");
	}
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

	// Found that enabling/disabling these lines affected SPI trace.
	// Reason it seems that this disables pullup (enabled by default)
	IOCON_PIO0_8 = IOCON_PIO0_8_FUNC_GPIO; // pin 1 MISO
	IOCON_PIO0_9 = IOCON_PIO0_9_FUNC_GPIO; // pin 2 MOSI


	//IOCON_JTAG_TCK_PIO0_10 = IOCON_JTAG_TCK_PIO0_10_FUNC_GPIO;
	IOCON_JTAG_TDI_PIO0_11 = IOCON_JTAG_TDI_PIO0_11_FUNC_GPIO; 
	IOCON_JTAG_TMS_PIO1_0 = IOCON_JTAG_TMS_PIO1_0_FUNC_GPIO;	
	IOCON_JTAG_TDO_PIO1_1 = IOCON_JTAG_TDO_PIO1_1_FUNC_GPIO;
	IOCON_JTAG_nTRST_PIO1_2 = IOCON_JTAG_nTRST_PIO1_2_FUNC_GPIO; // causes extra 150µA drain?!
	IOCON_SWDIO_PIO1_3 = IOCON_SWDIO_PIO1_3_FUNC_GPIO;
	IOCON_PIO1_4 = IOCON_PIO1_4_FUNC_GPIO;
	IOCON_PIO1_5 = IOCON_PIO1_5_FUNC_GPIO;
	//IOCON_PIO1_6 = IOCON_PIO1_6_FUNC_GPIO; // UART
	//IOCON_PIO1_7 = IOCON_PIO1_7_FUNC_GPIO; // UART
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

