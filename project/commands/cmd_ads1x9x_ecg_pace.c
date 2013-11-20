#include <stdio.h>
#include <stdlib.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "core/systick/systick.h"
#include "commands.h"
#include "ads1x9x.h"
#include "parse_hex.h"
#include "stream_encode.h"

#define USE_FP_MATH
#define USE_INT_MATH

/**
 * PACE (Pace detect command).
 */
void cmd_ads1x9x_ecg_pace (uint8_t argc, char **argv)
{

	int32_t i=0,last_qrs=0,hp,i_hp;
	int32_t ch1,ch2;

	//uint32_t last_qrs_t = systickGetTicks();

	int status;
	uint8_t buf[12];

	#ifdef USE_FP_MATH
	double ch1d;
	double upper_envelope=0; 
	double d;
	double baseline_lpf=0; 
	double rm_mains_lpf=0; 
	double r; int32_t i_r;
	double hp_lpf=500; 
	#endif

	int32_t i_upper_envelope=0;
	int32_t i_baseline_lpf=0;
	int32_t i_rm_mains_lpf=0;
	int32_t i_hp_lpf=500*256;


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
	
		#ifdef USE_FP_MATH
		ch1d = (double)ch1;
		baseline_lpf += (ch1d-baseline_lpf)/256;
		rm_mains_lpf += (ch1d-rm_mains_lpf)/16;
		#endif

		i_baseline_lpf += (ch1*256 - i_baseline_lpf)/256;
		i_rm_mains_lpf += (ch1*256 - i_rm_mains_lpf)/16;

		// Upper envelope detector
		#ifdef USE_FP_MATH
		if (rm_mains_lpf > upper_envelope) {
			upper_envelope = rm_mains_lpf;
		} else {
			/*
			d = upper_envelope-baseline_lpf;
			if (d>0 && d < 2048) {
				d=2048;
			} else if (d<0 && d>-2048) {
				d=-2048;
			}
			upper_envelope -= d/2048;
			*/
			upper_envelope -= (upper_envelope-baseline_lpf)/2048;
		}
		#endif
		if (i_rm_mains_lpf > i_upper_envelope) {
			i_upper_envelope = i_rm_mains_lpf;
		} else {
			i_upper_envelope -= (i_upper_envelope-i_baseline_lpf)/2048;
		}

		// Heart period in sample periods
		hp = i - last_qrs;
		i_hp = (i - last_qrs)*256;

		if ( hp>100 ) {

			// Where is the ECG signal (after 50Hz removal LPF) in the range of
			// Q to R range? Expect approx -64 -> 256


			#ifdef USE_FP_MATH
			r =  (rm_mains_lpf-baseline_lpf) / (upper_envelope-baseline_lpf);
			if (r > 0.7) {
				if ( hp < (int)(hp_lpf*1.4)) {
					hp_lpf += ((double)hp - hp_lpf)/8.0;
				} else {
					hp_lpf += ((double)hp - hp_lpf)/32.0;
				}
				last_qrs=i;
				printf ("%d %d %d %d\r\n",(int)i,(int)hp_lpf*2, 
					(int)baseline_lpf, (int)upper_envelope);
			}
			#else
			// i_r 0 = baseline; 256 = upper_envelope and also dip negative
			// TODO: is long casting necessary?
			i_r = (long)(i_rm_mains_lpf - i_baseline_lpf) / ((long)(i_upper_envelope - i_baseline_lpf)/256);
			if (i_r > 180) {
				if ( hp < (i_hp_lpf*14)/10) {
					i_hp_lpf += (i_hp - i_hp_lpf)/8;
				} else {
					i_hp_lpf += (i_hp - i_hp_lpf)/32;
				}
				last_qrs=i;
				printf ("%d %d %d %d\r\n",(int)i,(int)(i_hp_lpf*2/256), 
					(int)i_baseline_lpf/256, (int)i_upper_envelope/256);
			}
			#endif
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
