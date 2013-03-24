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


void delay(void);
void setLED(int);
void testFail(void);
void sramSelect(void);
void sramDeselect(void);
void set_pins(void);
void adas1000_init(void);
void adas1000_register_write (uint8_t reg, uint32_t value);
uint32_t adas1000_register_read (uint8_t reg);
void adas1000_testtone_enable(void);
uint32_t reverse_byte_order (uint32_t in);

/**************************************************************************/
/*! 
    Capture ECG.
*/
/**************************************************************************/
void cmd_ecg_capture(uint8_t argc, char **argv)
{
	#ifdef CFG_ADAS1000
	adas1000_init();
	adas1000_testtone_enable();
	printf ("Capture...\n");
	adas1000_ecg_capture(10000);
	printf ("End capture.\n");
	adas1000_power_off();
	#else
	printf ("ADAS1000 not in build.\n");
	#endif // end ifdef ADAS1000
}
void cmd_ecg_playback(uint8_t argc, char **argv)
{
	#ifdef CFG_ADAS1000
	adas1000_ecg_playback(10000);
	#else
	printf ("ADAS1000 not in build.\n");
	#endif // end ifdef ADAS1000
}
void cmd_ecg_capture_stop (uint8_t argc, char **argv)
{
	adas1000_ecg_capture_stop();
}
