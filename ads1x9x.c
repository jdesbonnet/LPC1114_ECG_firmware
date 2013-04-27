#include "projectconfig.h"
#include "sysinit.h"
#include "core/gpio/gpio.h"
#include "core/ssp/ssp.h"
#include "lpc111x.h"
#include "ads1x9x.h"

uint8_t ads1292r_default_register_settings[15] = {
	0x00, //Device ID read Only

	// CONFIG1 (0x01)
	0x02, // SINGLE_SHOT=0 (continuous), 500sps
	//0x82, // SINGLE_SHOT=1 , 500sps


	// CONFIG2 (0x02)
	// was E0
	0xC8, //CONFIG2: PDB_LOFF_COMP=1 (lead off comp enabled), PDB_REFBUF=1 (ref buf en), VREF_4V=0, CLK_EN=1
	//0xCB, //CONFIG2: PDB_LOFF_COMP=1 (lead off comp enabled), PDB_REFBUF=1 (ref buf en), VREF_4V=0, CLK_EN=1, int test

	// LOFF (0x03)
     	0xF0,

	//CH1SET (0x04) (PGA gain = 6)
	0x00,
	//0x05, // MUX1=Test

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
	0x0C // GPIOC2=INPUT, GPIOC1=INPUT
	//0x01 // GPIOC2=OUTPUT, GPIOC1=OUTPUT, GPIOD2=0, GPIOD1=1
};




/**
 * Wait until /DRDY signal is asserted.
 *
 * @param timeout Timeout in iterations. 0 means no timeout.
 *
 * @return 0 /DRDY detected. -1 for timeout.
 */
int ads1x9x_drdy_wait (int timeout) {
	while (gpioGetValue(ADS1x9x_DRDY_PORT,ADS1x9x_DRDY_PIN) == 1)  {
		if (timeout != 0) {
			if ( (--timeout) == 0 ) {
				// reached timeout
				return -1;
			}
		} 
	}
	return 0;
}

/**
 * Issue ADS1x9x command.
 *
 * @param command Allowed commands are CMD_WAKEUP, CMD_STANDBY, CMD_RESET, 
 * CMD_START, CMD_STOP,
 * CMD_RDATAC, CMD_SDATAC, CMD_RDATA
 * 
 * @return void 
 */
void ads1x9x_command (uint8_t command) {
	uint8_t request[4];
	request[0] = command;
	ssp0Select(); delay(1);
	sspSend(0, (uint8_t *)&request, 1);
	delay(32);
	ssp0Deselect();
	delay(32);
}

/**
 * Read the value of a ADS1x9x register.
 * @return Register value.
 */
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

/**
 * Write to ADS1x9x register.
 */
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

/**
 * Read one ECG record which comprises 3 x 24 bit (9 bytes).
 */
void ads1x9x_ecg_read (uint8_t *buf) {
	ssp0Select(); delay(32);
	sspReceive (0, (uint8_t *)buf, 9);
	delay(32);
	ssp0Deselect();
	delay(32);
}


/**
 * Assert physical ADS1x9x RESET line. NB: the device requires 
 * some time [ref] to re-initialize after hardware reset.
 */
void ads1x9x_hw_reset (void) {
	// Assert physical reset line
	gpioSetValue (0,6,0);
	delay(16);
	gpioSetValue (0,6,1);
	delay(512);
}

/**
 * Reset and initialize ADS1x9x device.
 */
void ads1x9x_init (void) {

	int i;

	ads1x9x_hw_reset();
	delay(100000);
	//ads1x9x_command (CMD_STOP);

	// SDATAC (stop continuous data conversion) command is required prior
	// to sending register write commands.
	ads1x9x_command (CMD_SDATAC);

	// CLKSEL tied high (internal ck)
	for (i = 1; i < 12; i++) {
		ads1x9x_register_write (i,ads1292r_default_register_settings[i]);
	}
	delay(512);
}

