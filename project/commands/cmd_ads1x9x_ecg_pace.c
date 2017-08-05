#include <stdio.h>
#include <stdlib.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "core/systick/systick.h"
#include "commands.h"
#include "ads1x9x.h"
#include "parse_hex.h"
#include "stream_encode.h"

// Define USE_FP_MATH to enable double precision floating point 
// calculations. This adds at least 7kB to the program for FP
// library as Cortex M0 does not support FP in hardware.
//#define USE_FP_MATH


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
	double r; 
	double hp_lpf=500; 
	#endif

	int32_t i_upper_envelope=0;
	int32_t i_baseline_lpf=0;
	int32_t i_rm_mains_lpf=0;
	int32_t i_hp_lpf=500*256;
	int32_t i_r;

	// QRS threshold detection. 256 = baseline -> upper envelope, 180 default
	int32_t i_qrs_threshold = config_var[CONFIG_QRS_DETECT_THRESHOLD];
	i_qrs_threshold = i_qrs_threshold == 0 ? 180 : i_qrs_threshold;

	// UI LED on to indicate ECG capture in progress
	setLED(1,1);


	// Write to CONFIG1 to set continuous mode 500sps (0x02)
	ads1x9x_register_write(REG_CONFIG1, 0x02);

	// Write to CONFIG1 to set continuous mode 500sps (0x02)
	ads1x9x_register_write(REG_CH1SET, 0x00);

	// Start conversions
	ads1x9x_command(CMD_START);


	// Calibrate offset
	ads1x9x_command(CMD_OFFSETCAL); 
	delay(2000000);

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


		// Oscilloscope observation on GREEN_LED: 
		// from here to end of loop (as of 20 Nov 2013)
		// ~ 32 µs from here to end off loop (with no outpu)
		// 900µs when outputing pace data
		//setLED(GREEN_LED,0);

		// Record comprises 24bit status + 2 x 24bit channel data
		// Status is 1100 + LOFF_STAT[4:0] + GPIO[1:0] + 00000 0000 0000
		status = ((buf[0]<<8 | buf[1])>>5)&0xff;

		// TODO: looking as assembler output, each byte is loaded separately
		// Should be possible to move 3 conseq bytes in buf into a 32 bit var
		// in one instruction (?)
		//ch1 = (buf[3]<<24 | buf[4]<<16 | buf[5]<<8)/256;
		//ch2 = (buf[6]<<24 | buf[7]<<16 | buf[8]<<8)/256;
		ch1 = (buf[3]<<24 | buf[4]<<16 | buf[5]<<8);
		ch2 = (buf[6]<<24 | buf[7]<<16 | buf[8]<<8);

		// http://electronics.stackexchange.com/questions/30370/fast-and-memory-efficient-moving-average-calculation
		// FILT <-- FILT + FF(NEW - FILT)
	
		#ifdef USE_FP_MATH
		ch1d = (double)ch1;
		baseline_lpf += (ch1d-baseline_lpf)/256;
		rm_mains_lpf += (ch1d-rm_mains_lpf)/16;
		#else
		//i_baseline_lpf += (ch1*256 - i_baseline_lpf)/256;
		//i_rm_mains_lpf += (ch1*256 - i_rm_mains_lpf)/16;
		i_baseline_lpf += (ch1 - i_baseline_lpf)/256;
		i_rm_mains_lpf += (ch1 - i_rm_mains_lpf)/16;
		#endif

		// Upper envelope detector
		#ifdef USE_FP_MATH
		if (rm_mains_lpf > upper_envelope) {
			upper_envelope = rm_mains_lpf;
		} else {
			upper_envelope -= (upper_envelope-baseline_lpf)/2048;
		}
		#else
		if (i_rm_mains_lpf > i_upper_envelope) {
			i_upper_envelope = i_rm_mains_lpf;
		} else {
			i_upper_envelope -= (i_upper_envelope-i_baseline_lpf)/2048;
		}
		#endif

		// Number of samples since last ECG QRS (x256 for integer implementation)
		hp = i - last_qrs;
		i_hp = hp * 256;

		// Ignore anything 100 samples after last QRS: too short for next QRS.
		if ( hp>100 ) {

			// r/i_r: where is the ECG signal (after 50Hz removal LPF) in
			// baseline to R range? For FP implementation 0=baseline, 1=R. 
			// For integer implementation 0=baseline, 256=R.
			// In both cases r/i_r can be negative (typically -0.3 for FP)
			// or -64 for integer.

			// If r/i_r crosses 70% threshold then consider it a 'R' event.
			// If R event update hp_lpf (time averaged heart period). Also
			// update last_qrs and output data.

			// Important that data output (and all processing) be accomplished
			// in significantly under the 2000µs sampling period (500sps) or
			// ECG data will be lost.

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

			if (i_r > i_qrs_threshold) {
				if ( hp < (i_hp_lpf*14)/10) {
					i_hp_lpf += (i_hp - i_hp_lpf)/4;
				} else {
					i_hp_lpf += (i_hp - i_hp_lpf)/16;
				}
				last_qrs=i;

				// Blink LED (use duration of UART output for blink length).
				setLED(GREEN_LED,1);
				printf ("%d %d %d\r\n",(int)i,(int)(hp*2),(int)(i_hp_lpf*2/256));
				setLED(GREEN_LED,0);
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
