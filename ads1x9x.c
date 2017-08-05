#include "projectconfig.h"
#include "sysinit.h"
#include "core/gpio/gpio.h"
#include "core/ssp/ssp.h"
#include "lpc111x.h"
#include "ads1x9x.h"


uint32_t int_sqrt(uint32_t a) {
    uint64_t rem = 0;
    int32_t root = 0;
    int32_t i;

    for (i = 0; i < 16; i++) {
        root <<= 1;
        rem <<= 2;
        rem += a >> 30;
        a <<= 2;

        if (root < rem) {
            root++;
            rem -= root;
            root++;
        }
    }

    return (uint32_t) (root >> 1);
}


uint8_t ads1292r_default_register_settings[15] = {
	0x00, //Device ID read Only

	// CONFIG1 (0x01)
	0x02, // SINGLE_SHOT=0 (continuous), 500sps
	//0x82, // SINGLE_SHOT=1 , 500sps


	// CONFIG2 (0x02)
	// was E0
	0xA0, //CONFIG2: PDB_LOFF_COMP=0 (lead off comp off), PDB_REFBUF=1 (ref buf en), VREF_4V=0, CLK_EN=0
	//0xC8, //CONFIG2: PDB_LOFF_COMP=1 (lead off comp enabled), PDB_REFBUF=1 (ref buf en), VREF_4V=0, CLK_EN=1
	//0xCB, //CONFIG2: PDB_LOFF_COMP=1 (lead off comp enabled), PDB_REFBUF=1 (ref buf en), VREF_4V=0, CLK_EN=1, int test

	// LOFF (0x03)
     	0xF0,

	//CH1SET (0x04) (PGA gain = 6)
	//0x00,
	//0x05, // MUX1=Test
	0x10, // PGA gain = 1

	//CH2SET (0x05) (PGA gain = 6)
	0x10,
	//0x80, // disable ch

	//RLD_SENS (0x06) (default)
	// PDB_RLD=1, RLD_LOFF_SENS=0, RLD2N=1, RLD2P=1, RLD1N=0, RLD1P=0
	//0x2C,
	0x20,


	//LOFF_SENS (0x07) (default)
	0x0F,    

	//LOFF_STAT (0x08)
 	0x00,

	//RESP1 (0x09)
	// bit 7: RESP_DEMOD_EN1: 1 = enable respiration demodulation circuitary on Ch1
	// bit 6: RESP_MOD_EN: 1 = enable respiration modulation circuitary on ch1
	// bits [5:2] : RESP_PH respiration phase
	// bit 1 : must be set to 1
	// bit 0 : RESP_CTRL respiration control. 1 = internal respiration with external ck, 0=internal ck.
	//0xEA,
	0x02,

	//RESP2 (0x0a0)
	// bit 7: CALIB_ON
	// bits [6:3]: must be 0
	// bit 2: RESP_FREQ 0 = 32kHz, 1 = 64kHz
	// bit 1: RLDREF_INT 0 = signal fed externally, 1 = AVDD-AVSS/2 generated internally
	// bit 0: must be 1
	0x03,
	//0x83,

	//GPIO
	// bits [7:4] must be 0
	// bit 3: GPIOC2 control, 0=output, 1=input
	// bit 2: GPIOC1 control, 0=output, 1=input
	// bit 1: GPIOD2 data,
	// bit 0: GPIOD1 data 
	//0x0C // GPIOC2=INPUT, GPIOC1=INPUT, data n/a
	0x00 // GPIO2,1 as output, set low
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
		if (timeout-- == 0) {
			return -1;
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
	delay(16);
	ssp0Deselect();
	delay(16);
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
	uint8_t v = ads1x9x_register_read (REG_ID);
	//return ( v == 0x53 || v == 0x73) ? 0 : -1;
	if (! (v == 0x53 || v == 0x73) ) {
		return -1;
	}

	// Blink LED1 on GPIO1
	int i;
	for (i = 0; i < 2; i++) {
		v = ads1x9x_register_read (REG_GPIO);
		v |= (0x03); // set GPIO1D=1
		ads1x9x_register_write(REG_GPIO,v);
		delay(1000000);
		v &= ~(0x03); // set GPIO1D=0
		ads1x9x_register_write(REG_GPIO,v);
		delay(1000000);
	}
}

/**
 * Return bit mask for PGA gain (needs to be <<4). Allowed
 * gain values: 1-4,6,8,12.
 */
int ads1x9x_get_pga_bits(int pga_gain) {
	switch (pga_gain) {
		case 1: 
		case 2: 
		case 3:
		case 4: 
			return pga_gain;
			break;
		case 6: 
			return 0;
			break;
		case 8:
			return 5;
			break;
		case 12:
			return 6;
			break;
	}
	return 0;
}

void ads1x9x_measure_shorted () {

	uint8_t buf[9];

	int32_t adc[2] = {0,0};

	// http://www.johndcook.com/standard_deviation.html
	// also // See Knuth TAOCP vol 2, 3rd edition, page 232
	int64_t oldM[2], newM[2],oldS[2],newS[2];
	int64_t sum[2]= {0,0};

	int i, j;

	int nsample = SPS/2;
	//int nsample = 16;

	ads1x9x_command (CMD_RDATAC);

	for (i = 0; i < nsample; i++) {

		if ( ads1x9x_drdy_wait (10000) == -1 ) {
			printf ("ERROR 2\n");
			return;
		}

		ads1x9x_ecg_read (buf);
		adc[CH1] = (buf[3]<<24 | buf[4]<<16 | buf[5]<<8) / 256;
		adc[CH2] = (buf[6]<<24 | buf[7]<<16 | buf[8]<<8) / 256;

		if (i == 0) {
			for (j = 0; j < 2; j++) {
				oldM[j] = newM[j] = adc[j];
				oldS[j] = 0;
			}
		} else {
			for (j = 0; j < 2; j++) {
				newM[j] = oldM[j] + (adc[j] - oldM[j])/(i+1);
				newS[j] = oldS[j] + (adc[j] - oldM[j])*(adc[j] - newM[j]);
				// setup for next iter
				oldM[j] = newM[j];
				oldS[j] = newS[j];
			}
		}
		for (j = 0; j < 2; j++) {
			sum[j] += adc[j];
		}

	}

	ads1x9x_command (CMD_SDATAC);


	//printf ("isqrt(1000000)=%d\r\n", int_sqrt(1000000));

	printf ("Short (mean of %d samples):", nsample);
	for (j = 0; j < 2; j++) {
		printf (" ch%dmeanN=%d", (j+1), (int)newM[j]);
		printf (" ch%dmeanO=%d", (j+1), (int)(sum[j]/nsample) );
		printf (" ch%dstd=%d", (j+1), (int)int_sqrt(newS[j]/(nsample-1)));
	}
	printf ("\r\n");
	
}



void ads1x9x_measure_test_signal () {

	int32_t adc[2] = {0,0};
	int64_t adc_sum[2] = {0,0};
	int32_t adc_min[2] = {0,0};
	int32_t adc_max[2] = {0,0};
	int32_t adc_midpoint[2] = {0,0};

	uint8_t buf[9];

	
	ads1x9x_command (CMD_RDATAC);


	int i,j;

	for (j = 0; j < 2; j++) {
		adc_min[j] = (1<<24);
		adc_max[j] = -(1<<24);
	}

//24 bits per data channel in binary twos complement format, MSB first.
// LSB has weight of Vref / (1<<23 - 1).
// Test signal is  ±(Vrefp - Vrefn)/ 2400 = ±1.008 mV (ptpv 2.016mV) with 2.42 ref.


	// Analyze one cycle of test signal to find mid point between
	// high and low level of test signal.
	for (i = 0; i < SPS; i++) {
		if ( ads1x9x_drdy_wait (10000) == -1 ) {
			printf ("ERROR 2\n");
			return;
		}

		ads1x9x_ecg_read (buf);
		adc[CH1] = (buf[3]<<24 | buf[4]<<16 | buf[5]<<8) / 256;
		adc[CH2] = (buf[6]<<24 | buf[7]<<16 | buf[8]<<8) / 256;

		for (j = 0; j < 2; j++) {
			adc_sum[j] += adc[j];
			if (adc[j] > adc_max[j]) {
				adc_max[j] = adc[j];
			}
			if (adc[j] < adc_min[j]) {
				adc_min[j] = adc[j];
			}
		}
		
	}

	for (j = 0; j < 2; j++) {
		adc_midpoint[j] = adc_sum[j] / SPS;
		printf ("CH%d %d / %d / %d range=%d\r\n", 
			(j+1),
			adc_min[j], adc_midpoint[j], adc_max[j], 
			adc_max[j]-adc_min[j]
		);
	}

	//
	// Now use the mid-point to get a mean of the high and low signal levels.
	//

	int32_t low_sum[2] = {0,0};
	int32_t high_sum[2] = {0,0};
	int32_t low_sum_count[2] = {0,0};
	int32_t high_sum_count[2] = {0,0};
	int64_t low_sum2[2] = {0,0};
	int64_t high_sum2[2] = {0,0};

	// http://www.johndcook.com/standard_deviation.html
	// also // See Knuth TAOCP vol 2, 3rd edition, page 232
	int64_t low_oldM[2], low_newM[2], low_oldS[2],low_newS[2];

	for (i = 0; i < SPS; i++) {
		if ( ads1x9x_drdy_wait (10000) == -1 ) {
			printf ("ERROR 1\n");
			return;
		}

		ads1x9x_ecg_read (buf);
		adc[CH1] = (buf[3]<<24 | buf[4]<<16 | buf[5]<<8) / 256;
		adc[CH2] = (buf[6]<<24 | buf[7]<<16 | buf[8]<<8) / 256;

		for (j = 0; j < 2; j++) {
			if (adc[j] >= adc_midpoint[j]) {
				high_sum[j] += adc[j];
				high_sum2[j] += adc[j]*adc[j];
				high_sum_count[j]++;
			}
			if (adc[j] < adc_midpoint[j]) {
				low_sum[j] += adc[j];
				low_sum2[j] += adc[j]*adc[j];
				low_sum_count[j]++;
			}
		}
	}

	// sigma2 = (1/(N-1)) * ( sumOfSquares - sum*sum / N)

	int32_t low_mean, high_mean, low_var, high_var, adc_per_v;
	int64_t sum2;
	//float low_sd,high_sd;
	for (j = 0; j < 2; j++) {
		low_mean = low_sum[j]/low_sum_count[j];
		high_mean = high_sum[j]/high_sum_count[j];
		sum2 = (int64_t)low_sum[j]*(int64_t)low_sum[j];
		low_var = (low_sum2[j] - sum2/(int64_t)low_sum_count[j])/((int64_t)low_sum_count[j]-1);
		sum2 = (int64_t)high_sum[j]*(int64_t)high_sum[j];
		high_var = (high_sum2[j] - sum2/(int64_t)high_sum_count[j])/((int64_t)high_sum_count[j]-1);
		adc_per_v =  (1000*(high_mean - low_mean))/ 2016 ;

		printf ("Test Signal on CH%d low=%d (%d samples, var=%d) high=%d (%d samples, var=%d) range=%d adc/V=%d\r\n", 
			(j+1),
			(int)low_mean, (int)low_sum_count[j], (int)low_var,
			(int)high_mean, (int)high_sum_count[j], (int)high_var,
			(int)(high_mean - low_mean), 
			(int)adc_per_v
		);
	}


}


