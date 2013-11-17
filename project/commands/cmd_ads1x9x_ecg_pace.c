#include <stdio.h>
#include <stdlib.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"
#include "ads1x9x.h"
#include "parse_hex.h"
#include "stream_encode.h"

void cmd_ads1x9x_ecg_pace (uint8_t argc, char **argv)
{

	//uint32_t i;
	uint32_t ch1,ch2;
	uint32_t lpf=0,v;

	int status;
	uint8_t buf[12];

	// UI LED on to indicate ECG capture in progress
	setLED(1,1);


	// Write to CONFIG1 to set continuous mode 500sps (0x02)
	ads1x9x_register_write(REG_CONFIG1, 0x02);

	// Issue read continuous (RDATAC)
	ads1x9x_command(CMD_RDATAC);
	
printf ("PACEDETECT\r\n" );


	// Loop for number of samples required
	while (1) {

		// Wait for data available (DRDY line goes low)
		status = ads1x9x_drdy_wait(1000000);
		if (status != 0) {
			printf ("TIMEOUT\r\n");
			break;
		}

		ads1x9x_ecg_read (&buf);

		// Record comprises 24bit status + 2 x 24bit channel data
		// Status is 1100 + LOFF_STAT[4:0] + GPIO[1:0] + 00000 0000 0000
		status = ((buf[0]<<8 | buf[1])>>5)&0xff;

		ch1 = (buf[3]<<24 | buf[4]<<16 | buf[5]<<8)/256;
		ch2 = (buf[6]<<24 | buf[7]<<16 | buf[8]<<8)/256;

		lpf += ch1;
		v = lpf>>10;
		lpf -= v;
		printf ("%d ", (int)v );

		cmdPoll();

		// Check for ESC from host
		if (cmdIsEscape()) {
			cmdResetEscape();
			printf ("\r\nESCAPE\r\n");
			break;
		}


	}

	// Stop continuous data conversion
	ads1x9x_command(CMD_SDATAC);
	

	// Stop conversions
	ads1x9x_command(CMD_STOP);

	// UI LED off
	setLED(1,0);

}
