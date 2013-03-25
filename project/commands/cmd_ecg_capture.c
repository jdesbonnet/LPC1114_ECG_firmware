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
	adas1000_init();
	adas1000_ecg_capture(NSAMPLE);
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
