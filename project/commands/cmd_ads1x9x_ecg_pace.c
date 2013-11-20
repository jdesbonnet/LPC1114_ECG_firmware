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

	int32_t i=0,last_qrs=0;
	int32_t ch1,ch2;

	int status;
	uint8_t buf[12];

	int32_t upper_envelope=0;
	int32_t baseline_lpf=0;
	int32_t rm_mains_lpf=0;
	int32_t r;
	int32_t hp, hp_lpf=500;

	// UI LED on to indicate ECG capture in progress
	setLED(1,1);


	// Write to CONFIG1 to set continuous mode 500sps (0x02)
	ads1x9x_register_write(REG_CONFIG1, 0x02);

	// Write to CONFIG1 to set continuous mode 500sps (0x02)
	ads1x9x_register_write(REG_CH1SET, 0x00);

	// Calibrate offset
	ads1x9x_command(CMD_OFFSETCAL); 

	// Start conversions
	ads1x9x_command(CMD_START);

	// Issue read continuous (RDATAC)
	ads1x9x_command(CMD_RDATAC);
	

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
	
		baseline_lpf += (ch1-baseline_lpf)/256;
		rm_mains_lpf += (ch1-rm_mains_lpf)/16;

		// Upper envelope detector
		if (rm_mains_lpf > upper_envelope) {
			upper_envelope = rm_mains_lpf;
		} else {
			upper_envelope -= (upper_envelope-baseline_lpf)/2048;
		}

		// Heart period in sample periods
		hp = i - last_qrs;

		// 38/64 =~ 60%
		if ( hp > (hp_lpf*38)/64 ) {

			// Where is the ECG signal (after 50Hz removal LPF) in the range of
			// Q to R range? Expect approx -64 -> 256
			r = (int32_t)( ((long)(rm_mains_lpf-baseline_lpf)*64L) / (long)(upper_envelope-baseline_lpf));

			// 45/64 =~ 70%
			if (r > 45) {
				// 90/64 =~ 140%
				if ( hp < (hp_lpf*90)/64) {
					hp_lpf += (hp - hp_lpf)/16;
				}
				last_qrs=i;
				printf ("%d %d %d %d %d %d %d\r\n",(int)i,(int)hp, (int)(hp_lpf*2), (int)baseline_lpf, (int)upper_envelope, (int)rm_mains_lpf, (int)r);
			}
		}


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
