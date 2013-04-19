
#include <stdio.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"       // Generic helper functions
#include "core/adc/adc.h"
#include "core/systick/systick.h"
#include "core/gpio/gpio.h"
#include "core/iap/iap.h"
#include "ads1x9x.h"


/**************************************************************************/
/*! 
    'sysinfo' command handler
*/
/**************************************************************************/
void cmd_ads1x9x_reg_read_all(uint8_t argc, char **argv)
{
	int i;

/*
  printf("%-25s : %d.%d MHz %s", "System Clock", CFG_CPU_CCLK / 1000000, CFG_CPU_CCLK % 1000000, CFG_PRINTF_NEWLINE);
  printf("%-25s : %d.%d.%d %s", "Firmware", CFG_FIRMWARE_VERSION_MAJOR, CFG_FIRMWARE_VERSION_MINOR, CFG_FIRMWARE_VERSION_REVISION, CFG_PRINTF_NEWLINE);

  // 128-bit MCU Serial Number
  IAP_return_t iap_return;
  iap_return = iapReadSerialNumber();
  if(iap_return.ReturnCode == 0)
  {
    printf("%-25s : %08X %08X %08X %08X %s", "Serial Number", iap_return.Result[0],iap_return.Result[1],iap_return.Result[2],iap_return.Result[3], CFG_PRINTF_NEWLINE);
  }
*/

	// Read all registers
	for (i = 0; i < 12; i++) {
		printf ("%x ", ads1x9x_register_read(i));
	}
	printf ("\r\n");

}
