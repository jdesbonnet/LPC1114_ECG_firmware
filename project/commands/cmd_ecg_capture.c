/**************************************************************************/
/*! 
    @file     cmd_ecg_capture.c
    @author   Joe Desbonnet, jdesbonnet@gmail.com

    @brief    Code to capture ECG from ADI ADAS1000 ECG AFE.

    @section LICENSE

	TBD
*/
/**************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"
#include "adas1000.h"

#define NSAMPLE (2000)

void delay(void);
void setLED(int);
void testFail(void);
void sramSelect(void);
void sramDeselect(void);
void set_pins(void);
uint32_t reverse_byte_order (uint32_t in);

/**************************************************************************/
/*! 
    Capture ECG.
*/
/**************************************************************************/
void cmd_ecg_capture(uint8_t argc, char **argv)
{

int i;
for (i = 0; i < argc; i++) {
	printf ("arg[%d]=%s\n",i,argv[i]);
}

	int nsample = NSAMPLE;
	int rate = 500;
	if (argc >= 1) {
		nsample = atoi(argv[0]);
		printf ("nsample=%d\n",nsample);
	}
	if (argc >= 2) {
		if ( (argv[1][0]=='-') && (argv[1][1]=='r') ) {
			rate = atoi(argv[1]+2);
			printf ("rate=%d\n", rate);
		}
	}
	adas1000_init();
	adas1000_ecg_capture(nsample);
	adas1000_power_off();
}
void cmd_ecg_reset(uint8_t argc, char **argv)
{
	adas1000_reset();
}

void cmd_ecg_playback(uint8_t argc, char **argv)
{
	adas1000_ecg_playback(NSAMPLE);
}
void cmd_ecg_capture_stop (uint8_t argc, char **argv)
{
	adas1000_ecg_capture_stop();
}
void cmd_ecg_testtone (uint8_t argc, char **argv)
{
	adas1000_testtone_enable(TONETYPE_10HZ_SINE);
}
