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
	//0x00,
	//0x05, // MUX1=Test
	0x10, // PGA gain = 1

	//CH2SET (0x05) (PGA gain = 6)
	//0x00,
	0x80, // disable ch

	//RLD_SENS (0x06) (default)
	// PDB_RLD=1, RLD_LOFF_SENS=0, RLD2N=1, RLD2P=1, RLD1N=0, RLD1P=0
	0x2C,


	//LOFF_SENS (0x07) (default)
	0x0F,    

	//LOFF_STAT (0x08)
 	0x00,

	//RESP1 (0x09)
	0xEA,

	//RESP2 (0x0a0)
	//0x03,
	0x83,

	//GPIO
	0x0C // GPIOC2=INPUT, GPIOC1=INPUT
	//0x01 // GPIOC2=OUTPUT, GPIOC1=OUTPUT, GPIOD2=0, GPIOD1=1
};


void ads1x9x_select(void) {
	sspInit(0, sspClockPolarity_Low, sspClockPhase_FallingEdge); // works for ADS1x9x
	ssp0Select();
}

void ads1x9x_deselect(void) {
	ssp0Deselect();
}


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
	ads1x9x_select(); delay(1);
	sspSend(0, (uint8_t *)&request, 1);
	delay(32);
	ads1x9x_deselect();
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
	ads1x9x_select(); delay(32);
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
	ads1x9x_select(); delay(1);
	sspSend(0, (uint8_t *)&request, 3);
	delay(32);
	ssp0Deselect();
	delay(32);
}

/**
 * Read one ECG record which comprises 3 x 24 bit (9 bytes).
 */
void ads1x9x_ecg_read (uint8_t *buf) {
	ads1x9x_select(); delay(32);
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

int ads1x9x_test(void) {
	return (ads1x9x_register_read (REG_ID) == 0x53) ? 0 : -1;
}

void ads1x9x_measure_test_signal (int pga_gain) {

	uint8_t buf[9];

	printf ("PGA Gain %d\r\n", pga_gain);

	// Stop continuous data to allow register read.
	ads1x9x_command (CMD_SDATAC);

	// Using internal reference. Set PDB_REFBUF=1 and wait for ref to settle
	// Bit 7: always 1
	// Bit 6: PDB_LOFF_COMP = 0 (Lead-off comparitor power-down)
	// Bit 5: PDB_REFBUF = 1 (reference buffer enabled)
	// Bit 4: VREF_4V = 0 (reference set to 2.42V)
	// Bit 3: CLK_EN = 0 (oscillator clock output disabled)
	// Bit 2: always 0
	// Bit 1: INT_TEST = 0 (test signal off)
	// Bit 0: TEST_FREQ = 0 (DC)
	ads1x9x_register_write (REG_CONFIG2, 0xA0);

	// Bits[2:0] = 2 (500 kSPS)
	ads1x9x_register_write (REG_CONFIG1, 0x02);

	// Set channels to input short
	ads1x9x_register_write (REG_CH1SET, 0x01);
	ads1x9x_register_write (REG_CH2SET, 0x01);

	ads1x9x_command (CMD_RDATAC);

	if ( ads1x9x_drdy_wait (100000) == -1 ) {
		printf ("ERROR 1\n");
		return;
	}


	ads1x9x_ecg_read (buf);

	ads1x9x_command (CMD_SDATAC);

	// Display ECG record
	print_ecg_record (buf);

	// Activate a 1mV x Vref/2.4 square wave test signal
	ads1x9x_register_write(REG_CONFIG2, 0xA3);

	int pga_bits = 0;
	switch (pga_gain) {
		case 1: 
		case 2: 
		case 3:
		case 4: 
			pga_bits = pga_gain;
			break;
		case 6: 
			pga_bits = 0;
			break;
		case 8:
			pga_bits = 5;
			break;
		case 12:
			pga_bits = 6;
			break;
	}

	ads1x9x_register_write(REG_CH1SET, 0x05 | (pga_bits<<4));
	ads1x9x_register_write(REG_CH2SET, 0x05 | (pga_bits<<4));

	ads1x9x_command (CMD_RDATAC);





	int i;
	int32_t ch1,ch2;
	int64_t ch1_sum=0, ch2_sum=0;
	int32_t min_ch1 = (1<<24);
	int32_t max_ch1 = -(1<<24);
	int32_t min_ch2 = (1<<24);
	int32_t max_ch2 = -(1<<24);

//24 bits per data channel in binary twos complement format, MSB first.
// LSB has weight of Vref / (1<<23 - 1).
// Test signal is  ±(Vrefp - Vrefn)/ 2400 = ±1.008 mV (ptpv 2.016mV) with 2.42 ref.


	// Analyze one cycle of test signal to find mid point between
	// high and low level of test signal.
	for (i = 0; i < 500; i++) {
		if ( ads1x9x_drdy_wait (10000) == -1 ) {
			printf ("ERROR 2\n");
			return;
		}

		ads1x9x_ecg_read (buf);
		ch1 = (buf[3]<<24 | buf[4]<<16 | buf[5]<<8) / 256;
		ch2 = (buf[6]<<24 | buf[7]<<16 | buf[8]<<8) / 256;


		ch1_sum += ch1;
		ch2_sum += ch2;

		if (ch1 > max_ch1) {
			max_ch1 = ch1;
		}
		if (ch1 < min_ch1) {
			min_ch1 = ch1;
		}
		if (ch2 > max_ch2) {
			max_ch2 = ch2;
		}
		if (ch2 < min_ch2) {
			min_ch2 = ch2;
		}
		
	}

	int32_t ch1_mean = (uint32_t)(ch1_sum/500);
	int32_t ch2_mean = (uint32_t)(ch2_sum/500);


	printf ("min_ch1=%d max_ch1=%d range=%d mean=%d\r\n", min_ch1, max_ch1, (max_ch1-min_ch1), ch1_mean );
	printf ("min_ch2=%d max_ch2=%d range=%d mean=%d\r\n", min_ch2, max_ch2, (max_ch2-min_ch2), ch2_mean );

	//
	// Now use the mid-point to get a mean of the high and low signal levels.
	//
	int32_t ch1_low=0;
	int32_t ch1_high=0;
	int32_t ch2_low=0;
	int32_t ch2_high=0;

	int ch1_low_count=0;
	int ch1_high_count=0;
	int ch2_low_count=0;
	int ch2_high_count=0;

	for (i = 0; i < 500; i++) {
		if ( ads1x9x_drdy_wait (10000) == -1 ) {
			printf ("ERROR 1\n");
			return;
		}

		ads1x9x_ecg_read (buf);
		ch1 = (buf[3]<<24 | buf[4]<<16 | buf[5]<<8) / 256;
		ch2 = (buf[6]<<24 | buf[7]<<16 | buf[8]<<8) / 256;

		if (ch1 >= ch1_mean) {
			ch1_high+= ch1;
			ch1_high_count++;
		}
		if (ch1 < ch1_mean) {
			ch1_low += ch1;
			ch1_low_count++;
		}

		if (ch2 >= ch2_mean) {
			ch2_high+= ch2;
			ch2_high_count++;
		}
		if (ch2 < ch2_mean) {
			ch2_low += ch2;
			ch2_low_count++;
		}

	}

	int32_t ch1_low_mean = ch1_low/ch1_low_count;
	int32_t ch1_high_mean = ch1_high/ch1_high_count;
	int32_t ch2_low_mean = ch2_low/ch2_low_count;
	int32_t ch2_high_mean = ch2_high/ch2_high_count;

	printf ("ch1_low_mean=%d ch1_low_count=%d\r\n", ch1_low_mean, ch1_low_count);
	printf ("ch1_high_mean=%d ch1_high_count=%d\r\n", ch1_high_mean, ch1_high_count);
	printf ("ch2_low_mean=%d ch2_low_count=%d\r\n", ch2_low_mean, ch2_low_count);
	printf ("ch2_high_mean=%d ch2_high_count=%d\r\n", ch2_high_mean, ch2_high_count);
	printf ("ch1_range=%d\r\n", (ch1_high_mean - ch1_low_mean));
	printf ("ch2_range=%d\r\n", (ch2_high_mean - ch2_low_mean));
	printf ("ch1_adc_per_V=%d\r\n", (1000*(ch1_high_mean - ch1_low_mean))/ 2016);
	printf ("ch2_adc_per_V=%d\r\n", (1000*(ch2_high_mean - ch2_low_mean))/ 2016);
}
