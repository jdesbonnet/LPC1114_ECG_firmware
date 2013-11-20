#include <stdio.h>
#include <stdlib.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "core/systick/systick.h"
#include "commands.h"
#include "ads1x9x.h"
#include "parse_hex.h"
#include "stream_encode.h"

/**
 * PACE (Pace detect command).
 */
void cmd_ads1x9x_ecg_pace (uint8_t argc, char **argv)
{

	uint32_t i;
	int32_t ch1,ch2;
	int32_t lpf=0,v;

	int status,skip=0;
	uint8_t buf[12];


	int32_t threshold = atoi (argv[0]);
	int32_t SKIP = atoi(argv[1]);

	// UI LED on to indicate ECG capture in progress
	setLED(1,1);


	// Write to CONFIG1 to set continuous mode 500sps (0x02)
	ads1x9x_register_write(REG_CONFIG1, 0x02);

	// Start conversions
	ads1x9x_command(CMD_START);

	// Issue read continuous (RDATAC)
	ads1x9x_command(CMD_RDATAC);
	

	printf ("threshold=%d skip=%d\r\n", (int)threshold, (int)SKIP);

	// Loop for number of samples required
	while (1) {

		i++;


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

		// http://electronics.stackexchange.com/questions/30370/fast-and-memory-efficient-moving-average-calculation
		// FILT <-- FILT + FF(NEW - FILT)
		lpf += (ch1-lpf)/256;

		
		if (skip==0) {
			if ( (ch1 - lpf) > threshold ) {
				printf ("P %d\r\n", (int)systickGetTicks());
				skip=SKIP;
			}
		} else {
			skip--;
		}

		/*
		if (i%250==0) {
			printf ("%d %d %d\r\n", (int)(ch1), (int)v, (int)lpf );
		}
		*/

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
